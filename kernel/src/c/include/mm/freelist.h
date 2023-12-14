#ifndef ARC_FREELIST_H
#define ARC_FREELIST_H

#include <stdint.h>
#include <stddef.h>

struct Arc_FreelistNode {
	struct Arc_FreelistNode *next;
};

struct Arc_FreelistMeta {
	struct Arc_FreelistNode *head;
	struct Arc_FreelistNode *base;
	struct Arc_FreelistNode *ciel;
	int object_size;
};

// Allocate one object in given list
// Return: non-NULL = success
void *Arc_ListAlloc(struct Arc_FreelistMeta *meta);

// Free given address in given list
// Return: non-NULL = success
void *Arc_ListFree(struct Arc_FreelistMeta *meta, void *address);

// void *base - Base address for the freelist
// void *ciel - Cieling address (address of last usable object) for the freelist
// int object_size - The size of each object
// struct Arc_FreelistMeta *meta - Generated metadata for the freelist (KEEP AT ALL COSTS)
// Return: 0 = success
int *Arc_InitializeFreelist(void *_base, void *_ciel, int _object_size, struct Arc_FreelistMeta *meta);

#endif
