#include "global.h"
#include <interface/terminal.h>
#include <arctan.h>

void memcpy(void *a, void *b, size_t size) {
	for (size_t i = 0; i < size; i++) {
		*(uint8_t *)(a + i) = *(uint8_t *)(b + i);
	}
}

void Arc_TermPutChar(struct ARC_TermMeta *term, char c) {
	if (term->cy >= term->term_height) {
		memcpy(term->term_mem, term->term_mem + term->term_width, (term->term_height - 1) * term->term_width);
		term->cy = term->term_height - 1;
	}

	switch (c) {
	case '\n': {
		term->cy++;
		term->cx = 0;

		break;
	}

	default: {
		if (term->term_mem != NULL) {
			term->term_mem[term->cy * term->term_width + term->cx] = c;
		}

		term->cx++;

		if (term->cx >= term->term_width) {
			term->cy++;
			term->cx = 0;
		}

		break;
	}
	}
}

void Arc_TermDraw(struct ARC_TermMeta *term) {
	if (term->framebuffer == NULL) {
		return;
	}

	for (int y = 0; y < term->term_height; y++) {
		for (int x = 0; x < term->term_width; x++) {
			int sx = x * term->font_width;
			int sy = y * term->font_height;

			char c = term->term_mem[y * term->term_width + x];

			uint8_t *data = term->font_bmp + (c * term->font_height);

			for (int i = 0; i < term->font_height; i++) {
				for (int j = term->font_width - 1; j >= 0; j--) {
					if (((data[i] >> j) & 1) == 1 && c != 0) {
						*((uint32_t *)term->framebuffer + (i + sy) * term->fb_width + (j + sx)) = 0x00FFFFFF;
					}
				}
			}
		}
	}
}

// TODO: Implement terminal input FIFO push and pop
