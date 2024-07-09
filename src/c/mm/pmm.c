/**
 * @file pmm.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is part of Arctan.
 *
 * Arctan is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
*/
#include <arctan.h>
#include <global.h>
#include <mm/freelist.h>
#include <mm/pmm.h>
#include <stdint.h>

static struct ARC_FreelistMeta *arc_physical_mem = NULL;

void *Arc_AllocPMM() {
	if (arc_physical_mem == NULL) {
		ARC_DEBUG(ERR, "arc_physical_mem is NULL!\n");
		return NULL;
	}

	return Arc_ListAlloc(arc_physical_mem);
}

void *Arc_ContiguousAllocPMM(size_t objects) {
	if (arc_physical_mem == NULL) {
		ARC_DEBUG(ERR, "arc_physical_mem is NULL!\n");
		return NULL;
	}

	return Arc_ListContiguousAlloc(arc_physical_mem, objects);
}

void *Arc_FreePMM(void *address) {
	if (arc_physical_mem == NULL) {
		ARC_DEBUG(ERR, "arc_physical_mem is NULL!\n");
		return NULL;
	}

	return Arc_ListFree(arc_physical_mem, address);
}

void *Arc_ContiguousFreePMM(void *address, size_t objects) {
	if (arc_physical_mem == NULL) {
		ARC_DEBUG(ERR, "arc_physical_mem is NULL!\n");
		return NULL;
	}

	return Arc_ListContiguousFree(arc_physical_mem, address, objects);
}

int Arc_InitPMM(struct ARC_MMap *mmap, int entries) {
	if (mmap == NULL || entries == 0) {
		ARC_DEBUG(ERR, "Failed to initialize 64-bit PMM (one or more are NULL: %p %d)\n", mmap, entries);
		ARC_HANG;
	}

	mmap = (struct ARC_MMap *)ARC_PHYS_TO_HHDM(mmap);

	arc_physical_mem = (struct ARC_FreelistMeta *)ARC_PHYS_TO_HHDM(Arc_BootMeta->pmm_state);

	ARC_DEBUG(INFO, "Converting bootstrap allocator to use HHDM addresses\n");

	// All addresses in the meta remain physical, need to
	// convert to HHDM addresses (except for head)
	struct ARC_FreelistMeta *current = arc_physical_mem;
	struct ARC_FreelistMeta *highest_meta = arc_physical_mem;
	while (current != NULL) {
		current->base = (struct ARC_FreelistNode *)ARC_PHYS_TO_HHDM(current->base);
		current->ciel = (struct ARC_FreelistNode *)ARC_PHYS_TO_HHDM(current->ciel);
		// Head may an HHDM address if the list has been used
		// therefore ignore the upper 32-bits and convert it to
		// an HHDM address anyway
		current->head = (struct ARC_FreelistNode *)ARC_PHYS_TO_HHDM(((uint64_t)current->head) & UINT32_MAX);

		if (current->next != NULL) {
			current->next = (struct ARC_FreelistMeta *)ARC_PHYS_TO_HHDM(current->next);
		}
	
		highest_meta = current;
		current = current->next;
	}

	ARC_DEBUG(INFO, "Converted: { B:%p C:%p H:%p SZ:%lu }\n", arc_physical_mem->base, arc_physical_mem->ciel, arc_physical_mem->head, arc_physical_mem->object_size);

	uint64_t highest_alloc = (uint64_t)highest_meta->ciel;

	ARC_DEBUG(INFO, "Highest allocatable address: 0x%"PRIx64"\n", highest_alloc);

	for (int i = 0; i < entries; i++) {
		struct ARC_MMap entry = mmap[i];

		ARC_DEBUG(INFO, "\t%3d : 0x%016"PRIx64" -> 0x%016"PRIx64" (0x%016"PRIx64" bytes) | (%d)\n", i, entry.base, entry.base + entry.len, entry.len, entry.type);

		if (ARC_PHYS_TO_HHDM(entry.base) < highest_alloc || entry.type != ARC_MEMORY_AVAILABLE) {
			// Entry is below the highest allocation or it is not available
			// then skip
			continue;
		}

		ARC_DEBUG(INFO, "\t\tEntry is not in list, adding\n");

		// Found a memory entry that is not yet in the allocator
		struct ARC_FreelistMeta *list = Arc_InitializeFreelist(ARC_PHYS_TO_HHDM(entry.base), ARC_PHYS_TO_HHDM(entry.base + entry.len), 0x1000);


		int ret = Arc_ListLink(highest_meta, list);
		if (ret != 0) {
			ARC_DEBUG(ERR, "\t\tFailed to link lists (%d)\n", ret);
			continue;
		}

		ARC_DEBUG(INFO, "\t\tAdded\n");

		highest_meta = list;
	}

	ARC_DEBUG(INFO, "Finished setting up kernel PMM\n");

	return 0;
}
