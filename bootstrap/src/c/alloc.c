#include "include/alloc.h"

struct free_node {
	struct free_node *next;
};

static struct free_node *head = NULL;

void *alloc() {
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
	for (int i = 0; i < size; i++) {
		uintptr_t j = (uintptr_t)entries[i].addr;
		uint8_t contains_kernel_end = ((kernel_end - j >= 0) && (kernel_end - j < entries[i].len));

		if ((!contains_kernel_end && j < kernel_end) || entries[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
			continue;
		}

		j = contains_kernel_end ? kernel_end + 0x1000 : j;

		if (head == NULL) {
			head = (struct free_node *)j;
		}

		struct free_node *current = (struct free_node *)j;

		for (uint64_t x = 0; x < entries[i].len; x += 0x1000) {
			current->next = (struct free_node *)(j + x + 0x1000);
			current = current->next;
		}
	}
}
