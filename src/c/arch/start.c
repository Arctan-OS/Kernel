/**
 * @file start.c
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
#include <arch/start.h>
#include <arch/acpi/acpi.h>
#include <arch/pager.h>
#include <global.h>
#include <mm/allocator.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <boot/parse.h>

#include <fs/vfs.h>
#include <lib/resource.h>
#include <drivers/dri_defs.h>

#ifdef ARC_TARGET_ARCH_X86_64
#include <arch/x86-64/gdt.h>
#include <arch/x86-64/idt.h>
#include <arch/x86-64/sse.h>
#include <arch/x86-64/syscall.h>
#include <arch/x86-64/apic/apic.h>
#endif

int init_arch() {
#ifdef ARC_TARGET_ARCH_X86_64
	ARC_DEBUG(INFO, "Successfully entered long mode\n");

	init_gdt();
	init_idt();
#endif
	init_pager();
	init_pmm((struct ARC_MMap *)Arc_BootMeta->arc_mmap, Arc_BootMeta->arc_mmap_len);

#ifdef ARC_TARGET_ARCH_X86_64
	init_sse();
#endif
	parse_boot_info();

	if (Arc_MainTerm.framebuffer != NULL) {
		Arc_MainTerm.term_width = (Arc_MainTerm.fb_width / Arc_MainTerm.font_width);
		Arc_MainTerm.term_height = (Arc_MainTerm.fb_height / Arc_MainTerm.font_height);
	}

        // Initialize memory
	init_allocator(128);

#ifdef ARC_TARGET_ARCH_X86_64
	create_tss(pmm_alloc() + PAGE_SIZE - 0x10, (void *)&__KERNEL_STACK__);
#endif

	init_vmm((void *)(ARC_HHDM_VADDR + Arc_BootMeta->highest_address), 0x100000000000);

	init_vfs();

	vfs_create("/initramfs/", ARC_STD_PERM, ARC_VFS_N_DIR, NULL);
        vfs_create("/dev/", ARC_STD_PERM, ARC_VFS_N_DIR, NULL);

	Arc_InitramfsRes = init_resource(0, ARC_SDRI_INITRAMFS, (void *)ARC_PHYS_TO_HHDM(Arc_BootMeta->initramfs));
	vfs_mount("/initramfs/", Arc_InitramfsRes);

        init_acpi(Arc_BootMeta->rsdp);

#ifdef ARC_TARGET_ARCH_X86_64
        init_apic();
	__asm__("sti");
	init_syscall();
#endif

	vfs_link("/initramfs/boot/ANTIQUE.F14", "/font.fnt", -1);
	vfs_rename("/font.fnt", "/fonts/font.fnt");
	vfs_open("/initramfs/boot/ANTIQUE.F14", 0, 0, 0, (void *)&Arc_FontFile);

	return 0;
}
