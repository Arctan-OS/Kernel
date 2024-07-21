/**
 * @file allocator.c
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
 * Implements functions used for in kernel allocations.
*/
#include <mm/allocator.h>
#include <mm/slab.h>
#include <global.h>

struct ARC_SlabMeta top_alloc = { 0 };

void *Arc_Alloc(size_t size) {
	return Arc_SlabAlloc(&top_alloc, size);
}

void *Arc_Calloc(size_t size, size_t count) {
	return Arc_SlabAlloc(&top_alloc, size * count);
}

void *Arc_Free(void *address) {
	return Arc_SlabFree(&top_alloc, address);
}

void *Arc_Realloc(void *address, size_t size) {
	(void)address;
	(void)size;

	ARC_DEBUG(ERR, "Unimplemented Arc_Realloc\n");

	return NULL;
}

int Arc_ExpandAllocator(size_t pages) {
	int cumulative_err = (Arc_ExpandSlab(&top_alloc, 0, pages) == 0);
	cumulative_err += (Arc_ExpandSlab(&top_alloc, 1, pages) == 0);
	cumulative_err += (Arc_ExpandSlab(&top_alloc, 2, pages) == 0);
	cumulative_err += (Arc_ExpandSlab(&top_alloc, 3, pages) == 0);
	cumulative_err += (Arc_ExpandSlab(&top_alloc, 4, pages) == 0);
	cumulative_err += (Arc_ExpandSlab(&top_alloc, 5, pages) == 0);
	cumulative_err += (Arc_ExpandSlab(&top_alloc, 6, pages) == 0);
	cumulative_err += (Arc_ExpandSlab(&top_alloc, 7, pages) == 0);

	return cumulative_err;
}

int Arc_InitializeAllocator(size_t pages) {
	return Arc_InitSlabAllocator(&top_alloc, pages);
}
