#include <global.h>
#include <mm/freelist.h>
#include <stdint.h>
#include <stdio.h>

// Allocate one object in given list
// Return: non-NULL = success
void *Arc_ListAlloc(struct ARC_FreelistMeta *meta) {
	// Get address, mark as used
	void *address = (void *)meta->head;
	meta->head = meta->head->next;

	return address;
}

// Free given address in given list
// Return: non-NULL = success
void *Arc_ListFree(struct ARC_FreelistMeta *meta, void *address) {
	struct ARC_FreelistNode *node = (struct ARC_FreelistNode *)address;

	if (node == NULL || (node < meta->base || node > meta->ciel)) {
		// Node doesn't exist, is below the freelist, or is above, return NULL
		return NULL;
	}

	// Mark as free
	node->next = meta->head;
	meta->head = node;

	return address;
}

// Combine list A and list B into a single list, combined
// Return: 0 = success
// Return: -1 = object size mismatch
// Return: -2 = lists are dirty *
//
// *: The lists have already been allocated into, thus cannot be
//    combined nicely. If they were to be combined, data within
//    the higher list would be lost
int Arc_ListLink(struct ARC_FreelistMeta *A, struct ARC_FreelistMeta *B, struct ARC_FreelistMeta *combined) {
	if (A->head != A->base || B->head != B->base) {
		// Lists are dirty, cannot link lists
		return -2;
	}

	if (A->object_size != B->object_size) {
		// Object size mismatch, cannot link lists
		return -1;
	}

	combined->object_size = A->object_size;

	if ((uintptr_t)A < (uintptr_t)B) {
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

// void *base - Base address for the freelist
// void *ciel - Cieling address (address of last usable object) for the freelist
// int object_size - The size of each object
// struct Arc_FreelistMeta *meta - Generated metadata for the freelist (KEEP AT ALL COSTS)
// Return: 0 = success
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
