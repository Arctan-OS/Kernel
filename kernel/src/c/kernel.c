#include <stdint.h>
#include <stddef.h>
#include <mbi_struct.h>
#include <framebuffer/framebuffer.h>
#include <io/port.h>
#include <temp/interface.h>
#include <mm/vmm.h>
#include <mm/alloc.h>

struct pool_descriptor *kernel_heap_pool;

int kernel_main(uint32_t mbi_ptr) {
	printf("\nWelcome to 64-bit wonderland! Please enjoy your stay.\n");

	*kernel_heap_pool = init_pool((void *)&__KERNEL_END__, PAGE_SIZE, 512);

	parse_mbi(mbi_ptr);

	void *a = alloc_pages(kernel_heap_pool,14);
	printf("%X\n", (uintptr_t)a);
	free_pages(kernel_heap_pool, a, 2);
	void *b = alloc_pages(kernel_heap_pool,1);
	printf("%X\n", (uintptr_t)b);
	b = alloc_pages(kernel_heap_pool,1);
	printf("%X\n", (uintptr_t)b);
	b = alloc_pages(kernel_heap_pool,1);
	printf("%X\n", (uintptr_t)b);
	b = alloc_pages(kernel_heap_pool,1);
	printf("%X\n", (uintptr_t)b);
	b = alloc_pages(kernel_heap_pool,1);
	printf("%X\n", (uintptr_t)b);

	int t = 0;

	uint8_t sw = 1;

	while (1) {
		for (int i = 0; i < fb_current_context.height; i++) {
			for (int j = 0; j < fb_current_context.width; j++) {
				int value = (i - j + t);

				int x = (j + value) % fb_current_context.width;
				int y = (i + value) % fb_current_context.height;

				int xf = ((x * x) / x);
				int yf = ((y * y) / y);
				
				if (xf < 0 || yf < 0) {
					continue;
				}

				*((uint32_t *)fb_current_context.virtual_buffer + (yf * fb_current_context.width) + xf) += value;
			}
		}

		// if (t < 0x100 && sw == 1) {
			t++;
		// } else if (t >= 0x100 || sw == 0) {
			// t--;
		// 	sw = (t <= 2);
		// }
	}

	return 0;
}