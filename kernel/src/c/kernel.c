#include <stdint.h>
#include <stddef.h>
#include <mbi_struct.h>
#include <framebuffer/framebuffer.h>

int kernel_main(uint32_t mbi_ptr) {
	parse_mbi(mbi_ptr);
	
	// Wavium
	for (int t = 0; ; t += 0x100)
		for (int i = 0; i < fb_current_context.height; i++)
			for (int j = 0; j < fb_current_context.width; j++)
				*((uint32_t *)0xFD000000 + ((i * fb_current_context.width) + j)) = (i * j + t) >> 8;

	return 0;
}