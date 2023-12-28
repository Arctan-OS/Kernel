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

#include <io/port.h>
#include <stdint.h>
#include <interface/printf.h>

const char *string = "Hello World";

int kernel_main(uint64_t boot) {
	for (int i = 0; i < 32; i++) {
		outb(0xE9, *(char *)(string + i));
//		outb(0xE9, *(string + i));
	}
//	printf("Hello\n");

	for (;;);

	return 0;
}
