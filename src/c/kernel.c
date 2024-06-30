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
#include <mm/slab.h>
#include <mm/freelist.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <arctan.h>
#include <global.h>
#include <arch/x86-64/io/port.h>
#include <stdint.h>
#include <interface/printf.h>
#include <arch/x86-64/ctrl_regs.h>
#include <arch/x86-64/acpi/acpi.h>
#include <arch/x86-64/apic/apic.h>
#include <boot/parse.h>

#include <arch/x86-64/idt.h>
#include <arch/x86-64/gdt.h>

#include <interface/terminal.h>
#include <mm/pmm.h>
#include <fs/vfs.h>

#include <arch/x86-64/syscall.h>

struct ARC_BootMeta *Arc_BootMeta = NULL;
struct ARC_TermMeta Arc_MainTerm = { 0 };
struct ARC_Resource *Arc_InitramfsRes = NULL;
struct ARC_File *Arc_FontFile = NULL;
static char Arc_MainTerm_mem[180 * 120] = { 0 };

int empty() {
	return 0;
}

int kernel_main(struct ARC_BootMeta *boot_meta) {
        *((uint8_t *)0xB8002) = 'A';

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

	ARC_DEBUG(INFO, "Sucessfully entered long mode\n");

        // Initialize really basic things
	Arc_InstallGDT();
	Arc_InstallIDT();
	Arc_ParseBootInfo();

	if (Arc_MainTerm.framebuffer != NULL) {
		Arc_MainTerm.term_height = (Arc_MainTerm.fb_height / Arc_MainTerm.font_height);
	}

        // Initialize memory
	Arc_InitPMM((struct ARC_MMap *)boot_meta->arc_mmap, boot_meta->arc_mmap_len);
	Arc_InitVMM();
        // Arc_InitBuddy(big_block_size, max_subdivisions);
	Arc_InitSlabAllocator(100);

        // Initialize more complicated things
	Arc_InitializeVFS();
	Arc_CreateVFS("/initramfs/", 0, ARC_VFS_N_DIR, NULL);
        Arc_CreateVFS("/dev/", 0, ARC_VFS_N_DIR, NULL);

        Arc_InitializeACPI(boot_meta->rsdp);
        // TODO: Implement properly
        Arc_InitAPIC();
	Arc_InitializeSyscall();

	Arc_InitramfsRes = Arc_InitializeResource("initramfs", 0, 0, (void *)ARC_PHYS_TO_HHDM(boot_meta->initramfs));
	Arc_MountVFS("/initramfs/", Arc_InitramfsRes, ARC_VFS_FS_INITRAMFS);
	Arc_LinkVFS("/initramfs/boot/ANTIQUE.F14", "/font.fnt", 0);
	Arc_RenameVFS("/font.fnt", "/fonts/font.fnt");
	Arc_OpenVFS("/fonts/font.fnt", 0, 0, 0, (void *)&Arc_FontFile);

	size_t size = 64;
	struct ARC_File *buffer0 = NULL;
	Arc_CreateVFS("/buffer0", 0, ARC_VFS_N_BUFF, &size);
	Arc_OpenVFS("/buffer0", 0, 0, 0, (void *)&buffer0);
	Arc_WriteVFS("Hello World", 1, 12, buffer0);
	Arc_SeekVFS(buffer0, 11, ARC_VFS_SEEK_SET);
	Arc_WriteVFS("Silly", 1, 6, buffer0);
	char data[64];
	Arc_SeekVFS(buffer0, 0, ARC_VFS_SEEK_SET);
	Arc_ReadVFS(data, 1, 64, buffer0);
	printf("%s\n", data);

	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");

	Arc_ListVFS("/", 8);

        // Quickly map framebuffer in
	uint64_t fb_size = Arc_MainTerm.fb_width * Arc_MainTerm.fb_height * (Arc_MainTerm.fb_bpp / 8);
	for (uint64_t i = 0; i < fb_size; i += 0x1000) {
		Arc_MapPageVMM(ARC_HHDM_TO_PHYS(Arc_MainTerm.framebuffer + i), (uintptr_t)(Arc_MainTerm.framebuffer + i), ARC_VMM_OVERW_FLAG | 3 | ARC_VMM_PAT_WC(0));
	}

	for (int i = 0; i < 60; i++) {
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
