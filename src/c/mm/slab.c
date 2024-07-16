/**
 * @file slab.c
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
#include <mm/freelist.h>
#include <mm/pmm.h>
#include <mm/slab.h>
#include <global.h>
#include <lib/util.h>

void *Arc_SlabAlloc(struct ARC_SlabMeta *meta, size_t size) {
	if (size > 0x1000) {
		// Just allocate a contiguous set of pages
		ARC_DEBUG(ERR, "Failed to allocate size %d\n", size);
		return NULL;
	}

	int i = 0;
	for (; i < 8; i++) {
		if (size <= meta->list_sizes[i]) {
			break;
		}
	}

	return Arc_ListAlloc(meta->lists[i]);
}

void *Arc_SlabFree(struct ARC_SlabMeta *meta, void *address) {
	int list = -1;
	for (int i = 0; i < 8; i++) {
		void *base = meta->lists[i]->base;
		void *ceil = meta->lists[i]->ceil;

		if (base <= address && address <= ceil) {
			list = i;
			break;
		}
	}

	if (list == -1) {
		// Could not find the list
		ARC_DEBUG(ERR, "Failed to free %p\n", address);
		return NULL;
	}

	memset(address, 0, meta->list_sizes[list]);

	return Arc_ListFree(meta->lists[list], address);
}

// TODO: Realloc, Calloc

int Arc_InitSlabAllocator(struct ARC_SlabMeta *meta, size_t init_page_count) {
	ARC_DEBUG(INFO, "Initializing SLAB allocator (%d)\n", init_page_count);

	// TODO: Abstract this away, so that it uses a given memory range rather than
	//       allocating its own.

	size_t object_size = 16;
	for (int i = 0; i < 8; i++) {
		ARC_DEBUG(INFO, "Initializing SLAB (%p) list %d { .size = %lu pages, .obj_size = %lu bytes }\n", meta, i, init_page_count, object_size);

		uint64_t base = (uint64_t)Arc_ContiguousAllocPMM(init_page_count);
		meta->lists[i] = Arc_InitializeFreelist(base, base + (init_page_count * PAGE_SIZE), object_size);
		meta->list_sizes[i] = object_size;

		object_size *= 2;
	}

	ARC_DEBUG(INFO, "Initialized SLAB allocator\n");

	return 0;
}
