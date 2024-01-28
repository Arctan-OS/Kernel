/**
 * @file freelist.c
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
#include <arctan.h>
#include <global.h>
#include <mm/freelist.h>
#include <stdint.h>
#include <interface/printf.h>

void *Arc_ListAlloc(struct ARC_FreelistMeta *meta) {
	// Get address, move head
	void *address = (void *)meta->head;
	meta->head = meta->head->next;

	return address;
}

void *Arc_ListContiguousAlloc(struct ARC_FreelistMeta *meta, int objects) {
	struct ARC_FreelistMeta to_free = { 0 };
	to_free.object_size = meta->object_size;
	to_free.base = meta->base;
	to_free.ciel = meta->ciel;

	// Number of objects currently allocated
	int object_count = 0;
	// Limit so we don't try to allocate all of memory
	int fails = 0;
	// Object allocated from previous iteration
	void *last_allocation = NULL;
	// Base of the contiguous allocation
	void *base = NULL;
	// First allocation
	void *bottom_allocation = NULL;
	// Current allocation
	void *allocation = NULL;

	while (object_count < objects) {
		allocation = Arc_ListAlloc(meta);

		if (to_free.head == NULL) {
			// Keep track of the first allocation
			// so if we fail, we are able to free
			// everything
			to_free.head = allocation;
			bottom_allocation = allocation;
			base = allocation;
		}

		if (last_allocation != NULL && abs((intptr_t)(last_allocation - allocation)) != (int64_t)meta->object_size) {
			// Keep track of this little contiguous allocation
			Arc_ListContiguousFree(&to_free, base, object_count + 1);

			// Move onto the next base
			base = allocation;
			fails++;
			object_count = 0;
			last_allocation = NULL;
		}

		if (fails >= 16) {
			break;
		}

		last_allocation = allocation;
		object_count++;
	}

	ARC_DEBUG(INFO, "B:%p A:%p\n", base, allocation);

	if (fails == 0) {
		// FIRST TRY!!!!
		// Just return, no pages to be freed
		return min(base, allocation);
	}

        // Free all pages to be freed
        struct ARC_FreelistNode *current = bottom_allocation;

	while (current->next != bottom_allocation) {
		struct ARC_FreelistNode *next = current->next;
		Arc_ListFree(meta, current);
		current = next;
	}

	return min(base, allocation);
}

void *Arc_ListFree(struct ARC_FreelistMeta *meta, void *address) {
	struct ARC_FreelistNode *node = (struct ARC_FreelistNode *)address;

	if (node == NULL || (node < meta->base || node > meta->ciel)) {
		// Node doesn't exist, it's below the freelist, or is above, return NULL
		return NULL;
	}

	// Update links
	node->next = meta->head;
	meta->head = node;

	return address;
}

void *Arc_ListContiguousFree(struct ARC_FreelistMeta *meta, void *address, int objects) {
	for (int i = objects - 1; i >= 0; i--) {
		Arc_ListFree(meta, address + (i * meta->object_size));
	}

	return address;
}

int Arc_ListLink(struct ARC_FreelistMeta *A, struct ARC_FreelistMeta *B, struct ARC_FreelistMeta *combined) {
	if (A->head != A->base && B->head != B->base) {
		// Both lists are dirty, cannot link lists
		// If only a single list is dirty, linking
		// should be fine
		return -2;
	}

	if (A->object_size != B->object_size) {
		// Object size mismatch, cannot link lists
		return -1;
	}

	combined->object_size = A->object_size;

	if ((uintptr_t)A->base < (uintptr_t)B->base) {
		// A is lower than B, link with A as base
		combined->base = A->base;
		combined->ciel = B->ciel;
		combined->head = A->head;

		return 0;
	}

	// B is lower than A, link with B as base
	combined->base = B->base;
	combined->ciel = B->ciel;
	combined->head = B->head;

	return 0;
}


int Arc_InitializeFreelist(void *_base, void *_ciel, int _object_size, struct ARC_FreelistMeta *meta) {
	struct ARC_FreelistNode *base = (struct ARC_FreelistNode *)_base;
	struct ARC_FreelistNode *ciel = (struct ARC_FreelistNode *)_ciel;

	// Store meta information
	meta->base = base;
	meta->head = base;
	meta->ciel = ciel;
	meta->object_size = _object_size;

	// Initialize the linked list
	for (; _base < _ciel; _base += _object_size) {
		struct ARC_FreelistNode *current = (struct ARC_FreelistNode *)_base;
		struct ARC_FreelistNode *next = (struct ARC_FreelistNode *)(_base + _object_size);

		current->next = next;
	}

	return 0;
}
