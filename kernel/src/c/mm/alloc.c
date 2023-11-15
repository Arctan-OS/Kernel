#include <mm/alloc.h>
#include <global.h>
#include <temp/interface.h>

struct free_node {
	struct free_node *next;
};

struct pool_descriptor {
	struct free_node *physical_base;
	struct free_node *virtual_base;
};

struct pool_descriptor *current_descriptor = NULL;

struct free_node *kernel_pool_base = NULL;

// If kernel_pool_head is NULL, that means
// we have allocated all pages in the kernel
// pool.
struct free_node *kernel_pool_head = NULL;

void *alloc_kpages(size_t pages) {
	if (pages == 0) {
		return NULL;
	}

	void *address = (void *)kernel_pool_head;

	size_t count = 1;

	while (kernel_pool_head != NULL && count <= pages) {
		size_t difference = (size_t)((uintptr_t)kernel_pool_head->next - (uintptr_t)kernel_pool_head);

		kernel_pool_head = kernel_pool_head->next;

		if (difference == PAGE_SIZE) {
			count++;
			continue;
		}

		count = 1;
		address = (void *)kernel_pool_head;
	}

	if (count < pages) {
		printf("Failed to allocate %d kernel pages\n", pages);
		return NULL;
	}
	
	return address;
}

void *free_kpages(void *address, size_t pages) {
	if (pages == 0) {
		return NULL;
	}

	struct free_node *current = (struct free_node *)address;

	for (size_t i = 0; i < pages; i++) {
		current = (struct free_node *)((uintptr_t)address + i * PAGE_SIZE);
		current->next = (struct free_node *)((uintptr_t)current + PAGE_SIZE);
		current = (struct free_node *)((uintptr_t)current + PAGE_SIZE);
	}

	current->next = kernel_pool_head;
	kernel_pool_head = (struct free_node *)address;

	return address;
}

int switch_to_pool(struct pool_descriptor *pool) {
	// Map pool into memory
	// Update top pointer

	return 0;
}

void init_allocator() {
	kernel_pool_head = (void *)(&__KERNEL_END__);
	kernel_pool_base = kernel_pool_head;
	
	for (int i = 0; i < 255; i++) {
		uintptr_t address = (uintptr_t)kernel_pool_base + i * PAGE_SIZE;

		struct free_node *current = (struct free_node *)(address);
		struct free_node *next = (struct free_node *)(address + PAGE_SIZE);

		current->next = next;
		next->next = 0;
	}
}