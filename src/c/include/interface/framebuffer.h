/**
 * @file framebuffer.h
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
#ifndef ARC_INTERFACE_FRAMEBUFFER_H
#define ARC_INTERFACE_FRAMEBUFFER_H

#include <stdint.h>

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

struct ARC_FramebufferMeta {
	void *vaddr;
	void *paddr;
	int width;
	int height;
	int bpp;
	int lock;
};


int init_framebuffer(struct ARC_FramebufferMeta *framebuffer);

#endif
