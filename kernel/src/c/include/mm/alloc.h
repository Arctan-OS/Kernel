#ifndef ALLOC_H
#define ALLOC_H

// Provide functions which utilize pmm, vmm, and mapper functions
// to allocate an address range.

#include <global.h>

void *alloc_kpage(size_t pages);

void init_allocator();

#endif