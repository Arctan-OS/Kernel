#include <mm/alloc.h>
#include <global.h>

static void *kernel_heap_base = NULL;
static size_t page_ptr = 0;
static size_t page_limit = 0;

void *alloc_kpage(size_t pages) {
	if (page_ptr + pages >= page_limit) {
		return NULL;
	}

	void *address = (void *)((uintptr_t)kernel_heap_base + (page_ptr * PAGE_SIZE));
	
	page_ptr += pages;

	return address;
}

void init_allocator() {
	kernel_heap_base = (void *)&__KERNEL_END__;
	page_limit = (UINT64_MAX - (uintptr_t)kernel_heap_base) / 0x1000;
}