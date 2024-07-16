/**
 * @file buddy.c
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

#include <mm/buddy.h>
#include <mm/pmm.h>
#include <global.h>
#include <stdbool.h>

struct buddy_node {
	struct buddy_node *left;
	struct buddy_node *right;
	struct buddy_node *buddy;
	void *base;
	size_t size;
	int exponent; // Encodes node's usage (-: Allocated, +: Free)
	ARC_GenericMutex mutex;
};

int quick_log2(size_t number, bool round) {
	int exp = 0;
	size_t estimate = 2;

	while (number > 1) {
		exp++;
		estimate <<= 1;
		number >>= 1;
	}

	if (round && estimate < number) {
		// 2^exp < number, caller wants 2^exp
		// to fit number
		exp++;
	}

	return exp;
}

int split(struct ARC_BuddyMeta *meta, struct buddy_node *node) {
	// Expected: node is locked

	if (node == NULL || node->exponent < 0) {
		return -1;
	}

	if (meta->lowest_exponent == node->exponent) {
		return -2;
	}

	struct buddy_node *new = (struct buddy_node *)Arc_ListAlloc(meta->allocator);
	new->exponent = node->exponent - 1;
	new->base = node->base;
	new->size = node->size / 2;
	Arc_MutexStaticInit(&node->mutex);

	node->left = new;

	new = (struct buddy_node *)Arc_ListAlloc(meta->allocator);
	new->exponent = node->exponent - 1;
	new->base = node->base + (node->size / 2);
	new->size = node->size / 2;
	Arc_MutexStaticInit(&node->mutex);

	node->right = new;

	return 0;
}

void *Arc_BuddyAlloc(struct ARC_BuddyMeta *meta, size_t size) {
	if (meta == NULL || size == 0) {
		return NULL;
	}

	struct buddy_node *current = (struct buddy_node *)meta->tree;

	int exponent = quick_log2(size, 1);

	if (exponent > abs(current->exponent) || -exponent == current->exponent) {
		// Requested size it too large, or perfect but this top node is
		// already allocated
		return NULL;
	}

	Arc_MutexLock(&current->mutex);

	struct buddy_node *last = current;
	while (current != NULL && (abs(current->exponent) > exponent || -exponent == current->exponent)) {
		// Unlock the last last mutex, and advance last
		if (last != current) {
			Arc_MutexUnlock(&last->mutex);
		}
	
		last = current;

		if (current->exponent < 0) {
			// Current block is allocated, continue to its buddy
			// if possible
			if (current->buddy != NULL) {
				Arc_MutexLock(&current->buddy->mutex);
			}

			current = current->buddy;
			continue;
		}

		// Current block is still larger than the desired allocation
		// go lower
		if (current->left != NULL) {
			Arc_MutexLock(&current->left->mutex);
		}

		current = current->left;
	}

	current = last;

	// Last is invalid, return allocation failure
	if (current == NULL) {
		return NULL;
	} else if (current->exponent < 0) {
		Arc_MutexUnlock(&last->mutex);
		return NULL;
	}

	// Current is still larger than the target,
	// split current
	while (current->exponent > exponent) {
		if (split(meta, current) != 0) {
			Arc_MutexUnlock(&current->mutex);
			return NULL;
		}

		Arc_MutexLock(&current->left->mutex);
		Arc_MutexUnlock(&current->mutex);
		current = current->left;
	}

	current->exponent *= -1;
	Arc_MutexUnlock(&current->mutex);

        return last->base;
}

void *Arc_BuddyFree(struct ARC_BuddyMeta *meta, void *address) {
	if (meta == NULL) {
		return NULL;
	}

	struct buddy_node *current = (struct buddy_node *)meta->tree;

	Arc_MutexLock(&current->mutex);

	while (current != NULL && current->base != address) {
		if (address > current->base) {
			// Addresses to the left are higher in memory
			if (current->left != NULL) {
				Arc_MutexLock(&current->left->mutex);
			}
			Arc_MutexUnlock(&current->mutex);
		
			current = current->left;
		
			continue;
		}

		// Addresses to the right are lower in memory
		if (current->right != NULL) {
			Arc_MutexLock(&current->right->mutex);
		}
		Arc_MutexUnlock(&current->mutex);

		current = current->right;
	}

	if (current == NULL) {
		return NULL;
	}

	current->exponent *= -1;

	Arc_MutexUnlock(&current->mutex);

        return 0;
}

int Arc_InitBuddy(struct ARC_BuddyMeta *meta, void *base, size_t size, int lowest_exponent) {
        ARC_DEBUG(INFO, "Initializing new buddy allocator (%llu bytes, lowest exponent 2^%d bytes) at %p\n", size, lowest_exponent, base);

	meta->base = base;
	meta->ceil = base + size;
	meta->lowest_exponent = lowest_exponent;
	meta->next = NULL;
	Arc_MutexStaticInit(&meta->mutex);

	// 128 pages is quite a lot, might be worth while to calculate
	// the maximum size of the tree
	uint64_t alloc_base = (uint64_t)Arc_ContiguousAllocPMM(128);
	meta->allocator = Arc_InitializeFreelist(alloc_base, alloc_base + 128 * PAGE_SIZE, sizeof(struct buddy_node));

	// Approximate log2(size)
	int max_exponent = quick_log2(size, 0);

	struct buddy_node *head = (struct buddy_node *)Arc_ListAlloc(meta->allocator);
	head->exponent = max_exponent;
	head->base = base;
	head->size = size;
	Arc_MutexStaticInit(&head->mutex);

	meta->tree = (void *)head;

        return 0;
}
