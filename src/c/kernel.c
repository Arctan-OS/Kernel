/**
 * @file kernel.c
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
#include <global.h>
#include <arch/start.h>
#include <arch/smp.h>
#include <fs/vfs.h>
#include <interface/framebuffer.h>
#include <mp/sched/abstract.h>
#include <mm/allocator.h>
#include <lib/perms.h>
#include <mm/pmm.h>
#include <lib/util.h>
#include <abi-bits/stat.h>
#include <abi-bits/fcntl.h>

struct ARC_BootMeta *Arc_BootMeta = NULL;
struct ARC_TermMeta Arc_MainTerm = { 0 };
struct ARC_Resource *Arc_InitramfsRes = NULL;
struct ARC_File *Arc_FontFile = NULL;
static char Arc_MainTerm_mem[180 * 120] = { 0 };

int kernel_main(struct ARC_BootMeta *boot_meta) {
	// NOTE: Cannot use ARC_HHDM_VADDR before Arc_BootMeta is set
	Arc_BootMeta = boot_meta;

	Arc_MainTerm.rx_buf = NULL;
	Arc_MainTerm.tx_buf = NULL;
	Arc_MainTerm.term_width = 180;
	Arc_MainTerm.term_height = 25;
	Arc_MainTerm.term_mem = Arc_MainTerm_mem;
	Arc_MainTerm.font_width = 8;
	Arc_MainTerm.font_height = 14;
	Arc_MainTerm.cx = 0;
	Arc_MainTerm.cy = 0;
	init_static_mutex(&Arc_MainTerm.lock);

	init_arch();

	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");

	for (int i = 0; i < 60; i++) {
		for (int y = 0; y < Arc_MainTerm.fb_height; y++) {
			for (int x = 0; x < Arc_MainTerm.fb_width; x++) {
				ARC_FB_DRAW(Arc_MainTerm.framebuffer, x, (y * Arc_MainTerm.fb_width), Arc_MainTerm.fb_bpp, (x * y * i / 300) & 0x3FFF);
			}
		}
	}

	struct ARC_File *file = NULL;
	uint8_t *data = alloc(PAGE_SIZE * 4);
	vfs_open("/dev/nvme_namespace", 0, ARC_STD_PERM, &file);
	// TODO: Fix include
	// TODO: This does not actually work as the size of the
	//       namespace is not reported, so the seek is just
	//       ignored. I need to fix this in the vfs_traverse
	//       function

	vfs_read(data, 1, 512, file);
	for (int i = 0; i < 512; i++) {
		printf("%02X ", *(data + i));
	}
	printf("\n");

	data[0] = 0xAB;
	vfs_seek(file, 0, SEEK_SET);
	vfs_write(data, 1, 512, file);

	vfs_seek(file, 0, SEEK_SET);
	vfs_read(data, 1, 512, file);
	for (int i = 0; i < 512; i++) {
		printf("%02X ", *(data + i));
	}
	printf("\n");

	vfs_list("/", 8);

	term_draw(&Arc_MainTerm);

	for (;;) ARC_HANG;

	return 0;
}
