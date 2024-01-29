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
#include <global.h>

struct ARC_AllocMeta {
	struct ARC_FreelistMeta *physical_mem;
	struct ARC_FreelistMeta lists[8];
	size_t list_sizes[8];
};

static struct ARC_AllocMeta heap = { 0 };

void *Arc_SlabAlloc(size_t size) {
	if (size > 0x1000) {
		// Just allocate a contiguous set of pages
		return NULL;
	}

	int i = 0;
	for (; i < 8; i++) {
		if (size <= heap.list_sizes[i]) {
			break;
		}
	}

	return Arc_ListAlloc(&heap.lists[i]);
}

void *Arc_SlabFree(void *address) {
	int list = -1;
	for (int i = 0; i < 8; i++) {
		void *base = heap.lists[i].base;
		void *ciel = heap.lists[i].ciel;

		if (base <= address && address <= ciel) {
			list = i;
			break;
		}
	}

	if (list == -1) {
		// Could not find the list
		return NULL;
	}

	return Arc_ListFree(&heap.lists[list], address);
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
	ARC_DEBUG(INFO, "Initializing SLAB (%p) list %d { .size = %d pages, .obj_size = %d bytes }\n", &heap, i, size, object_size);

	heap.list_sizes[i] = object_size;

	void *base = Arc_ListContiguousAlloc(heap.physical_mem, size);

	Arc_InitializeFreelist(base, base + (size << 12), object_size, &heap.lists[i]);

	return 0;
}

int Arc_InitSlabAllocator(struct ARC_FreelistMeta *memory, int init_page_count) {
	heap.physical_mem = memory;

	Arc_InitList(0, init_page_count, 16);
	Arc_InitList(1, init_page_count, 32);
	Arc_InitList(2, init_page_count, 64);
	Arc_InitList(3, init_page_count, 128);
	Arc_InitList(4, init_page_count, 256);
	Arc_InitList(5, init_page_count, 512);
	Arc_InitList(6, init_page_count, 1024);
	Arc_InitList(7, init_page_count, 2048);

	ARC_DEBUG(INFO, "Initialized SLAB allocator\n");

	return 0;
}
