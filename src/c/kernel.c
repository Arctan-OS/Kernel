/**
 * @file kernel.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
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
#include <userspace/thread.h>
#include <lib/atomics.h>
#include <global.h>
#include <arch/start.h>
#include <arch/smp.h>
#include <fs/vfs.h>
#include <interface/framebuffer.h>
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

struct ARC_BootMeta *Arc_BootMeta = NULL;
struct ARC_TermMeta Arc_MainTerm = { 0 };
struct ARC_Resource *Arc_InitramfsRes = NULL;
struct ARC_File *Arc_FontFile = NULL;
static char Arc_MainTerm_mem[180 * 120] = { 0 };
struct ARC_Process *Arc_ProcessorHold = NULL;

/*
int proc_test(int processor) {
	struct ARC_File *file = NULL;
	int i = vfs_open("/initramfs/boot/credit.txt", 0, ARC_STD_PERM, (void *)&file);
	char data[26] = { 0 };
	vfs_read(&data, 1, 24, file);
	printf("Processor %d has arrived %d %s\n", processor, i, data);
	vfs_close(file);
	
	size_t size = 26;
	struct ARC_VFSNodeInfo info = {
                .driver_arg = &size,
		.mode = ARC_STD_PERM,
		.type = ARC_VFS_N_BUFF,
		.driver_index = -1,
        };
	vfs_create("/write_test.txt", &info);
	vfs_open("/write_test.txt", 0, ARC_STD_PERM, &file);
	vfs_seek(file, 3 * (processor - 1), SEEK_SET);
	sprintf_(data, "C%d ", processor);
	vfs_write(data, 1, 3, file);
	vfs_close(file);
	
	vfs_open("/initramfs/boot/reference.txt", 0, ARC_STD_PERM, &file);
	vfs_read(data, 1, 24, file);
	printf("Link resolves: %s\n", data);
	vfs_close(file);
	
	printf("Processor did not deadlock %d\n", processor);
	
	for (int i = 0; i < 100; i++) {
		void *memory = pmm_alloc(0x2000);
		*(uint16_t *)memory = 'A' + processor;
		printf("[%d]: PMM/vbuddy(2/2): Got address %p %d %s\n", i, memory, processor, memory);
		size_t ret = pmm_free(memory);
		printf("[%d]: PMM/vbuddy(2/2): Freed %lu %d\n", i, ret, processor);
		
		memory = alloc(16);
		*(uint16_t *)memory = 'A' + processor;
		printf("[%d]: GEN/slab(2/2): Got address %p %d %s\n", i, memory, processor, memory);
		ret = free(memory);
		printf("[%d]: GEN/slab(2/2): Freed %lu %d\n", i, ret, processor);
	}
	
	ARC_HANG;
}
*/

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

	if (parse_boot_info() != 0) {
		ARC_DEBUG(ERR, "Failed to parse boot information\n");
		ARC_HANG;
	}

	if (Arc_MainTerm.framebuffer != NULL) {
		Arc_MainTerm.term_width = (Arc_MainTerm.fb_width / Arc_MainTerm.font_width);
		Arc_MainTerm.term_height = (Arc_MainTerm.fb_height / Arc_MainTerm.font_height);
	}

	if (init_pmm((struct ARC_MMap *)Arc_BootMeta->arc_mmap, Arc_BootMeta->arc_mmap_len) != 0) {
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

	Arc_InitramfsRes = init_resource(ARC_DRIDEF_INITRAMFS_SUPER, (void *)ARC_PHYS_TO_HHDM(Arc_BootMeta->initramfs));
	vfs_mount("/initramfs/", Arc_InitramfsRes);

	init_arch();
	ARC_DISABLE_INTERRUPT;

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

	vfs_link("/initramfs/boot/ANTIQUE.F14", "/fonts/font.fnt", -1);
	vfs_open("/fonts/font.fnt", 0, ARC_STD_PERM, &Arc_FontFile);

	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");

	for (int i = 0; i < 60; i++) {
		for (int y = 0; y < Arc_MainTerm.fb_height; y++) {
			for (int x = 0; x < Arc_MainTerm.fb_width; x++) {
				ARC_FB_DRAW(Arc_MainTerm.framebuffer, x, (y * Arc_MainTerm.fb_width), Arc_MainTerm.fb_bpp, (x * y * i / 300) & 0x3FFF);
			}
		}
	}
	
	/*
	// The Battery Resilience Test
	// Tell every processor, except the current one, to go to the test function.
	// This function will test will subject the desired parts of the kernel to 
	// a true parallel environment in order to test if synchronization primitives
	// are properly used.
	
	ARC_ENABLE_INTERRUPT;
	
	struct ARC_ProcessorDescriptor *proc = smp_get_proc_desc();
	proc = proc->next;
	
	while (proc != NULL) {
		proc->flags |= 1 << 1;
		while ((proc->flags >> 1) & 1) __asm__("pause");
		
		smp_jmp(proc, proc_test, 1, proc->acpi_uid);
		
		proc = proc->next;
	}
	
	for (;;) ARC_HANG;
	*/

	init_scheduler();
	sched_queue(Arc_ProcessorHold, ARC_SCHED_PRI_LO);
	
	struct ARC_Process *userspace = process_create_from_file(1, "/initramfs/userspace.elf");
	if (userspace == NULL) {
		ARC_DEBUG(ERR, "Failed to load userspace\n");
	}
	sched_queue(userspace, ARC_SCHED_PRI_HI);

	smp_switch_to_userspace();
	
	ARC_ENABLE_INTERRUPT;

	for (;;) ARC_HANG;

	return 0;
}
