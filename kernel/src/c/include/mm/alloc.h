#ifndef ALLOC_H
#define ALLOC_H

// Provide functions which utilize pmm, vmm, and mapper functions
// to allocate an address range.

#include <stdint.h>
#include <stddef.h>

struct free_node {
	struct free_node *next;
};

struct pool_descriptor {
	struct free_node *pool_base;
	struct free_node *pool_head;
	size_t object_size;
};

void *alloc_pages(struct pool_descriptor *pool, size_t pages);
void *free_pages(struct pool_descriptor *pool, void *address, size_t pages);

struct pool_descriptor init_pool(void *base, size_t object_size, size_t objects);

#endif