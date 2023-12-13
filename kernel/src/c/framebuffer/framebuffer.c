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

#include <framebuffer/framebuffer.h>
#include <temp/interface.h>

struct framebuffer_context fb_current_context = { 0 };
struct framebuffer_context branches[MAX_BRANCH_FRAMEBUFFERS];
uint64_t branch_bmp = 0; // 0: free, 1: allocated

int init_branch_framebuffer(void *physical_buffer, void *virtual_buffer) {
	// Allocate a section of memory as a "branch" framebuffer.
	// Place newly created branch into the next free position in the list.

	return -1; // Return position of framebuffer in list (-1 = error)
}

void draw_branch_framebuffers() {
	// Walk through branch list 0 -> n, copying each branch into the master buffer.
	// An addition of all branches onto the main buffer.

	for (int i = 0; i < MAX_BRANCH_FRAMEBUFFERS; i++) {
		if (((branch_bmp >> i) & 1) == 0)
			continue;

		// TODO: Test the below assembly code
		switch (fb_current_context.bpp) {
		case 32: {
			__asm__("   mov rcx, %2;\
				mov rsi, %0;\
				mov rdi, %1;\
				0: mov rax, dword [rsi];\
				add dword [rdi], rax;\
				add rdi, 4;\
				add rsi, 4;\
				dec rcx;\
				jnz 0b;" : : 
				"m"(branches[i].virtual_buffer),
				"m"(fb_current_context.virtual_buffer),
				"r"((fb_current_context.size / 4)) : // Divide the size by the number of bytes we copy at a time (making dec rcx possible)
				"rax", "rcx", "rsi", "rdi");

			break;
		}

		case 24: {

			break;
		}

		case 16: {

			break;
		}

		case 8: {

			break;
		}
		}
	}

}

void init_master_framebuffer(void *physical_buffer, void *virtual_buffer, uint64_t width, uint64_t height, uint64_t bpp) {
	fb_current_context.physical_buffer = physical_buffer;
	fb_current_context.virtual_buffer = (virtual_buffer == NULL ? physical_buffer : virtual_buffer);
	// Ensure buffers are mapped
	fb_current_context.width = width;
	fb_current_context.height = height;
	fb_current_context.bpp = bpp;
	fb_current_context.size = width * height * (bpp / 8);

	printf("Setup master framebuffer P:%X V:%X (%dx%dx%d)\n", (uintptr_t)physical_buffer, (uintptr_t)virtual_buffer, width, height, bpp);
}
