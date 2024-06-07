#include <mm/slab.h>
#include <util.h>
#include <global.h>

int strcmp(char *a, char *b) {
	uint8_t *ua = (uint8_t *)a;
	uint8_t *ub = (uint8_t *)b;

	size_t max = strlen(a);
	size_t b_len = strlen(b);

	if (max > b_len) {
		max = b_len;
	}

	int i = 0;
	for (; i < max - 1; i++) {
		if (ua[i] != ub[i] || ua[i] == 0 || ub[i] == 0) {
			break;
		}
	}

	return ua[i] - ub[i];
}

int strncmp(char *a, char *b, size_t len) {
	uint8_t *ua = (uint8_t *)a;
	uint8_t *ub = (uint8_t *)b;

	int i = 0;
	for (; i < len - 1; i++) {
		if (ua[i] != ub[i] || ua[i] == 0 || ub[i] == 0) {
			break;
		}
	}

	return ua[i] - ub[i];
}

void memset(void *a, uint8_t value, size_t size) {
	for (size_t i = 0; i < size; i++) {
		*((uint8_t *)a + i) = value;
	}
}

void memcpy(void *a, void *b, size_t size) {
	for (size_t i = 0; i < size; i++) {
		*(uint8_t *)(a + i) = *(uint8_t *)(b + i);
	}
}

size_t strlen(char *a) {
	size_t i = 0;

	while (*(a + i) != 0) {
		i++;
	}

	return i;
}

char *strdup(char *a) {
	size_t len = strlen(a);

	char *b = Arc_SlabAlloc(len + 1);
	memset(b, 0, len + 1);
	memcpy(b, a, len);

	return b;
}

char *strndup(char *a, size_t n) {
	char *b = Arc_SlabAlloc(n + 1);
	memset(b, 0, n + 1);
	memcpy(b, a, n);

	return b;
}

static const char *NUMBERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

long strtol(char *string, char **end, int base) {
	char c = *string;
	int offset = 0;
	long number = 0;

	while (c < *(NUMBERS + base)) {
		number *= base;

		if (c >= '0' && c <= '9') {
			number += c - '0';
		} else if (c >= 'A' && c <= 'Z') {
			number += c- 'A';
		}

		c = *(string + offset++);
	}

	*end = (char *)(string + offset);

	return number;
}
