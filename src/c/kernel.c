/**
 * @file kernel.c
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
*/
#include "arctan.h"
#include "interface/terminal.h"
#include <userspace/thread.h>
#include <lib/atomics.h>
#include <global.h>
#include <arch/start.h>
#include <arch/smp.h>
#include <fs/vfs.h>
#include <mp/scheduler.h>
#include <mm/allocator.h>
#include <lib/perms.h>
#include <lib/util.h>
#include <lib/checksums.h>
#include <drivers/dri_defs.h>
#include <userspace/process.h>
#include <abi-bits/stat.h>
#include <abi-bits/fcntl.h>

#include <arch/pager.h>
#include <global.h>
#include <mm/allocator.h>
#include <mm/pmm.h>
#include <boot/parse.h>
#include <stdint.h>
#include <arch/pci/pci.h>
#include <arch/acpi/acpi.h>

struct ARC_TermMeta Arc_InitTerm = { 0 };
static char Arc_InitTerm_mem[180 * 120] = { 0 };

struct ARC_KernelMeta *Arc_KernelMeta = NULL;
struct ARC_BootMeta *Arc_BootMeta = NULL;
struct ARC_TermMeta *Arc_CurrentTerm = NULL;
struct ARC_Resource *Arc_InitramfsRes = NULL;
struct ARC_File *Arc_FontFile = NULL;
struct ARC_Process *Arc_ProcessorHold = NULL;

int kernel_main(struct ARC_KernelMeta *kernel_meta, struct ARC_BootMeta *boot_meta) {
	Arc_KernelMeta = kernel_meta;
	Arc_BootMeta = boot_meta;

	Arc_InitTerm.term_width = 180;
	Arc_InitTerm.term_height = 25;
	Arc_InitTerm.term_mem = Arc_InitTerm_mem;
	Arc_InitTerm.font_width = 8;
	Arc_InitTerm.font_height = 14;
	Arc_InitTerm.cx = 0;
	Arc_InitTerm.cy = 0;

	init_static_mutex(&Arc_InitTerm.lock);
	
	Arc_CurrentTerm = &Arc_InitTerm;

	if (parse_boot_info() != 0) {
		ARC_DEBUG(ERR, "Failed to parse boot information\n");
		ARC_HANG;
	}
	
	if (Arc_InitTerm.framebuffer != NULL) {
		Arc_InitTerm.term_width = (Arc_InitTerm.fb_width / Arc_InitTerm.font_width);
		Arc_InitTerm.term_height = (Arc_InitTerm.fb_height / Arc_InitTerm.font_height);
	}
	
	if (init_pmm((struct ARC_MMap *)ARC_PHYS_TO_HHDM(Arc_KernelMeta->arc_mmap.base), Arc_KernelMeta->arc_mmap.len) != 0) {
		ARC_DEBUG(ERR, "Failed to initialize physical memory manager\n");
		ARC_HANG;
	}
	
	
	if (init_pager() != 0) {
		ARC_DEBUG(ERR, "Failed to initialize pager\n");
		ARC_HANG;
	}
	
	if (init_allocator(256) != 0) {
		ARC_DEBUG(ERR, "Failed to initialize kernel allocator\n");
		ARC_HANG;
	}
	
	init_checksums();
	
	init_vfs();
	
	struct ARC_VFSNodeInfo info = {
		.type = ARC_VFS_N_DIR,
		.mode = ARC_STD_PERM,
    	};
	
	vfs_create("/initramfs/", &info);
        vfs_create("/dev/", &info);
	
	Arc_InitramfsRes = init_resource(ARC_DRIDEF_INITRAMFS_SUPER, (void *)ARC_PHYS_TO_HHDM(Arc_KernelMeta->initramfs.base));
	vfs_mount("/initramfs/", Arc_InitramfsRes);
	
	if (init_acpi() != 0) {
		return -1;
	}
	
	init_arch();
	
	if (init_pci() != 0) {
		return -2;
	}
	
	ARC_DISABLE_INTERRUPT;
	
	vfs_open("/initramfs/font.fnt", 0, ARC_STD_PERM, &Arc_FontFile);
	
	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");
	
	Arc_ProcessorHold = process_create(0, NULL);
	
	if (Arc_ProcessorHold == NULL) {
		ARC_DEBUG(ERR, "Failed to create hold process\n");
		ARC_HANG;
	}
	
	Arc_ProcessorHold->allocator = init_vmm((void *)0x1000, 0x10000);
	
	if (Arc_ProcessorHold->allocator == NULL) {
		ARC_DEBUG(ERR, "Failed to create hold process allocator\n");
		ARC_HANG;
	}
	
	struct ARC_Thread *hold = thread_create(Arc_ProcessorHold->allocator, Arc_ProcessorHold->page_tables, (void *)smp_hold, 0x1000);
	
	if (hold == NULL) {
		ARC_DEBUG(ERR, "Failed to create hold thread\n");
		process_delete(Arc_ProcessorHold);
		ARC_HANG;
	}
	
	process_associate_thread(Arc_ProcessorHold, hold);
	
	init_scheduler();
	sched_queue(Arc_ProcessorHold, ARC_SCHED_PRI_LO);
	
	struct ARC_Process *userspace = process_create_from_file(1, "/initramfs/userspace.elf");
	if (userspace == NULL) {
		ARC_DEBUG(ERR, "Failed to load userspace\n");
	}
	sched_queue(userspace, ARC_SCHED_PRI_HI);
	
	vfs_list("/", 16);

	smp_switch_to_userspace();

	ARC_HANG;

	return 0;
}
