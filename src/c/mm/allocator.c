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
#include <mm/algo/slab.h>
#include <mm/vmm.h>
#include <global.h>

static struct ARC_SlabMeta meta = { 0 };

void *alloc(size_t size) {
	return slab_alloc(&meta, size);
}

void *calloc(size_t size, size_t count) {
	return slab_alloc(&meta, size * count);
}

void *free(void *address) {
	return slab_free(&meta, address);
}

void *realloc(void *address, size_t size) {
	(void)address;
	(void)size;

	ARC_DEBUG(ERR, "Unimplemented Arc_Realloc\n");

	return NULL;
}

int allocator_expand(size_t pages) {
	return 0;
}

int init_allocator(size_t pages) {
	size_t range_length = (pages << 12) * 8;
	void *range = (void *)vmm_alloc(range_length);

	if (range == NULL) {
		return -1;
	}

	return init_slab(&meta, range, range_length) != range + range_length;
}
