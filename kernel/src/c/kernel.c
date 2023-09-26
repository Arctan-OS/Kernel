#include <stdint.h>
#include <stddef.h>
#include <mbi_struct.h>
#include <framebuffer/framebuffer.h>

int kernel_main(uint32_t mbi_ptr) {
	parse_mbi(mbi_ptr);
	
	// Wavium
	int t = 0;
	uint8_t sw = 1;
	while (1) {
		for (int i = 0; i < fb_current_context.height; i++)
			for (int j = 0; j < fb_current_context.width; j++)
				*((uint32_t *)0xFD000000 + ((i * fb_current_context.width) + j)) = ((i + fb_current_context.height) / (j + fb_current_context.width / 16) * t);

		if (t < 0x100 && sw == 1) {
			t++;
		} else if (t >= 0x100 || sw == 0) {
			t--;
			sw = (t <= 2);
		}
	}

	return 0;
}