#include "include/interface.h"
#include "include/global.h"

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

	case '\t':
		char_x += 8;

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

	if (char_y >= HEIGHT) {
		memcpy(screen, screen + (WIDTH * 2), WIDTH * HEIGHT * 2);
		char_y--;
	}

	// Move cursor
	uint16_t pos = char_y * WIDTH + char_x;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));

	E9_HACK(c)
}

void puts(char *s) {
	while (*s)
		putc(*(s++));
}

const char *ALNUM = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int __putn(uint32_t val, uint8_t base, int depth, int org_depth) {
	int r = 0;
	if (val / base != 0 || depth > 0)
		r = __putn(val / base, base, (depth == -1) ? (-1) : (depth - 1), org_depth);

	if (r > org_depth && org_depth > 0)
		return r + 1;

	putc(*(ALNUM + (val % base)));

	return r + 1;
}

int putn(uint32_t val, uint8_t base, int depth) {
	return __putn(val, base, depth, depth);
}

void printf(const char *form, ...) {
	va_list args;
	va_start(args, form);

	uint8_t found_arg = 0;

	char aux_chars[16];
	int aux_idx = 0;

	while (*form) {
		if (*form == '%') {
			arg_seek:;

			form++;

			switch (*form) {
			case 'd': {
				putn(va_arg(args, uint32_t), 10, -1);
				found_arg = 1;

				break;
			}

			case 'X': {
				int depth = -1;
				if (aux_idx > 0) {
					depth = 0;
					int multiplier = 1;

					for (int i = 0; i < aux_idx; i++) {
						depth += (aux_chars[i] - '0') * multiplier;
						multiplier *= 10;
					}

					depth--;
				}

				putn(va_arg(args, uint32_t), 16, depth);
				found_arg = 1;

				break;
			}

			case 's': {
				puts(va_arg(args, char*));
				found_arg = 1;

				break;
			}

			case 'c': {
				putc(va_arg(args, int));
				found_arg = 1;

				break;
			}

			default: {
				aux_chars[aux_idx++] = *form;

				goto arg_seek;
			}
			}
		}

		if (!found_arg)
			putc(*form);

		memset(aux_chars, 0, 16);
		aux_idx = 0;

		found_arg = 0;
		form++;
	}
}
