/**
 * @file terminal.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Kernel - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan-OS/Kernel.
 *
 * Arctan is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
 * A basic terminal implementation capable of displaying to a framebuffer and
 * sending written data through a COM port specified at compile time.
*/
#include <fs/vfs.h>
#include <mm/allocator.h>
#include <global.h>
#include <interface/terminal.h>
#include <arctan.h>
#include <lib/util.h>
#include <arch/io/port.h>
#include <util.h>

static void *term_fb = NULL;
static int term_fb_width = 0;
static int term_fb_height = 0;
static int term_fb_bpp = 0;
static int term_x = 0;
static int term_y = 0;
static int term_width = 0;
static int term_height = 0;
uint8_t term_mem[0x4000] = { 0 };

void term_putchar(char c) {
#ifdef ARC_COM_PORT
        uint8_t lsr = inb(ARC_COM_PORT + 5);
        while (MASKED_READ(lsr, 5, 1) == 0) {
                lsr = inb(ARC_COM_PORT + 5);
        }

        outb(ARC_COM_PORT, c);
#endif

	if (term_y >= term_height) {
		memcpy(term_mem, term_mem + term_width, (term_height - 1) * term_width);
		memset(term_mem + (term_height - 1) * term_width, 0, term_width);
		term_y = term_height - 1;
	}

	switch (c) {
	        case '\n': {
		        term_y++;
        		term_x = 0;

        		break;
        	}

                case '\t': {
                        term_x += 8;

                        break;
                }
        
	        default: {
        		term_mem[term_y * term_width + term_x] = c;
        		
        		term_x++;

        		if (term_x >= term_width) {
        			term_y++;
        			term_x = 0;
        		}

        		break;
        	}
	}
}

void term_draw() {
	if (term_fb == NULL) {
		return;
	}

	memset(term_fb, 0, term_fb_width * term_fb_height * (term_fb_bpp / 8));

	int cwidth = 8;
	int cheight = 8;
	uint8_t *base = (uint8_t *)ARC_PHYS_TO_HHDM(Arc_BootMeta->term.char_rom);

	if (base == NULL) {
		return;
	}

	for (int cy = 0; cy < term_height; cy++) {
		for (int cx = 0; cx < term_width; cx++) {
                        if (cy * term_width + cx > 0x4000)  {
                                break;
                        }

			char c = term_mem[cy * term_width + cx];
			int fx = cx * cwidth;
			int fy = cy * cheight;

			if (c == 0) {
				continue;
			}
			uint8_t *char_base = (uint8_t *)(base + (c * cheight));

			for (int i = 0; i < cheight; i++) {
				int _j = 0;
				for (int j = cwidth - 1; j >= 0; j--, _j++) {
					if (((char_base[i] >> j) & 1) == 0) {
						continue;
					}

	       			ARC_FB_DRAW(term_fb, (fx + _j), ((i + fy) * term_fb_width), term_fb_bpp, 0xFFFFFF);
				}
			}
		}
	}
}

int init_terminal() {
	term_fb = (void *)ARC_PHYS_TO_HHDM(Arc_BootMeta->term.base);
	term_fb_width = Arc_BootMeta->term.width;
	term_fb_height = Arc_BootMeta->term.height;
	term_fb_bpp = Arc_BootMeta->term.bpp;
	term_width = term_fb_width / 8;
	term_height = term_fb_height / 8;

	ARC_DEBUG(INFO, "Initialized terminal (%dx%d)\n", term_width, term_height);

	return 0;
}
