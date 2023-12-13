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

#include "include/alloc.h"
#include "include/interface.h"

struct free_node {
	struct free_node *next;
};

static struct free_node *head = NULL;

void *alloc() {
	if (head == NULL) {
		printf("Ran out of memory!\n");
		return NULL;
	}

	void *address = (void *)head;
	head = head->next;
	return address;
}

void *free(void *address) {
	struct free_node *current = (struct free_node *)address;
	current->next = head;
	head = current;

	return address;
}

void init_allocator(struct multiboot_mmap_entry *entries, int size, uintptr_t kernel_end) {
	struct free_node *current = NULL;

	for (int i = 0; i < size; i++) {
		uint64_t j = entries[i].addr;

		uint8_t contains_kernel_end = ((kernel_end >= j) && (kernel_end < entries[i].len + j));

		if ((!contains_kernel_end && j < kernel_end) || entries[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
			continue;
		}

		printf("Found free region (MMAP Entry %d), %s, initializing as free\n", i + 1, (contains_kernel_end ? "which contains the end of the kernel" : "which does not contain the end of the kernel"));

		j = ALIGN(contains_kernel_end ? kernel_end + 0x1000 : j, 0x1000);

		if (head == NULL) {
			head = (struct free_node *)j;
			current = head;
		}

		for (uint64_t x = 0; x < entries[i].len; x += 0x1000) {
			if (x + 0x1000 >= entries[i].len) {
				current->next = NULL;
			} else {
				current->next = (struct free_node *)(j + x + 0x1000);
				current = current->next;
			}
		}
	}

	printf("Allocator head is located at 0x%"PRIX64"\n", (uint64_t)head);
	printf("Allocator tail is located at 0x%"PRIX64"\n", (uint64_t)current);
}
