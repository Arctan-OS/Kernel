#ifndef UTIL_H
#define UTIL_H

#include <global.h>

void memset(void *dest, int value, size_t length);
void memcpy(void *dest, void *src, size_t length);
size_t strlen(char *a);
int strcmp(char *a, char *b);

#endif