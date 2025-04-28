/**
 * @file terminal.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
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
#ifndef ARC_INTERFACE_TERMINAL_H
#define ARC_INTERFACE_TERMINAL_H

#include <stdint.h>
#include <lib/atomics.h>

typedef struct {
	uint32_t data : 24;
}__attribute__((packed)) uint24_s;

// Convert a given AARRGGBB color to the given ordering
#define ARC_FB_GET_COLOR(__color, __ordering) \
	switch (__ordering) { \
		case 0: break; \
		case 1: break; \
	}

#define ARC_FB_DRAW(__addr, __x, __y, __bpp, __color)			\
	switch (__bpp) {						\
		case 32:						\
			*((uint32_t *)__addr + __x + __y) = __color;	\
			break;						\
		case 24:						\
			((uint24_s *)__addr + __x + __y)->data = __color; \
			break;						\
		case 16:						\
			*((uint16_t *)__addr + __x + __y) = __color;	\
			break;						\
	}

struct ARC_TermMeta {
	/// Virtual address of framebuffer
	void *framebuffer;
	/// Framebuffer width in pixels
	int fb_width;
	/// Framebuffer height in pixels
	int fb_height;
	/// Framebuffer bpp
	int fb_bpp;
	/// Framebuffer pitch
	int fb_pitch;
	/// Current character x position
	int cx;
	/// Current character y position
	int cy;
	/// The BMP font
	uint8_t *font_bmp;
	/// Width of the font in pixels
	int font_width;
	/// Height of the font in pixels
	int font_height;
	/// Character memory of terminal
	char *term_mem;
	/// Width of the terminal in characters
	int term_width;
	/// Height of the terminal in characters
	int term_height;
	/// Length (in bytes) of the RX and TX buffers
	int rxtx_buf_len;
	/// Pointer to the RX buffer
	char *rx_buf;
	/// Pointer to the next character in the RX buffer
	int rx_buf_idx;
	/// Pointer to the TX buffer
	char *tx_buf;
	/// Pointer to the next character in the TX buffer
	int tx_buf_idx;
	/// RW Lock
	ARC_GenericMutex lock;
};

void term_putchar(struct ARC_TermMeta *term, char c);
void term_draw(struct ARC_TermMeta *term);
int term_push(struct ARC_TermMeta *term, int rx, char c);
char term_pop(struct ARC_TermMeta *term, int rx);

#endif
