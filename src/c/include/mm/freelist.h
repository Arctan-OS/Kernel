/**
 * @file freelist.h
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
#ifndef ARC_MM_FREELIST_H
#define ARC_MM_FREELIST_H

#include <stdint.h>
#include <stddef.h>

struct ARC_FreelistNode {
	struct ARC_FreelistNode *next __attribute__((aligned(8)));
};

struct ARC_FreelistMeta {
	struct ARC_FreelistNode *head __attribute__((aligned(8)));
	struct ARC_FreelistNode *base __attribute__((aligned(8)));
	struct ARC_FreelistNode *ciel __attribute__((aligned(8)));
	uint64_t object_size __attribute__((aligned(8)));
}__attribute__((packed));

/**
 * Allocate a single object in the given meta.
 *
 * @param struct ARC_FreelistMeta *meta - The list from which to allocate one object
 * @return A void * to the base of the newly allocated object.
 * */
void *Arc_ListAlloc(struct ARC_FreelistMeta *meta);

/**
 * Allocate a contiguous section of memory.
 *
 * @param struct ARC_FreelistMeta *meta - The list in which to allocate the contiguous region of memory.
 * @param int objects - Number of contiguous objects to allocate.
 * @return The base address of the contiguous section.
 * */
void *Arc_ListContiguousAlloc(struct ARC_FreelistMeta *meta, int objects);

/**
 * Free the object at the given address in the given meta.
 *
 * @param struct ARC_FreelistMeta *meta - The freelist in which to free the address
 * @param void *address - A pointer to the base of the given object to be freed.
 * @return /a address when successfull.
 * */
void *Arc_ListFree(struct ARC_FreelistMeta *meta, void *address);

/**
 * Free a contiguous section of memory.
 *
 * @param struct ARC_FreelistMeta *meta - The list in which to free the contiguous region of memory.
 * @param void *address - The base address of the contiguous section.
 * @param int objects - The number of objects the section consists of.
 * @return The base address if the free was successful. */
void *Arc_ListContiguousFree(struct ARC_FreelistMeta *meta, void *address, int objects);

/**
 * Combine list A and list B into a single list C.
 *
 * @return When a 0 is returned, linking of A and B was successfull.\n
 * When a -1 is returned, the object size of A and B don't match.\n
 * When a -2 is returned, both lists have been allocated to already,
 * making combination impossible without losing one list's data.
 * */
int Arc_ListLink(struct ARC_FreelistMeta *A, struct ARC_FreelistMeta *B, struct ARC_FreelistMeta *combined);

/**
 * Initialize the given memory as a freelist.
 *
 * @param void *_base - The lowest address within the list.
 * @param void *_ciel - The highest address within the list + object_size.
 * @param int _object_size - The size of each object in bytes.
 * @param struct ARC_FrelistMeta *meta - The variable in which list information should be stored.
 * @return Error code (0: success)
 * */
int Arc_InitializeFreelist(void *_base, void *_ciel, int _object_size, struct ARC_FreelistMeta *meta);

#endif
