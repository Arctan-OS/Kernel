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
//	outb(0xE9, 'A');
	printf("\nWelcome to 64-bit wonderland! Please enjoy your stay.\n");

//	const char *string = "Hello World";

//	for (int i = 0; i < 20; i++) {
//		outb(0xE9, *(string + i));
//	}


	for (;;);

	*kernel_heap_pool = init_pool((void *)&__KERNEL_END__, PAGE_SIZE, 128);

	parse_mbi(mbi_ptr);

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

				*((uint32_t *)fb_current_context.virtual_buffer + (yf * fb_current_context.width) + xf) += (value + t * (-1 * sw)) * (sw + 1);
			}
		}

		if (t < 0x400 && sw == 1) {
			t++;
		} else if (t >= 0x1000 || sw == 0) {
			t--;
			sw = (t <= 2);
		}
	}

	return 0;
}
