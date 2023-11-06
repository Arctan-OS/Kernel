#include "include/global.h"

void memset(void *dest, int value, size_t length) {
	for (size_t i = 0; i < length; i++) {
		*((uint8_t *)dest + i) = value;
	}
}

void memcpy(void *dest, void* src, size_t length) {
	for (size_t i = 0; i < length; i++) {
		*((uint8_t *)dest + i) = *((uint8_t *)src + i);
	}
}

size_t strlen(char *a) {
	size_t sz = 0;
	
	while (*(a + sz)) {
		sz++;
	}

	return sz;
}

int strcmp(char *a, char *b) {
	size_t b_len = strlen(b);

	if (strlen(a) != b_len) {
		return 1;
	}

	for (int i = 0; i < b_len; i++) {
		if (*(b + i) != *(a + i)) {
			return 1;
		}
	}
	
	return 0;
}
