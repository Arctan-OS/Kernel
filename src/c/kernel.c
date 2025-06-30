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
#include <global.h>
#include <util.h>
#include <arch/pager.h>
#include <arch/acpi/acpi.h>
#include <arch/pci/pci.h>
#include <arch/start.h>
#include <arch/smp.h>
#include <drivers/dri_defs.h>
#include <mm/pmm.h>
#include <mm/allocator.h>
#include <lib/checksums.h>
#include <lib/perms.h>
#include <fs/vfs.h>

struct ARC_KernelMeta *Arc_KernelMeta = NULL;
struct ARC_BootMeta *Arc_BootMeta = NULL;

int kernel_main(struct ARC_KernelMeta *kernel_meta, struct ARC_BootMeta *boot_meta) {
	Arc_KernelMeta = kernel_meta;
	Arc_BootMeta = boot_meta;
	
	if (init_terminal() != 0) {
		ARC_HANG;
	}

	ARC_HANG;
	
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
	
	struct ARC_Resource *Arc_InitramfsRes = init_resource(ARC_DRIDEF_INITRAMFS_SUPER, (void *)ARC_PHYS_TO_HHDM(Arc_KernelMeta->initramfs.base));
	vfs_mount("/initramfs/", Arc_InitramfsRes);
	
	if (init_acpi() != 0) {
		return -1;
	}
	
	init_arch();
	
	if (init_pci() != 0) {
		return -2;
	}
	
	ARC_DISABLE_INTERRUPT;
	
	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");
	
	/*
	
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
	*/
	
	vfs_list("/", 16);

	smp_switch_to_userspace();

	ARC_HANG;

	return 0;
}
