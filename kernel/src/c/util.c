#include <mm/allocator.h>
#include <util.h>
#include <global.h>

int strcmp(char *a, char *b) {
	int sum = 0;
	while (*a != 0) {
		sum += *a - *b;

		a++;
		b++;
	}

	return sum;
}

int strncmp(char *a, char *b, size_t len) {
	size_t i = 0;
	int sum = 0;

	while (i < len && *a != 0 && *b != 0) {
		sum += a[i] - b[i];
 		i++;
	}

	return sum + (len - i);
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
