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
	uint32_t flags;
	uint16_t gdt_size;
	uint64_t gdt_addr;
	uint64_t gdt_table[4];
	uint64_t stack;
	// EDX: Processor information
	uint32_t edx;
	// EAX: Return value of BIST
	uint32_t eax;
}__attribute__((packed));

struct ap_descriptor {
	uint32_t id;
	uint32_t bist;
	uint32_t model_info;
	struct ap_descriptor *next;
};

struct ap_descriptor *ap_list = NULL;

int tmp_counter = 1;
int smp_hold() {
	printf("Hello World from processor %d\n", tmp_counter++);
	for (;;) ARC_HANG;
}

int smp_list_aps() {
	struct ap_descriptor *desc = ap_list;
	while (desc != NULL) {
		printf("AP#%d:\n\tBIST: %"PRIx32"\n\tModel: %"PRIx32"\n", desc->id, desc->bist, desc->model_info);
		desc = desc->next;
	}

	return 0;
}

int init_smp(uint32_t lapic, uint32_t version) {
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
	info->entry = (uintptr_t)smp_hold;
	info->stack = ARC_HHDM_TO_PHYS(stack) + PAGE_SIZE - 0x10;
	info->gdt_size = 0x1F;
	info->gdt_addr = ARC_HHDM_TO_PHYS(&info->gdt_table);

	__asm__("" ::: "memory");

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

	printf("EAX (BIST): %X\n", info->eax);
	printf("EDX (Model & Stepping Info): %X\n", info->edx);

	while ((info->flags & 1) == 0) __asm__("pause");

	struct ap_descriptor *desc = (struct ap_descriptor *)alloc(sizeof(struct ap_descriptor));

	if (ap_list != NULL) {
		desc->id = ap_list->id + 1;
	} else {
		desc->id = 1;
	}

	desc->bist = info->eax;
	desc->model_info = info->edx;
	desc->next = ap_list;
	ap_list = desc;

	return 0;
}
