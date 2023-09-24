#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <global.h>

void init_framebuffer(void *physical_buffer, void *virtual_buffer, uint64_t width, uint64_t height, uint64_t bpp);

#endif