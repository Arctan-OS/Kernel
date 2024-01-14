#ifndef ARC_INTERFACE_TERMINAL_H
#define ARC_INTERFACE_TERMINAL_H

#include <stdint.h>

struct ARC_TermMeta {
	void *framebuffer; // Virtual address of framebuffer
	int fb_width; // Framebuffer width in pixels
	int fb_height; // Framebuffer height in pixels
	int fb_bpp; // Framebuffer bpp
	int fb_pitch; // Framebuffer pitch
	int cx; // Current character x position
	int cy; // Current character y position
	uint8_t *font_bmp; // The BMP font
	int font_width; // Width of the font in pixels
	int font_height; // Height of the font in pixels
	char *term_mem; // Character memory of terminal
	int term_width; // Width of the terminal in characters
	int term_height; // Height of the terminal in characters
};

void Arc_TermPutChar(struct ARC_TermMeta *term, char c);
void Arc_TermDraw(struct ARC_TermMeta *term);

#endif
