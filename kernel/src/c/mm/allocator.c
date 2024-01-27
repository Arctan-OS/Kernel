/**
 * @file freelist.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is apart of Arctan.
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
#include "mm/freelist.h"
#include <mm/allocator.h>

struct ARC_AllocMeta {
	struct ARC_FreelistMeta *physical_mem;
	struct ARC_FreelistMeta lists[10];
	size_t list_sizes[10];
};

static struct ARC_AllocMeta heap = { 0 };

void *Arc_SlabAlloc(size_t size) {
	if (size > 0x1000) {
		// Just allocate a contiguous set of pages
		return NULL;
	}

	int i = 0;
	for (; i < 9; i++) {
		if (heap.list_sizes[i] <= size && size <= heap.list_sizes[i + 1]) {
			break;
		}
	}

	return Arc_ListAlloc(&heap.lists[i]);
}

void *Arc_SlabFree(void *address) {

	return NULL;
}

/**
 * Initialize the given list in \a heap.
 *
 * @param int i - List to initialize.
 * @size_t size_t size - The page count for the entry.
 * @size_t size_t object_size - The size of each object.
 * @return Error code (0: success)
 *  */
static int Arc_InitList(int i, size_t size, size_t object_size) {
	heap.list_sizes[i] = size;

	struct ARC_FreelistMeta base = { 0 };
	struct ARC_FreelistMeta new = { 0 };

	for (size_t page_idx = 0; page_idx < size; page_idx++) {
		void *page = Arc_ListAlloc(heap.physical_mem);

		if (base.base == NULL) {
			Arc_InitializeFreelist(page, page + 0x1000, object_size, &base);
		} else {
			Arc_InitializeFreelist(page, page + 0x1000, object_size, &new);

			struct ARC_FreelistMeta combined = { 0 };

			Arc_ListLink(&base, &new, &combined);

			base.base = combined.base;
			base.ciel = combined.ciel;
			base.head = combined.head;
		}

	}

	heap.lists[i].base = base.base;
	heap.lists[i].ciel = base.ciel;
	heap.lists[i].head = base.head;
	heap.lists[i].object_size = object_size;
}

int Arc_InitSlabAllocator(struct ARC_FreelistMeta *memory) {


	return 0;
}
