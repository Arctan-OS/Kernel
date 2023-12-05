#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include "multiboot2.h"

#define ASSERT(cond) if (!(cond)) { \
			printf("Assertion %s failed (%s:%d)\n", #cond, __FILE__, __LINE__); \
			for (;;); \
		     }
#define ALIGN(v, a) ((v + (a - 1)) & ~(a - 1))

#define max(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

#define min(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })

#define printf printf_

void memset(void *dest, int value, size_t length);
void memcpy(void *dest, void *src, size_t length);
size_t strlen(char *a);
int strcmp(char *a, char *b);

extern struct multiboot_tag_framebuffer *framebuffer_tag;

#endif
