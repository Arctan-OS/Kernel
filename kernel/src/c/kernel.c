#include <stdint.h>
#include <stddef.h>

int kernel_main(int framebuffer_width, int framebuffer_height) {
	// Wavium
	for (int t = 0; ; t += 0x100)
		for (int i = 0; i < framebuffer_height; i++)
			for (int j = 0; j < framebuffer_width; j++)
				*((uint32_t *)0xFD000000 + ((i * framebuffer_width) + j)) = (i * j + t) >> 8;

	return 0;
}