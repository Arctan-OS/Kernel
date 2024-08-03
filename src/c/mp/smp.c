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

extern uint8_t __AP_START_BEGIN__;
extern uint8_t __AP_START_END__;
extern uint8_t __AP_START_INFO__;

struct ap_start_info {
	uint64_t pml4;
	uint64_t entry;
	// Flags
	//  Bit | Description
	//  0   | LM reached, core jumping to kernel_entry
	uint32_t flags;
	uint16_t gdt_size;
	uint64_t gdt_addr;
	uint64_t gdt_table[3];
	uint32_t stack;
}__attribute__((packed));

int tmp_lock = 0;
int tmp_counter = 1;
int smp_hold() {
	while (tmp_lock) __asm__("pause");
	tmp_lock = 1;
	printf("Hello World from processor %d\n", tmp_counter++);
	tmp_lock = 0;
	for (;;) ARC_HANG;
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
	memset(code, 0, PAGE_SIZE);
	memcpy(code, (void *)&__AP_START_BEGIN__, (size_t)((uintptr_t)&__AP_START_END__ - (uintptr_t)&__AP_START_BEGIN__));
	struct ap_start_info *info = (struct ap_start_info *)((uintptr_t)code + ((uintptr_t)&__AP_START_INFO__ - (uintptr_t)&__AP_START_BEGIN__));

	_x86_getCR3();
	info->pml4 = _x86_CR3;
	info->entry = (uintptr_t)smp_hold;
	info->stack = ARC_HHDM_TO_PHYS(stack) + PAGE_SIZE - 0x10;
	info->gdt_size = 0x17;
	info->gdt_addr = ((uintptr_t)info->gdt_table) << 16;

	printf("%x\n", info->stack);
	printf("%x %x %x\n", ARC_HHDM_TO_PHYS(&info->stack), ARC_HHDM_TO_PHYS(info), (ARC_HHDM_TO_PHYS(&info->stack) - ARC_HHDM_TO_PHYS(info)));

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

	// Verify sync
	for (;;);

	pmm_low_free(code);

	return 0;
}
