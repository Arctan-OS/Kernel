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

// Allocate one object in given list
// Return: non-NULL = success
void *Arc_ListAlloc(struct ARC_FreelistMeta *meta);

// Free given address in given list
// Return: non-NULL = success
void *Arc_ListFree(struct ARC_FreelistMeta *meta, void *address);

// Combine list A and list B into a single list, combined
// Return: 0 = success
// Return: -1 = object size mismatch
// Return: -2 = lists are dirty *
//
// *: The lists have already been allocated into, thus cannot be
//    combined nicely. If they were to be combined, data within
//    the higher list would be lost
int Arc_ListLink(struct ARC_FreelistMeta *A, struct ARC_FreelistMeta *B, struct ARC_FreelistMeta *combined);

// void *base - Base address for the freelist
// void *ciel - Cieling address (address of last usable object) for the freelist
// int object_size - The size of each object
// struct Arc_FreelistMeta *meta - Generated metadata for the freelist (KEEP AT ALL COSTS)
// Return: 0 = success
int Arc_InitializeFreelist(void *_base, void *_ciel, int _object_size, struct ARC_FreelistMeta *meta);

#endif
