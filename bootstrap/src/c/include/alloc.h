#ifndef ALLOC_H
#define ALLOC_H

#include "global.h"
#include "multiboot2.h"

void *alloc();
void *free(void *address);
void init_allocator(struct multiboot_mmap_entry *entries, int count, uintptr_t kernel_end);

#endif
