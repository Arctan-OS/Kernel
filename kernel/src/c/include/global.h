#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stddef.h>
#include <multiboot2.h>

#define ALIGN(v, a) ((v + (a - 1)) & ~(a - 1))

extern uint8_t __KERNEL_END__;

#endif