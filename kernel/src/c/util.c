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

void memcpy(void *a, void *b, size_t size) {
	for (size_t i = 0; i < size; i++) {
		*(uint8_t *)(a + i) = *(uint8_t *)(b + i);
	}
}

static const char *NUMBERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

long strtol(char *string, char *end, int base) {
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

	end = (char *)(string + offset);

	return number;
}
