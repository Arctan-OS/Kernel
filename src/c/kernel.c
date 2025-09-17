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
#include "arch/interrupt.h"
#include "arch/pager.h"
#include "arch/pci.h"
#include "arch/start.h"
#include "arch/smp.h"
//#include "arch/x86-64/context.h"
//#include "arch/x86-64/interrupt.h"
//#include "arch/x86-64/util.h"
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

//#include "arch/io/port.h"
//#include "arch/x86-64/smp.h"



struct ARC_KernelMeta *Arc_KernelMeta = NULL;
struct ARC_BootMeta *Arc_BootMeta = NULL;

/*
void irq0(ARC_InterruptFrame *frame) {
	printf("Hello World, RAX is: %"PRIx64"\n", frame->gpr.rax);
	printf("Coming from: %p\n", frame->rip);
}

ARC_DEFINE_IRQ_HANDLER(irq0);
*/

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
/*
	uint64_t tsc_a = 0;
	uint64_t tsc_b = 0;

	tsc_a = arch_get_cycles(); void *a = pmm_alloc(2097152); tsc_b = arch_get_cycles();
	printf("a1: %p (delta: %lu)\n", a, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); void *a1 = pmm_alloc(2097152); tsc_b = arch_get_cycles();
	printf("a2: %p (delta: %lu)\n", a1, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); size_t t = pmm_free(a); tsc_b = arch_get_cycles();
	printf("free(a): %lu bytes (delta: %lu)\n", t, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); a = pmm_alloc(2097152); tsc_b = arch_get_cycles();
	printf("a3: %p (delta: %lu)\n", a, (tsc_b - tsc_a));

	tsc_a = arch_get_cycles(); void *c = pmm_alloc(1073741824); tsc_b = arch_get_cycles();
	printf("c1: %p (delta: %lu)\n", c, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); void *c1 = pmm_alloc(1073741824); tsc_b = arch_get_cycles();
	printf("c2: %p (delta: %lu)\n", c1, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); t = pmm_free(c); tsc_b = arch_get_cycles();
	printf("free(c): %lu bytes (delta: %lu)\n", t, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); c = pmm_alloc(1073741824); tsc_b = arch_get_cycles();
	printf("c3: %p (delta: %lu)\n", c, (tsc_b - tsc_a));

	tsc_a = arch_get_cycles(); void *d = pmm_alloc(PAGE_SIZE); tsc_b = arch_get_cycles();
	printf("d1: %p (delta: %lu)\n", d, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); void *d1 = pmm_alloc(PAGE_SIZE); tsc_b = arch_get_cycles();
	printf("d2: %p (delta: %lu)\n", d1, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); t = pmm_free(d); tsc_b = arch_get_cycles();
	printf("free(d): %lu bytes (delta: %lu)\n", t, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); d = pmm_alloc(PAGE_SIZE); tsc_b = arch_get_cycles();
	printf("d3: %p (delta: %lu)\n", d, (tsc_b - tsc_a));

	tsc_a = arch_get_cycles(); void *e = pmm_fast_page_alloc(); tsc_b = arch_get_cycles();
	printf("e1: %p (delta: %lu)\n", e, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); void *e1 = pmm_fast_page_alloc(); tsc_b = arch_get_cycles();
	printf("e2: %p (delta: %lu)\n", e1, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); t = pmm_fast_page_free(e); tsc_b = arch_get_cycles();
	printf("free(e): %lu bytes (delta: %lu)\n", t, (tsc_b - tsc_a));
	tsc_a = arch_get_cycles(); e = pmm_fast_page_alloc(); tsc_b = arch_get_cycles();
	printf("e3: %p (delta: %lu)\n", e, (tsc_b - tsc_a));

	tsc_a = arch_get_cycles(); void *b = pmm_alloc(100); tsc_b = arch_get_cycles();
	printf("b: %p (delta: %lu)\n", b, (tsc_b - tsc_a));

	void *f = pmm_alloc(PAGE_SIZE * 2);
	void *f1 = pmm_alloc(PAGE_SIZE * 2);
	size_t l = pmm_free(f);
	printf("free(%p) = %lu; %p\n", f, l, f1);
	void *f2 = pmm_alloc(1 << 18);
	printf("free(%p) = %lu; %p\n", f1, pmm_free(f1), f2);
	printf("free(%p) = %lu\n", f2, pmm_free(f2));
	printf("%p\n", pmm_alloc(PAGE_SIZE * 2));
//*/

	if (init_allocator(256) != 0) {
		ARC_DEBUG(ERR, "Failed to initialize kernel allocator\n");
		ARC_HANG;
	}
/*
	struct ARC_VMMMeta *meta = init_vmm((void *)0x10000000, UINT32_MAX);

	void *a = vmm_alloc(meta, 0x1000);
	printf("a = %p\n", a);
	void *b = vmm_alloc(meta, 0x1000);
	printf("b = %p\n", b);
	printf("free(a) = %lu\n", vmm_free(meta, a));
	void *c = vmm_alloc(meta, 0x1000);
	printf("c = %p\n", c);
	a = vmm_alloc(meta, 0x1000);
	printf("a = %p\n", a);

	printf("free(a) = %lu\n", vmm_free(meta, a));
	printf("free(b) = %lu\n", vmm_free(meta, b));
	printf("free(c) = %lu\n", vmm_free(meta, c));

	printf("Merge test\n");
	a = vmm_alloc(meta, 0x1000);
	printf("a = %p\n", a);
	b = vmm_alloc(meta, 0x1000);
	printf("b = %p\n", b);
	c = vmm_alloc(meta, 0x1000);
	printf("c = %p\n", c);

	printf("free(a) = %lu\n", vmm_free(meta, a));
	printf("free(b) = %lu\n", vmm_free(meta, b));

	b = vmm_alloc(meta, 0x1000);
	printf("b = %p\n", b);
	a = vmm_alloc(meta, 0x1000);
	printf("a = %p\n", a);
//*/

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

	/*
	interrupt_set(NULL, 3, irq_handler_irq0, true);

	printf("A Processor id: %d\n", Arc_CurProcessorDescriptor->descriptor->acpi_uid);
	__asm__("mov rax, 0xABAB; int 3");
	printf("B Processor id: %d\n", Arc_CurProcessorDescriptor->descriptor->acpi_uid);

	ARC_HANG;
	*/

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
	*/

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
