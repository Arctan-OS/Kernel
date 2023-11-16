#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stddef.h>
#include <multiboot2.h>
#include <mm/alloc.h>

#define ALIGN(v, a) ((v + (a - 1)) & ~(a - 1))

#define PAGE_SIZE 0x1000
#define DEAD_64	  0xDEAD0ADDFFFFFFFF

extern uint8_t __KERNEL_END__;

extern struct pool_descriptor *kernel_heap_pool;

#endif