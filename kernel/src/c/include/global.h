#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stddef.h>
#include <multiboot2.h>

#define ALIGN(v, a) ((v + (a - 1)) & ~(a - 1))

#define PAGE_SIZE 0x1000

extern uint8_t __KERNEL_END__;

#endif