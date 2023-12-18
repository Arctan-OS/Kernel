/*
    Arctan - Operating System Kernel
    Copyright (C) 2023  awewsomegamer

    This file is apart of Arctan.

    Arctan is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <mbi_struct.h>
#include <multiboot2.h>
#include <framebuffer/framebuffer.h>
#include <mm/pmm.h>
#include <arctan.h>

struct mbi_tag_common {
	uint32_t type;
	uint32_t size;
}__attribute__((packed));

int parse_mbi(uint32_t ptr) {
	uint8_t *raw_ptr = (uint8_t *)((uintptr_t)ptr);
	uint32_t total_size = *(uint32_t *)(raw_ptr) - 8; // Exclude the fixed part
	struct mbi_tag_common *tag = (struct mbi_tag_common *)((uintptr_t)(ptr + 8));
	uint32_t bytes_parsed = 0;


	while (bytes_parsed < total_size) {
		tag = (struct mbi_tag_common *)((uintptr_t)(ptr + 8 + bytes_parsed));

		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			struct multiboot_tag_framebuffer *info = (struct multiboot_tag_framebuffer *)tag;

			init_master_framebuffer((void *)(info->common.framebuffer_addr + ARC_HHDM_VADDR), NULL, info->common.framebuffer_width, info->common.framebuffer_height, info->common.framebuffer_bpp);

			break;
		}

		case MULTIBOOT_TAG_TYPE_MMAP: {
			//initialize_pmm((struct multiboot_tag_mmap *)tag);

			break;
		}
		}

		bytes_parsed += ALIGN(tag->size, 8);
	}

	return 0;
}
