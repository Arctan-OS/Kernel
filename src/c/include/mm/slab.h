/**
 * @file slab.h
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
#ifndef ARC_MM_SLAB_H
#define ARC_MM_SLAB_H

#include <stddef.h>
#include <mm/freelist.h>

struct ARC_SlabMeta {
	struct ARC_FreelistMeta *physical_mem;
	struct ARC_FreelistMeta *lists[8];
	size_t list_sizes[8];
};

/**
 * Allocate \a size bytes in the kernel heap.
 *
 * @param size_t size - The number of bytes to allocate.
 * @return The base address of the allocation.
 * */
void *slab_alloc(struct ARC_SlabMeta *meta, size_t size);

/**
 * Free the allocation at \a address.
 *
 * @param void *address - The allocation to free from the kernel heap.
 * @return The given address if successful.
 * */
void *slab_free(struct ARC_SlabMeta *meta, void *address);

/**
 * Expand a given SLAB's list
 *
 * This will expand a certain list (0-7) by the given number
 * of pages.
 *
 * @param struct ARC_SlabMeta *slab - The SLAB to expand.
 * @param int list - The freelist within the SLAB (0-7).
 * @param int pages - The number of pages to expand the list by.
 * @return zero upon success.
 * */
int slab_expand(struct ARC_SlabMeta *slab, int list, size_t pages);
/**
 * Initialize the kernel SLAB allocator.
 *
 * @param struct ARC_FreelistMeta *memory - The freelist in which to initialize the allocator's lists.
 * @param int init_page_count - The number of 0x1000 byte pages each list is given.
 * @return Error code (0: success).
 * */
int init_slab(struct ARC_SlabMeta *meta, size_t init_page_count);

#endif
