/**
 * @file mb2.c
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
#include <arctan.h>
#include <global.h>
#include <boot/mb2.h>
#include <boot/multiboot2.h>
#include <interface/printf.h>
#include <mm/pmm.h>
#include <util.h>

struct ARC_MB2BootInfo {
	uint64_t mbi_phys;
	uint64_t fb;
}__attribute__((packed));

int Arc_ParseMB2I() {
	ARC_DEBUG(INFO, "Parsing Multiboot2\n");

	struct ARC_MB2BootInfo *info = (struct ARC_MB2BootInfo *)ARC_PHYS_TO_HHDM(Arc_BootMeta->boot_info);

	struct multiboot_tag_framebuffer *fb = (struct multiboot_tag_framebuffer *)ARC_PHYS_TO_HHDM(info->fb);

	struct multiboot_tag_framebuffer_common common = fb->common;
	ARC_DEBUG(INFO, "Framebuffer 0x%llX (%d) %dx%dx%d\n", common.framebuffer_addr, common.framebuffer_type, common.framebuffer_width, common.framebuffer_height, common.framebuffer_bpp);
	Arc_MainTerm.framebuffer = (void *)ARC_PHYS_TO_HHDM(common.framebuffer_addr);
	Arc_MainTerm.fb_width = common.framebuffer_width;
	Arc_MainTerm.fb_height = common.framebuffer_height;
	Arc_MainTerm.fb_bpp = common.framebuffer_bpp;
	Arc_MainTerm.fb_pitch = common.framebuffer_pitch;

	return 0;
}
