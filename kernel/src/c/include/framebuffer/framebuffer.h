/*
    Arctan - Operating System Kernel
    Copyright (C) 2023  awewsomegamer

    This file is apart of Arctan.

    Arctan is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <global.h>

#define MAX_BRANCH_FRAMEBUFFERS 16

struct framebuffer_context {
	void *physical_buffer;
	void *virtual_buffer; // NULL means (physical_buffer = virtual_buffer)
	int width;
	int height;
	int bpp;
	size_t size; // in bytes
};
extern struct framebuffer_context fb_current_context;

int init_branch_framebuffer(void *physical_buffer, void *virtual_buffer);
void draw_branch_framebuffers();
void init_master_framebuffer(void *physical_buffer, void *virtual_buffer, uint64_t width, uint64_t height, uint64_t bpp);

#endif
