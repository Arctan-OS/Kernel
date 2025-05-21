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

/**
 * A special unsigned 24-bit type
*/
typedef struct {
	uint32_t data : 24;
}__attribute__((packed)) uint24_s;

/// Convert a given AARRGGBB color to the given ordering
#define ARC_FB_GET_COLOR(__color, __ordering) \
	switch (__ordering) { \
		case 0: break; \
		case 1: break; \
	}

/// Draw a pixel to a give framebuffer of a given bpp
#define ARC_FB_DRAW(__addr, __x, __y, __bpp, __color)				\
	switch (__bpp) {							\
		case 32:							\
			*((uint32_t *)__addr + __x + __y) = (uint32_t)__color;	\
			break;							\
		case 24:							\
			((uint24_s *)__addr + __x + __y)->data = __color; 	\
			break;							\
		case 16:							\
			*((uint16_t *)__addr + __x + __y) = (uint16_t)__color;	\
			break;							\
	}

/**
 * The terminal meta structure
*/
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
	/// RW Lock
	ARC_GenericMutex lock;
};

/**
 * Put a character into the terminal stream
 * 
 * Will place the given character into the given terminal's
 * memory at the proper position and send it through to a COM
 * port if one is specified.
 *
 * @param struct ARC_TermMeta *term - Terminal to insert the character into.
 * @param char c - The character to insert.
*/
void term_putchar(struct ARC_TermMeta *term, char c);

/**
 * A function to draw the terminal to its framebuffer
 *
 * This function will display the given terminal to the framebuffer
 * that was attached to the terminal during initialization. In the event
 * Arc_FontFile is NULL, the builtin character ROM will be used.
 * 
 * @param struct ARC_TermMeta *term - The terminal to display
*/
void term_draw(struct ARC_TermMeta *term);

/**
 * Initialize the COM port if specified
 *
 * This function will initialize the COM port specified
 * at compile time. It does no checks to ensure it exists.
 * This is primarily for debugging, and non-debug builds
 * hopefully do not have the COM option specified.
*/
int term_bodge_init_uart();

#endif
