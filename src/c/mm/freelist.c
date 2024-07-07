/**
 * @file freelist.c
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
 * Abstract freelist implementation.
*/
#include <lib/atomics.h>
#include <util.h>
#include <global.h>
#include <mm/freelist.h>
#include <stdint.h>
#include <stdio.h>

// Allocate one object in given list
// Return: non-NULL = success
void *Arc_ListAlloc(struct ARC_FreelistMeta *meta) {
	Arc_MutexLock(&meta->mutex);

	// Get address, mark as used
	void *address = (void *)meta->head;
	meta->head = meta->head->next;

	meta->free_objects--;

	Arc_MutexUnlock(&meta->mutex);

	return address;
}

void *Arc_ListContiguousAlloc(struct ARC_FreelistMeta *meta, uint64_t objects) {
	while (meta->free_objects < objects && meta->next != NULL) {
		meta = meta->next;
	}

	if (meta == NULL) {
		ARC_DEBUG(INFO, "Found meta is NULL\n");
		return NULL;
	}

	Arc_MutexLock(&meta->mutex);

	struct ARC_FreelistMeta to_free = { 0 };
	to_free.object_size = meta->object_size;
	to_free.base = meta->base;
	to_free.ciel = meta->ciel;

	// Number of objects currently allocated
	uint64_t object_count = 0;
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

	meta->free_objects -= objects;

	if (fails == 0) {
		Arc_MutexUnlock(&meta->mutex);

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

	Arc_MutexUnlock(&meta->mutex);

	return min(base, allocation);
}

// Free given address in given list
// Return: non-NULL = success
void *Arc_ListFree(struct ARC_FreelistMeta *meta, void *address) {
	Arc_MutexLock(&meta->mutex);

	struct ARC_FreelistNode *node = (struct ARC_FreelistNode *)address;

	if (node == NULL || (node < meta->base || node > meta->ciel)) {
		// Node doesn't exist, is below the freelist, or is above, return NULL
		Arc_MutexUnlock(&meta->mutex);
		return NULL;
	}

	// Mark as free
	node->next = meta->head;
	meta->head = node;

	meta->free_objects++;

	Arc_MutexUnlock(&meta->mutex);

	return address;
}

void *Arc_ListContiguousFree(struct ARC_FreelistMeta *meta, void *address, uint64_t objects) {
	Arc_MutexLock(&meta->mutex);

	for (int i = objects - 1; i >= 0; i--) {
		Arc_ListFree(meta, address + (i * meta->object_size));
	}

	meta->free_objects += objects;

	Arc_MutexUnlock(&meta->mutex);

	return address;
}

// Combine list A and list B into a single list, combined
// Return: 0 = success
// Return: -1 = object size mismatch
// Return: -2 = either list was NULL
int Arc_ListLink(struct ARC_FreelistMeta *A, struct ARC_FreelistMeta *B) {
	if (A == NULL || B == NULL) {
		return -2;
	}

	if (A->object_size != B->object_size) {
		// Object size mismatch, cannot link lists
		return -1;
	}

	Arc_MutexLock(&A->mutex);

	// Advance to the last list
	struct ARC_FreelistMeta *last = A;
	while (last->next != NULL) {
		Arc_MutexLock(&last->next->mutex);
		Arc_MutexUnlock(&last->mutex);
		last = last->next;
	}

	// Link A and B
	last->next = B;

	Arc_MutexUnlock(&last->mutex);

	return 0;
}

struct ARC_FreelistMeta *Arc_InitializeFreelist(void *_base, void *_ciel, uint64_t _object_size) {
	if (_base == NULL || _ciel == NULL || _object_size == 0) {
		return NULL;
	}

	struct ARC_FreelistMeta *meta = (struct ARC_FreelistMeta *)_base;

	Arc_MutexStaticInit(&meta->mutex);

	// Number of objects to accomodate meta
	int objects = sizeof(struct ARC_FreelistMeta) / _object_size;
	_base += objects * _object_size;

	struct ARC_FreelistNode *base = (struct ARC_FreelistNode *)_base;
	struct ARC_FreelistNode *ciel = (struct ARC_FreelistNode *)_ciel;

	// Store meta information
	meta->base = base;
	meta->head = base;
	meta->ciel = ciel;
	meta->object_size = _object_size;
	meta->free_objects = ((uint64_t)ciel - (uint64_t)base) / _object_size;

	// Initialize the linked list
	for (; _base < _ciel; _base += _object_size) {
		struct ARC_FreelistNode *current = (struct ARC_FreelistNode *)_base;
		struct ARC_FreelistNode *next = (struct ARC_FreelistNode *)(_base + _object_size);

		current->next = next;
	}

	return meta;
}
