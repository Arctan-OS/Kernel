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

struct ARC_BootMeta *Arc_BootMeta = NULL;
struct ARC_TermMeta Arc_MainTerm = { 0 };
struct ARC_Resource *Arc_InitramfsRes = NULL;
struct ARC_File *Arc_FontFile = NULL;
static char Arc_MainTerm_mem[180 * 120] = { 0 };

int proc_test(int processor) {
	struct ARC_File *file = NULL;
	int i = vfs_open("/initramfs/boot/credit.txt", 0, ARC_STD_PERM, (void *)&file);
	char data[26] = { 0 };
	vfs_read(&data, 1, 24, file);
	printf("Processor %d has arrived %"PRIx64" %d %s\n", processor, get_current_tid(), i, data);
	vfs_close(file);

	size_t size = 26;
	vfs_create("/write_test.txt", ARC_STD_PERM, ARC_VFS_N_BUFF, &size);
	vfs_open("/write_test.txt", 0, ARC_STD_PERM, (void *)&file);
	vfs_seek(file, 3 * (processor - 1), SEEK_SET);
	sprintf_(data, "C%d ", processor);
	vfs_write(data, 1, 3, file);
	vfs_close(file);

	vfs_open("/initramfs/boot/reference.txt", 0, ARC_STD_PERM, &file);
	vfs_read(data, 1, 24, file);
	printf("Link resolves: %s\n", data);
	vfs_close(file);

	printf("Processor did not deadlock %d\n", processor);

	ARC_HANG;
}

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

	vfs_list("/", 8);

	printf("-----------------------------------------\n");

	struct ARC_ProcessorDescriptor *desc = Arc_BootProcessor->generic.next;
	while (desc != NULL) {
		desc->generic.flags |= 1 << 1;
		while ((desc->generic.flags >> 1) & 1) __asm__("pause");

		smp_jmp(desc, proc_test, 1, desc->generic.acpi_uid);

		desc = desc->generic.next;
	}

	for (int i = 0; i < 60; i++) {
		for (int y = 0; y < Arc_MainTerm.fb_height; y++) {
			for (int x = 0; x < Arc_MainTerm.fb_width; x++) {
				ARC_FB_DRAW(Arc_MainTerm.framebuffer, x, (y * Arc_MainTerm.fb_width), Arc_MainTerm.fb_bpp, (x * y * i / 300) & 0x3FFF);
			}
		}
	}

	struct ARC_File *file = NULL;
	vfs_open("/write_test.txt", 0, ARC_STD_PERM, (void *)&file);
	char buffer[26] = { 0 };
	vfs_read(buffer, 1, 24, file);
	printf("Processors wrote: %s\n", buffer);
	vfs_close(file);

	vfs_link("/initramfs/boot/credit.txt", "/initramfs/boot/a/b/c/credit.txt", -1);
	vfs_link("/initramfs/boot/credit.txt", "/initramfs/credit.txt", -1);
	vfs_link("/initramfs/boot/credit.txt", "/credit.txt", -1);

	term_draw(&Arc_MainTerm);

	vfs_close(Arc_FontFile);

	vfs_list("/", 8);

	for (;;) ARC_HANG;

	return 0;
}
