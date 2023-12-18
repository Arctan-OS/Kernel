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

#include <global.h>
#include <arctan.h>
#include <stdint.h>
#include <stddef.h>
#include <mbi_struct.h>
#include <framebuffer/framebuffer.h>
#include <io/port.h>
#include <temp/interface.h>
#include <mm/vmm.h>
#include <mm/freelist.h>

struct Arc_FreelistMeta kernel_heap;
// TODO: The following string aligns the entire
//       kernel. I have no idea why, but I am
//       not satisfied with this solution.
const char *hello = "Hello World";

int kernel_main(uint32_t mbi_ptr, uint32_t hhdm_pml4_end) {
	printf("\nWelcome to 64-bit wonderland! Please enjoy your stay.\n");

	uint64_t base = ((uint64_t)hhdm_pml4_end + ARC_HHDM_VADDR);
	printf("%X\n", base);
	if (Arc_InitializeFreelist((void *)(uintptr_t)base, (void *)(uintptr_t)(base + PAGE_SIZE * 512), PAGE_SIZE, &kernel_heap) != 0) {
		printf("Failed to initialize kernel heap freelist\n");
		for (;;);
	}

	void *a = Arc_ListAlloc(&kernel_heap);
	printf("%X\n", a);
	printf("%X\n", Arc_ListAlloc(&kernel_heap));
	Arc_ListFree(&kernel_heap, a);
	printf("%X\n", Arc_ListAlloc(&kernel_heap));

//	parse_mbi(mbi_ptr);

	// TODO: The kernel is extremely unstable.
	//       Sometimes it triple faults because
	//       we print, sometimes because we access
	//       mapped memory, or sometimes cause
	//       why not.

	for (;;);

	return 0;
}
