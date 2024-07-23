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

void *alloc(size_t size) {
	return slab_alloc(&top_alloc, size);
}

void *calloc(size_t size, size_t count) {
	return slab_alloc(&top_alloc, size * count);
}

void *free(void *address) {
	return slab_free(&top_alloc, address);
}

void *realloc(void *address, size_t size) {
	(void)address;
	(void)size;

	ARC_DEBUG(ERR, "Unimplemented Arc_Realloc\n");

	return NULL;
}

int Arc_ExpandAllocator(size_t pages) {
	int cumulative_err = (slab_expand(&top_alloc, 0, pages) == 0);
	cumulative_err += (slab_expand(&top_alloc, 1, pages) == 0);
	cumulative_err += (slab_expand(&top_alloc, 2, pages) == 0);
	cumulative_err += (slab_expand(&top_alloc, 3, pages) == 0);
	cumulative_err += (slab_expand(&top_alloc, 4, pages) == 0);
	cumulative_err += (slab_expand(&top_alloc, 5, pages) == 0);
	cumulative_err += (slab_expand(&top_alloc, 6, pages) == 0);
	cumulative_err += (slab_expand(&top_alloc, 7, pages) == 0);

	return cumulative_err;
}

int init_allocator(size_t pages) {
	return init_slab(&top_alloc, pages);
}
