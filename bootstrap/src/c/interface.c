#include "include/interface.h"

#define WIDTH 80
#define HEIGHT 25

int char_x = 0;
int char_y = 0;
uint8_t *screen = (uint8_t *)0xB8000;

void putc(char c) {
	switch (c) {
	case '\n':
		char_y++;
	case '\r':
		char_x = 0;
		break;

	default:
		*(screen + (char_x + (char_y * WIDTH)) * 2) = c;
		char_x++;

		if (char_x >= WIDTH) {
			char_x = 0;
			char_y++;
		}

		
		break;
	}

	E9_HACK(c)
}

void puts(char *s) {
	while (*s)
		putc(*(s++));
}

const char *ALNUM = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
void putn(uint32_t val, uint8_t base) {
	if (val / base != 0)
		putn(val / base, base);

	putc(*(ALNUM + (val % base)));
}

void printf(const char *form, ...) {
	puts(form);
}
