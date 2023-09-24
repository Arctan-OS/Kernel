#include <framebuffer/framebuffer.h>

struct framebuffer_context fb_current_context = { 0 };

int init_branch_framebuffer(void *physical_buffer, void *virtual_buffer, uint64_t width, uint64_t height, uint64_t bpp) {
	// Allocate a section of memory as a "branch" framebuffer.
	// Place newly created branch into the next free position in the list.

	return -1; // Return position of framebuffer in list (-1 = error)
}

void draw_branch_framebuffers() {
	// Walk through branch list 0 -> n, copying each branch into the master buffer.
	// An addition of all branches onto the main buffer.
}

void init_master_framebuffer(void *physical_buffer, void *virtual_buffer, uint64_t width, uint64_t height, uint64_t bpp) {
	fb_current_context.physical_buffer = physical_buffer;
	fb_current_context.virtual_buffer = (virtual_buffer == NULL ? physical_buffer : virtual_buffer);
	// Ensure buffers are mapped
	fb_current_context.width = width;
	fb_current_context.height = height;
	fb_current_context.bpp = bpp;
	fb_current_context.size = width * height * (bpp / 8);
}