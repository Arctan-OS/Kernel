#include <framebuffer/framebuffer.h>

struct framebuffer_context {
	void *physical_buffer;
	void *virtual_buffer; // NULL means (physical_buffer = virtual_buffer)
	uint64_t width;
	uint64_t height;
	uint64_t bpp;
	size_t size; // in bytes
};
struct framebuffer_context current_context = { 0 };

void init_framebuffer(void *physical_buffer, void *virtual_buffer, uint64_t width, uint64_t height, uint64_t bpp) {
	current_context.physical_buffer = physical_buffer;
	current_context.virtual_buffer = virtual_buffer;
	current_context.width = width;
	current_context.height = height;
	current_context.bpp = bpp;
	current_context.size = width * height * (bpp / 8);

	*((uint32_t *)physical_buffer) = 0xFFFFFFFF;
}