/**
 * @file smp.c
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
#include <mp/smp.h>
#include <arch/x86-64/apic/lapic.h>
#include <interface/printf.h>
#include <global.h>
#include <arch/x86-64/pager.h>
#include <mm/pmm.h>
#include <arch/x86-64/cmos.h>
#include <lib/util.h>
#include <arch/x86-64/ctrl_regs.h>
#include <mm/allocator.h>
#include <cpuid.h>
#include <arch/x86-64/gdt.h>
#include <arch/x86-64/idt.h>
#include <arch/x86-64/sse.h>

extern uint8_t __AP_START_BEGIN__;
extern uint8_t __AP_START_END__;
extern uint8_t __AP_START_INFO__;

struct ap_start_info {
	uint64_t pml4;
	uint64_t entry;
	// Flags
	//  Bit | Description
	//  0   | LM reached, core jumping to kernel_entry
	//  1   | Processor core started
	//  2   | Use PAT given
	//  3   | NX
	uint32_t flags;
	uint16_t gdt_size;
	uint64_t gdt_addr;
	uint64_t gdt_table[4];
	uint64_t stack;
	// EDX: Processor information
	uint32_t edx;
	// EAX: Return value of BIST
	uint32_t eax;
	uint64_t pat;
}__attribute__((packed));

static struct ARC_ProcessorDescriptor Arc_ProcessorList[256];

int smp_move_ap_high_mem(struct ap_start_info *info) {
	// NOTE: APs are initialized sequentially, therefore only one AP
	//       should be executing this code at a time

	register uint32_t eax;
        register uint32_t ebx;
        register uint32_t ecx;
        register uint32_t edx;

        __cpuid(0x1, eax, ebx, ecx, edx);

	if (((info->flags >> 2) & 1) == 1) {
                ARC_DEBUG(INFO, "PATs present, initializing\n");
                _x86_WRMSR(0x277, info->pat);
        }

	int id = lapic_get_id();

	if (id == -1) {
		ARC_DEBUG(ERR, "This is impossible, how can you have LAPIC but no LAPIC?\n");
		ARC_HANG;
	}

	__cpuid(0x80000001, eax, ebx, ecx, edx);

 	if (((info->flags >> 3) & 1) == 1) {
		ARC_DEBUG(INFO, "NX bit present\n");
		uint64_t efer = _x86_RDMSR(0xC0000080);
		efer |= 1 << 11;
		_x86_WRMSR(0xC0000080, efer);
	}

	_install_gdt();
	_install_idt();
	init_sse();
	lapic_calibrate_timer();

	void *stack = pmm_contig_alloc(2);
	// TODO Set RBP and RSP

	info->flags |= 1;

	smp_hold(&Arc_ProcessorList[id]);

	return 0;
}

int smp_hold(struct ARC_ProcessorDescriptor *processor) {
	ARC_DEBUG(INFO, "Holding processor %d\n", processor->processor);
	for (;;);
}

int smp_list_aps() {
	for (uint32_t i = 0; i < Arc_ProcessorCounter; i++) {
		printf("-------------------------\nProcessor #%d:\n\tLAPIC: %d\n\tBIST:  0x%x\n\tModel: 0x%x\n-------------------------\n", Arc_ProcessorList[i].processor, i, Arc_ProcessorList[i].bist, Arc_ProcessorList[i].model_info);
	}

	return 0;
}

int init_smp(uint32_t lapic, uint32_t acpi_uid, uint32_t acpi_flags, uint32_t version) {
	Arc_ProcessorList[lapic].processor = Arc_ProcessorCounter++;
	Arc_ProcessorList[lapic].acpi_uid = acpi_uid;
	Arc_ProcessorList[lapic].acpi_flags = acpi_flags;
	Arc_ProcessorList[lapic].bist = 0;
	Arc_ProcessorList[lapic].model_info = 0; // TODO: Set this properly

	if (lapic == (uint32_t)lapic_get_id()) {
		// BSP
		return 0;
	}


	// Set warm reset in CMOS and warm reset
        // vector in BDA (not sure if this is needed)
        // cmos_write(0xF, 0xA);
        // *(uint32_t *)(ARC_PHYS_TO_HHDM(0x467)) = (uint32_t)((uintptr_t)code);

	// Allocate space in low memory, copy ap_start code to it
	// which should bring AP to kernel_main where it will be
	// detected, logged, and put into smp_hold
	void *code = pmm_low_alloc();
	void *stack = pmm_low_alloc();

	pager_map(ARC_HHDM_TO_PHYS(code), ARC_HHDM_TO_PHYS(code), PAGE_SIZE, 1 << ARC_PAGER_4K | 1 << ARC_PAGER_RW);
	pager_map(ARC_HHDM_TO_PHYS(stack), ARC_HHDM_TO_PHYS(stack), PAGE_SIZE, 1 << ARC_PAGER_4K | 1 << ARC_PAGER_RW);

	memset(code, 0, PAGE_SIZE);
	memcpy(code, (void *)&__AP_START_BEGIN__, (size_t)((uintptr_t)&__AP_START_END__ - (uintptr_t)&__AP_START_BEGIN__));
	struct ap_start_info *info = (struct ap_start_info *)((uintptr_t)code + ((uintptr_t)&__AP_START_INFO__ - (uintptr_t)&__AP_START_BEGIN__));

	_x86_getCR3();
	info->pml4 = _x86_CR3;
	info->entry = (uintptr_t)smp_move_ap_high_mem;
	info->stack = ARC_HHDM_TO_PHYS(stack) + PAGE_SIZE - 0x10;
	info->gdt_size = 0x1F;
	info->gdt_addr = ARC_HHDM_TO_PHYS(&info->gdt_table);
	info->pat = _x86_RDMSR(0x277);
	info->flags |= (1 << 2);
	info->flags |= ((Arc_BootMeta->paging_features & 1) << 3);

	ARC_MEM_BARRIER;

	// AP start procedure
	// INIT IPI
	lapic_ipi(0, lapic, ARC_LAPIC_IPI_INIT | ARC_LAPIC_IPI_ASSERT);
	while (lapic_ipi_poll()) __asm__("pause");
	// INIT De-assert IPI
	lapic_ipi(0, lapic, ARC_LAPIC_IPI_INIT | ARC_LAPIC_IPI_DEASRT);
	while (lapic_ipi_poll()) __asm__("pause");

	// If (lapic->version != 82489DX)
	if (version >= 0xA) {
		uint8_t vector = (((uintptr_t)code) >> 12) & 0xFF;

		// SIPI
		lapic_ipi(vector, lapic, ARC_LAPIC_IPI_START | ARC_LAPIC_IPI_ASSERT);
		while (lapic_ipi_poll()) __asm__("pause");
		// SIPI
		lapic_ipi(vector, lapic, ARC_LAPIC_IPI_START | ARC_LAPIC_IPI_ASSERT);
		while (lapic_ipi_poll()) __asm__("pause");
	}

	while (((info->flags >> 1) & 1) == 0) __asm__("pause");

	Arc_ProcessorList[lapic].bist = info->eax;
	Arc_ProcessorList[lapic].model_info = info->edx;

	while ((info->flags & 1) == 0) __asm__("pause");

	pager_unmap(ARC_HHDM_TO_PHYS(code), PAGE_SIZE);
	pager_unmap(ARC_HHDM_TO_PHYS(stack), PAGE_SIZE);

	pmm_low_free(code);
	pmm_low_free(stack);

	return 0;
}
