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
static struct ARC_FreelistMeta new_list = { 0 };
static struct ARC_FreelistMeta combined = { 0 };

void *Arc_AllocPMM() {
	if (arc_physical_mem == NULL) {
		return NULL;
	}

	return Arc_ListAlloc(arc_physical_mem);
}

void *Arc_ContiguousAllocPMM(size_t objects) {
	if (arc_physical_mem == NULL) {
		return NULL;
	}

	return Arc_ListContiguousAlloc(arc_physical_mem, objects);
}

void *Arc_FreePMM(void *address) {
	if (arc_physical_mem == NULL) {
		return NULL;
	}

	return Arc_ListFree(arc_physical_mem, address);
}

void *Arc_ContiguousFreePMM(void *address, size_t objects) {
	if (arc_physical_mem == NULL) {
		return NULL;
	}

	return Arc_ListContiguousFree(arc_physical_mem, address, objects);
}

void Arc_InitPMM(struct ARC_MMap *mmap, int entries) {
	if (mmap == NULL || entries == 0) {
		ARC_DEBUG(ERR, "Failed to initialize 64-bit PMM (one or more are NULL: %p %d)\n", mmap, entries);
		ARC_HANG;
	}

	mmap = (struct ARC_MMap *)ARC_PHYS_TO_HHDM(mmap);

	arc_physical_mem = (struct ARC_FreelistMeta *)(Arc_BootMeta->pmm_state + ARC_HHDM_VADDR);

	// Revise old PMM meta to use HHDM addresses
	arc_physical_mem->base = (struct ARC_FreelistNode *)ARC_PHYS_TO_HHDM(arc_physical_mem->base);
	arc_physical_mem->ciel = (struct ARC_FreelistNode *)ARC_PHYS_TO_HHDM(arc_physical_mem->ciel);
	arc_physical_mem->head = (struct ARC_FreelistNode *)ARC_PHYS_TO_HHDM(arc_physical_mem->head);

	ARC_DEBUG(INFO, "Bootstrap allocator: { B:%p C:%p H:%p SZ:%lu }\n", arc_physical_mem->base, arc_physical_mem->ciel, arc_physical_mem->head, arc_physical_mem->object_size);

	// Update old PMM freelist to point to HHDM
	// addresses
	struct ARC_FreelistNode *current = arc_physical_mem->head;

	// Since all addresses are in the HHDM, NULL = ARC_HHDM_VADDR
	while ((uintptr_t)current != ARC_HHDM_VADDR) {
		// Since this list was made in 32-bit mode, care only about
		// the first 32-bits
		uintptr_t next = ((uintptr_t)current->next) & UINT32_MAX;

		// Update the next pointer to use an HHDM address
		current->next = (void *)ARC_PHYS_TO_HHDM(next);
		// Move on
		current = current->next;
	}

	for (int i = 0; i < entries; i++) {
		struct ARC_MMap entry = mmap[i];

		uintptr_t entry_base = ARC_PHYS_TO_HHDM(entry.base);
		uintptr_t entry_ciel = ARC_PHYS_TO_HHDM(entry.base + entry.len);

		ARC_DEBUG(INFO, "%8d: 0x%16"PRIx64" -> 0x%16"PRIx64", 0x%16"PRIx64" B (%d)\n", i, entry.base, entry_base, entry.len, entry.type);

		if (entry.type != MULTIBOOT_MEMORY_AVAILABLE) {
			// Memory is not available, we are not interested
			continue;
		}

		if (entry_base >= (uintptr_t)arc_physical_mem->base && entry_ciel <= (uintptr_t)arc_physical_mem->ciel) {
			// Entry is already apart of freelist
			continue;
		}

		if (entry_base <= (uintptr_t)arc_physical_mem->base && (uintptr_t)arc_physical_mem->base < entry_ciel) {
			// Entry contains the beginning of the freelist
			continue;
		}

		ARC_DEBUG(INFO, "MMAP entry %d is not apart of the freelist\n", i);

		Arc_InitializeFreelist((void *)entry_base, (void *)entry_ciel, 0x1000, &new_list);
		int code = Arc_ListLink(arc_physical_mem, &new_list, &combined);

		if (code != 0) {
			ARC_DEBUG(INFO, "Failed to link lists (%d)\n", code);
			continue;
		}

		arc_physical_mem = &combined;

		ARC_DEBUG(INFO, "MMAP entry %d has been successfully linked into freelist\n", i);
	}

	ARC_DEBUG(INFO, "Finished setting up kernel PMM\n");
}
