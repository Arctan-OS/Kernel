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

#include "include/global.h"

void memset(void *dest, int value, size_t length) {
	for (size_t i = 0; i < length; i++) {
		*((uint8_t *)dest + i) = value;
	}
}

void memcpy(void *dest, void* src, size_t length) {
	for (size_t i = 0; i < length; i++) {
		*((uint8_t *)dest + i) = *((uint8_t *)src + i);
	}
}

size_t strlen(char *a) {
	size_t sz = 0;
	
	while (*(a + sz)) {
		sz++;
	}

	return sz;
}

int strcmp(char *a, char *b) {
	size_t b_len = strlen(b);

	if (strlen(a) != b_len) {
		return 1;
	}

	for (int i = 0; i < b_len; i++) {
		if (*(b + i) != *(a + i)) {
			return 1;
		}
	}
	
	return 0;
}
