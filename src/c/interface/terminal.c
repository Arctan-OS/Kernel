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

void term_putchar(struct ARC_TermMeta *term, char c) {
	if (term->cy >= term->term_height) {
		memcpy(term->term_mem, term->term_mem + term->term_width, (term->term_height - 1) * term->term_width);
		memset(term->term_mem + (term->term_height - 1) * term->term_width, 0, term->term_width);
		term->cy = term->term_height - 1;
	}

	switch (c) {
	        case '\n': {
		        term->cy++;
        		term->cx = 0;

        		break;
        	}

                case '\t': {
                        term->cx += 8;

                        break;
                }
        
	        default: {
        		if (term->term_mem != NULL) {
        			term->term_mem[term->cy * term->term_width + term->cx] = c;
        		}

        		term->cx++;

        		if (term->cx >= term->term_width) {
        			term->cy++;
        			term->cx = 0;
        		}

        		break;
        	}
	}

#ifdef ARC_COM_PORT
        uint8_t lsr = inb(ARC_COM_PORT + 5);
        while (MASKED_READ(lsr, 5, 1) == 0) {
                lsr = inb(ARC_COM_PORT + 5);
        }

        outb(ARC_COM_PORT, c);
#endif
}

void term_draw(struct ARC_TermMeta *term) {
	if (term->framebuffer == NULL) {
		return;
	}

	memset(term->framebuffer, 0, term->fb_width * term->fb_height * (term->fb_bpp / 8));

	int cwidth = 8;
	int cheight = 8;
	uint8_t *base = (uint8_t *)ARC_PHYS_TO_HHDM(Arc_BootMeta->term.char_rom);

	if (Arc_FontFile != NULL) {
		cwidth = term->font_width;
		cheight = term->font_height;

		base = alloc(cwidth * cheight / 8);
	}

	if (base == NULL) {
		return;
	}
        
	for (int cy = 0; cy < term->term_height; cy++) {
		for (int cx = 0; cx < term->term_width; cx++) {
			char c = term->term_mem[cy * term->term_width + cx];
			int fx = cx * cwidth;
			int fy = cy * cheight;

			if (c == 0) {
				continue;
			}

			uint8_t *char_base = NULL;
                        
			if (Arc_FontFile != NULL) {
				char_base = base;
				vfs_seek(Arc_FontFile, c * (cwidth * cheight / 8), SEEK_SET);
				vfs_read(char_base, 1, cwidth * cheight / 8, Arc_FontFile);
			} else {
				char_base = (uint8_t *)(base + (c * cheight));
			}

			for (int i = 0; i < cheight; i++) {
				int _j = 0;
				for (int j = cwidth - 1; j >= 0; j--, _j++) {
					if (((char_base[i] >> j) & 1) == 0) {
						continue;
					}

 	       				ARC_FB_DRAW(term->framebuffer, (fx + _j), ((i + fy) * term->fb_width), term->fb_bpp, 0xFFFFFF);
				}
			}

		}
	}

	if (Arc_FontFile != NULL) {
		free(base);
	}
}