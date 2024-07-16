/**
 * @file buddy.h
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
#ifndef ARC_MM_BUDDY_H
#define ARC_MM_BUDDY_H

#include <stddef.h>
#include <lib/atomics.h>
#include <mm/freelist.h>

struct ARC_BuddyMeta {
	/// Base of the allocator.
	void *base;
	/// Next free block of any size.
	void *next;
	/// Ceiling of the allocator.
	void *ceil;
	/// Allocator tree.
	void *tree;
	/// The lowest exponent of two an allocation can be aligned to.
	int lowest_exponent;
	/// Buddy list
	void *levels[64]; // 64 = system width
	/// Allocator for the tree
	struct ARC_FreelistMeta *allocator;
	/// Lock for the meta.
	ARC_GenericMutex mutex;
};

void *Arc_BuddyAlloc(struct ARC_BuddyMeta *meta, size_t size);
void *Arc_BuddyFree(struct ARC_BuddyMeta *meta, void *address);

/**
 * Create a buddy allocator
 *
 * @param struct ARC_BuddyMeta *meta - Meta of the allocator.
 * @param void *base - First allocatable address.
 * @param size_t size - Size of the first allocatable region (ensure this is aligned to the nearest power of 2).
 * @param int lowest_exponent - The exponent of the smallest region (ideally log2(system_width)).
 * @return zero upon success.
 *  */
int Arc_InitBuddy(struct ARC_BuddyMeta *meta, void *base, size_t size, int lowest_exponent);

#endif
