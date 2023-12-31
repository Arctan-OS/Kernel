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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include "multiboot2.h"

#define ASSERT(cond) if (!(cond)) { \
			printf("Assertion %s failed (%s:%d)\n", #cond, __FILE__, __LINE__); \
			for (;;); \
		     }
#define ALIGN(v, a) ((v + (a - 1)) & ~(a - 1))

#define max(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

#define min(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })

#define printf printf_

void memset(void *dest, int value, size_t length);
void memcpy(void *dest, void *src, size_t length);
size_t strlen(char *a);
int strcmp(char *a, char *b);

extern struct multiboot_tag_framebuffer *framebuffer_tag;

#endif
