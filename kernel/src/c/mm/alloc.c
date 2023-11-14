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
	void *address = (void *)kernel_pool_head;

	if (pages > 1) {
		size_t count = 1;

		while (kernel_pool_head != NULL && count < pages) {
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
	}
	
	if (kernel_pool_head > kernel_pool_base) {
		struct free_node *prev = (struct free_node *)((uintptr_t)address - PAGE_SIZE);
		prev->next = kernel_pool_head->next;
	}

	kernel_pool_head = (struct free_node *)((uintptr_t)kernel_pool_head + PAGE_SIZE);

	return address;
}

void *free_kpages(void *address, size_t pages) {
	if (pages > 1) {
		return address;
	}
	
	struct free_node *current = (struct free_node *)address;

	if (current > kernel_pool_base) {
		struct free_node *prev = (struct free_node *)((uintptr_t)kernel_pool_head - PAGE_SIZE);
		
		current->next = prev->next;
		prev->next = current;
	}

	return address;
}

int switch_to_pool(struct pool_descriptor *pool) {


	return 0;
}

void init_allocator() {
	kernel_pool_head = (void *)(&__KERNEL_END__);
	kernel_pool_base = kernel_pool_head;
	
	for (int i = 0; i < 255; i++) {
		uintptr_t address = (uintptr_t)kernel_pool_base + i * PAGE_SIZE;
		printf("%X\n", address);

		struct free_node *current = (struct free_node *)(address);
		struct free_node *next = (struct free_node *)(address + PAGE_SIZE);

		current->next = next;
		next->next = 0;
	}
}