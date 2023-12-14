/*
    Arctan - Operating System Kernel
    Copyright (C) 2023  awewsomegamer

    This file is apart of Arctan.

    Arctan is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <global.h>
#include <mm/freelist.h>

// Allocate one object in given list
// Return: non-NULL = success
void *Arc_ListAlloc(struct Arc_FreelistMeta *meta) {
	// Get address, mark as used
	void *address = (void *)meta->head;
	meta->head = meta->head->next;

	return address;
}

// Free given address in given list
// Return: non-NULL = success
void *Arc_ListFree(struct Arc_FreelistMeta *meta, void *address) {
	struct Arc_FreelistNode *node = (struct Arc_FreelistNode *)address;

	// Node doesn't exist, is below the freelist, or is above, return NULL
	if (node == NULL || (node < meta->base || node > meta->ciel)) {
		return NULL;
	}

	// Mark as free
	node->next = meta->head;
	meta->head = node;

	return address;
}

// void *base - Base address for the freelist
// void *ciel - Cieling address (address of last usable object) for the freelist
// int object_size - The size of each object
// struct Arc_FreelistMeta *meta - Generated metadata for the freelist (KEEP AT ALL COSTS)
// Return: 0 = success
int *Arc_InitializeFreelist(void *_base, void *_ciel, int _object_size, struct Arc_FreelistMeta *meta) {
	struct Arc_FreelistNode *base = (struct Arc_FreelistNode *)_base;
	struct Arc_FreelistNode *ciel = (struct Arc_FreelistNode *)_ciel;

	// Store meta information
	meta->base = base;
	meta->head = base;
	meta->ciel = ciel;
	meta->object_size = _object_size;

	// Initialize the linked list
	for (; _base < _ciel; _base += _object_size) {
		struct Arc_FreelistNode *current = (struct Arc_FreelistNode *)_base;
		struct Arc_FreelistNode *next = (struct Arc_FreelistNode *)(_base + _object_size);

		current->next = next;
	}

	return 0;
}
