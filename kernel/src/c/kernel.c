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

#include <stdint.h>
#include <stddef.h>
#include <mbi_struct.h>
#include <framebuffer/framebuffer.h>
#include <io/port.h>
#include <temp/interface.h>
#include <mm/vmm.h>
#include <mm/alloc.h>

struct pool_descriptor *kernel_heap_pool;
	const char *string = "Hello World\n";

int kernel_main(uint32_t mbi_ptr) {
	printf("\nWelcome to 64-bit wonderland! Please enjoy your stay.\n");

	parse_mbi(mbi_ptr);

	//*kernel_heap_pool = init_pool((void *)&__KERNEL_END__, PAGE_SIZE, 128);

	int t = 0;
	uint8_t sw = 1;

	while (1) {
		for (int i = 0; i < fb_current_context.height; i++) {
			for (int j = 0; j < fb_current_context.width; j++) {
				int value = (i - j + t);

				int x = (j + value) % fb_current_context.width;
				int y = (i + value) % fb_current_context.height;

				int xf = ((x * x) / x);
				int yf = ((y * y) / y);

				if (xf < 0 || yf < 0) {
					continue;
				}

				*((uint32_t *)fb_current_context.virtual_buffer + (yf * fb_current_context.width) + xf) += (value + t * (-1 * sw)) * (sw + 1);
			}
		}

		if (t < 0x400 && sw == 1) {
			t++;
		} else if (t >= 0x1000 || sw == 0) {
			t--;
			sw = (t <= 2);
		}
	}

	return 0;
}
