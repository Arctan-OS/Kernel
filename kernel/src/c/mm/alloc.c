#include <mm/alloc.h>
#include <global.h>

struct free_pointer {
	struct free_pointer *prev;
	struct free_pointer *next;
};

static struct free_pointer *kernel_heap_head;

void *alloc_kpage() {
	void *base = (void *)kernel_heap_head;

	if (kernel_heap_head->next == NULL) {
		kernel_heap_head += PAGE_SIZE;

		return base;
	}

	kernel_heap_head = kernel_heap_head->next;

	return base;
}

void *free_kpage(void *page) {
	struct free_pointer *new = (struct free_pointer *)page;

	if (kernel_heap_head->prev == NULL && new < kernel_heap_head) {
		new->next = kernel_heap_head;
		kernel_heap_head->prev = new;
		kernel_heap_head = new;

		return page;
	} else if (kernel_heap_head->next == NULL && new > kernel_heap_head) {
		kernel_heap_head->next = new;

		return page;
	}

	struct free_pointer *current = kernel_heap_head;

	uint8_t last_operation = 0; // 0: Nothing, 1: Prev, 2: Next

	while (current != NULL) {
		switch (last_operation) {
		case 1: {
			if (current->prev != NULL && current->prev < new) {
				new->prev = current->prev;
				current->prev->next = new;
				new->next = current;
			}
			
			break;
		}

		case 2: {
			break;
		}
		}



		if (current < new) {
			last_operation = 2;

			current = current->next;
		} else {
			last_operation = 1;

			current = current->prev;
		}
	}

	return NULL;
}

void init_allocator() {
	kernel_heap_head = (struct free_pointer *)&__KERNEL_END__;
	kernel_heap_head->next = NULL;
}
