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
	void *virtual = Arc_BuddyAlloc(&vmm_meta, size);

	// TODO: Mapping code

	return virtual;
}

void *Arc_FreeVMM(void *address) {
	void *virtual = Arc_BuddyFree(&vmm_meta, address);

	// TODO: Mapping code

	return virtual;
}

int Arc_InitVMM(void *addr, size_t size) {
	Arc_InitBuddy(&vmm_meta, addr, size, 12);

	void *a = NULL;
	void *b = NULL;
	printf("a=%p\n", (a = Arc_BuddyAlloc(&vmm_meta, 0x1000)));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x2400));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x10000));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x2400));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x1000));
	printf("b=%p\n", (b = Arc_BuddyAlloc(&vmm_meta, 0x2040)));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x1000));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x2400));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x12800));
	printf("Free a %p\n", Arc_BuddyFree(&vmm_meta, a));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x24000));
	printf("Free b %p\n", Arc_BuddyFree(&vmm_meta, b));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x1280));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x2400));
	printf("%p\n", Arc_BuddyAlloc(&vmm_meta, 0x2400));

	return 0;
}
