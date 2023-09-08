#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stddef.h>

#define ASSERT(cond) if (!(cond)) { \
			printf("Assertion %s failed (%s:%d)\n", #cond, __FILE__, __LINE__); \
			for (;;); \
		     }
#define ALIGN(v, a) ((v + (a - 1)) & ~(a - 1))

#define KMAP_ADDR 0x20000

void memset(void *dest, int value, size_t length);
void memcpy(void *dest, void *src, size_t length);
size_t strlen(char *a);
int strcmp(char *a, char *b);

#endif