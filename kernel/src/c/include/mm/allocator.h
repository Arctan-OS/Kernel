/**
 * @file allocator.h
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
#ifndef ARC_MM_ALLOCATOR_H
#define ARC_MM_ALLOCATOR_H

#include <stddef.h>
#include <mm/freelist.h>

/**
 * Allocate \a size bytes in the kernel heap.
 *
 * @param size_t size - The number of bytes to allocate.
 * @return The base address of the allocation.
 * */
void *Arc_SlabAlloc(size_t size);

/**
 * Free the allocation at \a address.
 *
 * @param void *address - The allocation to free from the kernel heap.
 * @return The given address if successful.
 * */
void *Arc_SlabFree(void *address);

/**
 * Initialize the kernel SLAB allocator.
 *
 * @param struct ARC_FreelistMeta *memory - The freelist in which to initialize the allocator's lists.
 * @param int init_page_count - The number of 0x1000 byte pages each list is given.
 * @return Error code (0: success).
 * */
int Arc_InitSlabAllocator(size_t init_page_count);

#endif
