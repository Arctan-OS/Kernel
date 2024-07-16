/**
 * @file vmm.c
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
#include <mm/vmm.h>
#include <mm/buddy.h>

static struct ARC_BuddyMeta vmm_meta = { 0 };

void *Arc_AllocVMM(size_t size) {
	return NULL;
}

void *Arc_FreeVMM(void *address) {
	return NULL;
}

int Arc_InitVMM(void *addr, size_t size) {
	// Initialize buddy allocator for region
	// addr -> addr + size. This lets the caller
	// specify where to put the virtual memory space
	Arc_InitBuddy(&vmm_meta, addr, size, 3);

	void *a = NULL;
	printf("%p\n", (a = Arc_BuddyAlloc(&vmm_meta, 100)));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 100));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 100));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 100));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 24));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 24));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 24));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 128));
	Arc_BuddyFree(&vmm_meta, a);
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 24));

	return 0;
}
