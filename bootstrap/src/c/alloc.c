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

	printf("Allocated %p\n", address);

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

		printf("Freeing region: %d %d\n", i, contains_kernel_end);

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

	printf("Allocator head = %X\n", head);
}
