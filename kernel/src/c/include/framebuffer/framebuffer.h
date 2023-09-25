#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <global.h>

#define MAX_BRANCH_FRAMEBUFFERS 16

struct framebuffer_context {
	void *physical_buffer;
	void *virtual_buffer; // NULL means (physical_buffer = virtual_buffer)
	int width;
	int height;
	int bpp;
	size_t size; // in bytes
};
extern struct framebuffer_context fb_current_context;

int init_branch_framebuffer(void *physical_buffer, void *virtual_buffer);
void draw_branch_framebuffers();
void init_master_framebuffer(void *physical_buffer, void *virtual_buffer, uint64_t width, uint64_t height, uint64_t bpp);

#endif