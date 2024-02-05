/**
 * @file util.c
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
#include <util.h>

int strcmp(char *a, char *b) {
	int sum = 0;
	while (*a != 0) {
		sum += *a - *b;

		a++;
		b++;
	}

	return sum;
}

int memcpy(void *a, void *b, size_t size) {
	size_t i = 0;
	while (i < size) {
		*(uint8_t *)(a + i) = *(uint8_t *)(b + i);
		if (i < 6) {
			ARC_DEBUG(INFO, "(%p) %02X (%p )%02X\n", b, *(uint8_t *)(b + i), a,*(uint8_t *)(a + i));
		}
		i++;
	}

	return 0;
}

void memset(void *mem, uint8_t value, size_t size) {
	for (size_t i = 0; i < size; i++) {
		*(uint8_t *)(mem + i) = value;
	}
}
