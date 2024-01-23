/**
 * @file terminal.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is apart of Arctan.
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

struct ARC_TermMeta {
	void *framebuffer; // Virtual address of framebuffer
	int fb_width; // Framebuffer width in pixels
	int fb_height; // Framebuffer height in pixels
	int fb_bpp; // Framebuffer bpp
	int fb_pitch; // Framebuffer pitch
	int cx; // Current character x position
	int cy; // Current character y position
	uint8_t *font_bmp; // The BMP font
	int font_width; // Width of the font in pixels
	int font_height; // Height of the font in pixels
	char *term_mem; // Character memory of terminal
	int term_width; // Width of the terminal in characters
	int term_height; // Height of the terminal in characters
};

void Arc_TermPutChar(struct ARC_TermMeta *term, char c);
void Arc_TermDraw(struct ARC_TermMeta *term);

#endif
