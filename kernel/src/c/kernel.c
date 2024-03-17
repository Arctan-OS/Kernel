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
#include <lib/resource.h>
#include <mm/allocator.h>
#include <mm/freelist.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <arctan.h>
#include <global.h>
#include <arch/x86-64/io/port.h>
#include <stdint.h>
#include <interface/printf.h>
#include <arch/x86-64/ctrl_regs.h>
#include <multiboot/mbparse.h>

#include <arch/x86-64/idt.h>
#include <arch/x86-64/gdt.h>

#include <interface/terminal.h>
#include <mm/pmm.h>
#include <fs/vfs.h>

#include <arch/x86-64/syscall.h>

#include <errno.h>

struct ARC_BootMeta *Arc_BootMeta = NULL;
struct ARC_TermMeta Arc_MainTerm = { 0 };
struct ARC_Resource Arc_InitramfsRes = { .dri_group = 0, .dri_index = 0 };
struct ARC_VFSNode *Arc_FontFile = NULL;
static char Arc_MainTerm_mem[180 * 120] = { 0 };

int kernel_main(struct ARC_BootMeta *boot_meta) {
	Arc_BootMeta = boot_meta;

	Arc_MainTerm.rx_buf = NULL;
	Arc_MainTerm.tx_buf = NULL;
	Arc_MainTerm.term_width = 180;
	Arc_MainTerm.term_height = 120;
	Arc_MainTerm.term_mem = Arc_MainTerm_mem;
	Arc_MainTerm.font_width = 8;
	Arc_MainTerm.font_height = 8;
	Arc_MainTerm.cx = 0;
	Arc_MainTerm.cy = 0;

	ARC_DEBUG(INFO, "Sucessfully entered long mode\n")

	install_gdt();
	install_idt();

	parse_mbi();
	Arc_InitVMM();
	Arc_InitSlabAllocator(10);

	Arc_InitializeVFS();
	Arc_MountVFS(NULL, "initramfs", &Arc_InitramfsRes, ARC_VFS_FS_INITRAMFS);
	Arc_FontFile = Arc_OpenFileVFS("/initramfs/boot/CGA.F08", 0, 0);

	Arc_InitializeSyscall();

	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");

	// Quickly map framebuffer in
	uint64_t fb_size = Arc_MainTerm.fb_width * Arc_MainTerm.fb_height * (Arc_MainTerm.fb_bpp / 8);
	for (uint64_t i = 0; i < fb_size; i += 0x1000) {
		Arc_MapPageVMM(ARC_HHDM_TO_PHYS(Arc_MainTerm.framebuffer + i), (uintptr_t)(Arc_MainTerm.framebuffer + i), ARC_VMM_OVERW_FLAG | 3);
	}

	for (int i = 0; i < 600; i++) {
		for (int y = 0; y < Arc_MainTerm.fb_height; y++) {
			for (int x = 0; x < Arc_MainTerm.fb_width; x++) {
				*((uint32_t *)Arc_MainTerm.framebuffer + (y * Arc_MainTerm.fb_width) + x) = (x * y * i / 300) & 0x3FFF;
			}
		}
	}

	for (;;) {
		Arc_TermDraw(&Arc_MainTerm);
	}

	return 0;
}
