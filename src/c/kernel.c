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
#include "arch/acpi/acpi.h"
#include "arch/pager.h"
#include "arch/pci.h"
#include "arch/start.h"
#include "arch/smp.h"
#include "arch/x86-64/util.h"
#include "drivers/dri_defs.h"
#include "fs/vfs.h"
#include "global.h"
#include "lib/checksums.h"
#include "drivers/resource.h"
#include "mm/allocator.h"
#include "mm/pmm.h"
#include "mp/scheduler.h"
#include "userspace/process.h"

struct ARC_KernelMeta *Arc_KernelMeta = NULL;
struct ARC_BootMeta *Arc_BootMeta = NULL;

int kernel_main(struct ARC_KernelMeta *kernel_meta, struct ARC_BootMeta *boot_meta) {
	Arc_KernelMeta = kernel_meta;
	Arc_BootMeta = boot_meta;

	init_arch_early();

	if (init_pager() != 0) {
		ARC_DEBUG(ERR, "Failed to initialize pager\n");
		ARC_HANG;
	}

	if (init_terminal() != 0) {
		ARC_HANG;
	}

	if (init_pmm((struct ARC_MMap *)ARC_PHYS_TO_HHDM(Arc_KernelMeta->arc_mmap.base), Arc_KernelMeta->arc_mmap.len) != 0) {
		ARC_DEBUG(ERR, "Failed to initialize physical memory manager\n");
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
	vfs_create("/process/", &info);

	struct ARC_Resource *Arc_InitramfsRes = init_resource(ARC_DRIDEF_INITRAMFS_SUPER, (void *)ARC_PHYS_TO_HHDM(Arc_KernelMeta->initramfs.base));
	vfs_mount("/initramfs/", Arc_InitramfsRes);

	if (init_acpi() != 0) {
		return -1;
	}

	init_smp();

	init_arch();

	if (init_pci() != 0) {
		return -2;
	}

	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");

	vfs_list("/", 16);

	if (init_scheduler() != 0) {
		ARC_DEBUG(ERR, "No scheduler\n");
		ARC_HANG;
	}

	ARC_Process *userspace = process_create_from_file(true, "/initramfs/userspace.elf");
	if (userspace == NULL) {
		ARC_DEBUG(ERR, "Failed to load userspace\n");
	}
//	sched_queue_proc(userspace);

	ARC_ENABLE_INTERRUPT;

	ARC_HANG;

	return 0;
}
