/**
 * @file terminal.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is part of Arctan.
 *
 * Arctan is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
*/
#include "fs/vfs.h"
#include "mm/allocator.h"
#include <global.h>
#include <interface/terminal.h>
#include <arctan.h>
#include <util.h>

void Arc_TermPutChar(struct ARC_TermMeta *term, char c) {
	if (term->cy >= term->term_height) {
		memcpy(term->term_mem, term->term_mem + term->term_width, (term->term_height - 1) * term->term_width);
		memset(term->term_mem + (term->term_height - 1) * term->term_width, 0, term->term_width);
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

	size_t size_in_bytes = (term->font_width * term->font_height) / 8;

	uint8_t *data = Arc_SlabAlloc(size_in_bytes);

	for (int y = 0; y < term->term_height; y++) {
		for (int x = 0; x < term->term_width; x++) {
			int sx = x * term->font_width;
			int sy = y * term->font_height;

			char c = term->term_mem[y * term->term_width + x];

			memset(data, 0, size_in_bytes);
			Arc_SeekVFS(Arc_FontFile, (c * size_in_bytes), ARC_VFS_SEEK_SET);
			Arc_ReadVFS(data, 1, size_in_bytes, Arc_FontFile);

			for (int i = 0; i < term->font_height; i++) {
				int rx = 0;
				for (int j = term->font_width - 1; j >= 0; j--) {
					if (((data[i] >> j) & 1) == 1 && c != 0) {
						*((uint32_t *)term->framebuffer + (i + sy) * term->fb_width + (sx + rx)) = 0x00FFFFFF;
					}

					rx++;
				}
			}
		}
	}

	Arc_SlabFree(data);
}

// Returns error code (0: success, 1: could not enqueue)
int Arc_TermPush(struct ARC_TermMeta *term, int rx, char c) {
	char *buffer = term->tx_buf;
	int ptr = term->tx_buf_idx;

	if (rx) {
		buffer = term->rx_buf;
		ptr = term->rx_buf_idx;
	}

	if (ptr >= term->rxtx_buf_len) {
		// Cannot insert another character
		return 1;
	}

	buffer[ptr++] = c;

	if (rx) {
		term->rx_buf_idx = ptr;

		return 0;
	}

	term->tx_buf_idx = ptr;

	return 0;
}

char Arc_TermPop(struct ARC_TermMeta *term, int rx) {
	char *buffer = term->tx_buf;
	int ptr = term->tx_buf_idx;

	if (rx) {
		buffer = term->rx_buf;
		ptr = term->rx_buf_idx;
	}

	if (ptr < 0) {
		return 0;
	}

	char c = *buffer;

	memcpy(buffer, buffer + 1, term->rxtx_buf_len - 1);

	if (rx) {
		term->rx_buf_idx--;

		if (term->rx_buf_idx < 0) {
			term->rx_buf_idx = 0;
		}

		return c;
	}

	term->tx_buf_idx--;

	if (term->tx_buf_idx < 0) {
		term->tx_buf_idx = 0;
	}

	return c;
}
