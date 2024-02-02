/**
 * @file mbparse.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is part of Arctan.
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
*/
#include "fs/initramfs.h"
#include <global.h>
#include <multiboot/mbparse.h>
#include <multiboot/multiboot2.h>
#include <interface/printf.h>
#include <mm/pmm.h>

int strcmp(char *a, char *b) {
	int sum = 0;
	while (*a != 0) {
		sum += *a - *b;

		a++;
		b++;
	}

	return sum;
}

int parse_mbi() {
	struct multiboot_tag *tag = (struct multiboot_tag *)(Arc_BootMeta->mb2i + ARC_HHDM_VADDR);
	struct multiboot_tag *end = (struct multiboot_tag *)(tag + tag->type);

	tag = (struct multiboot_tag *)((uintptr_t)tag + 8);

	while (tag < end && tag->type != 0) {
		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_MMAP: {
			struct multiboot_tag_mmap *info = (struct multiboot_tag_mmap *)tag;

			Arc_InitPMM(info, Arc_BootMeta->pmm_state);

			break;
		}

		case MULTIBOOT_TAG_TYPE_MODULE: {
			struct multiboot_tag_module *info = (struct multiboot_tag_module *)tag;

			ARC_DEBUG(INFO, "----------------\n")
			ARC_DEBUG(INFO, "Found module: %s\n", info->cmdline);
			ARC_DEBUG(INFO, "\t0x%"PRIX32" -> 0x%"PRIX32" (%d B)\n", info->mod_start, info->mod_end, (info->mod_end - info->mod_start))

			if (strcmp(info->cmdline, "arctan-module.kernel.efi") == 0) {
				ARC_DEBUG(INFO, "\tFound kernel\n")
			} else if (strcmp(info->cmdline, "arctan-module.initramfs.cpio") == 0) {
				ARC_DEBUG(INFO, "\tFound initramfs\n")
				main_terminal.font_bmp = (uint8_t *)(Arc_FindFileInitramfs((void *)(info->mod_start + ARC_HHDM_VADDR), "initramfs/FONT.fnt"));
				main_terminal.font_width = 8;
				main_terminal.font_height = 8;
			}

			ARC_DEBUG(INFO, "----------------\n")

			break;
		}

		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			struct multiboot_tag_framebuffer *info = (struct multiboot_tag_framebuffer *)tag;
			struct multiboot_tag_framebuffer_common common = (struct multiboot_tag_framebuffer_common)(info->common);

			ARC_DEBUG(INFO, "Framebuffer 0x%"PRIX64"(%d) %dx%dx%d\n", common.framebuffer_addr, common.framebuffer_type, common.framebuffer_width, common.framebuffer_height, common.framebuffer_bpp)

			for (int i = 0; i < common.framebuffer_height; i++) {
				for (int j = 0; j < common.framebuffer_width; j++) {
					*((uint32_t *)(common.framebuffer_addr + ARC_HHDM_VADDR) + (i * common.framebuffer_width) + j) = 0x0043210FF;
				}
			}

			main_terminal.framebuffer = (void *)(common.framebuffer_addr + ARC_HHDM_VADDR);
			main_terminal.fb_width = common.framebuffer_width;
			main_terminal.fb_height = common.framebuffer_height;
			main_terminal.fb_bpp = common.framebuffer_bpp;
			main_terminal.fb_pitch = common.framebuffer_pitch;

			break;
		}
		}

		tag = (struct multiboot_tag *)((uintptr_t)tag + ALIGN(tag->size, 8));
	}


	return 0;
}
