/**
 * @file apic.c
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
#include <arch/x86-64/apic.h>
#include <arch/x86-64/ctrl_regs.h>
#include <global.h>
#include <cpuid.h>
#include <mm/vmm.h>
#include <mm/allocator.h>

struct lapic_reg {
	uint32_t resv0 __attribute__((aligned(16)));
	uint32_t resv1 __attribute__((aligned(16)));
	uint32_t lapic_id __attribute__((aligned(16)));
	uint32_t lapic_ver __attribute__((aligned(16)));
	uint32_t resv2 __attribute__((aligned(16)));
	uint32_t resv3 __attribute__((aligned(16)));
	uint32_t resv4 __attribute__((aligned(16)));
	uint32_t resv5 __attribute__((aligned(16)));
	uint32_t tpr __attribute__((aligned(16)));
	uint32_t apr __attribute__((aligned(16)));
	uint32_t ppr __attribute__((aligned(16)));
	uint32_t eoi_reg __attribute__((aligned(16)));
	uint32_t rrd __attribute__((aligned(16)));
	uint32_t logical_dest_reg __attribute__((aligned(16)));
	uint32_t dest_form_reg __attribute__((aligned(16)));
	uint32_t spurious_int_vector __attribute__((aligned(16)));
	uint32_t isr0 __attribute__((aligned(16)));
	uint32_t isr1 __attribute__((aligned(16)));
	uint32_t isr2 __attribute__((aligned(16)));
	uint32_t isr3 __attribute__((aligned(16)));
	uint32_t isr5 __attribute__((aligned(16)));
	uint32_t isr6 __attribute__((aligned(16)));
	uint32_t isr7 __attribute__((aligned(16)));
	uint32_t tmr0 __attribute__((aligned(16)));
	uint32_t tmr1 __attribute__((aligned(16)));
	uint32_t tmr2 __attribute__((aligned(16)));
	uint32_t tmr3 __attribute__((aligned(16)));
	uint32_t tmr4 __attribute__((aligned(16)));
	uint32_t tmr5 __attribute__((aligned(16)));
	uint32_t tmr6 __attribute__((aligned(16)));
	uint32_t tmr7 __attribute__((aligned(16)));
	uint32_t irr0 __attribute__((aligned(16)));
	uint32_t irr1 __attribute__((aligned(16)));
	uint32_t irr2 __attribute__((aligned(16)));
	uint32_t irr3 __attribute__((aligned(16)));
	uint32_t irr4 __attribute__((aligned(16)));
	uint32_t irr5 __attribute__((aligned(16)));
	uint32_t irr6 __attribute__((aligned(16)));
	uint32_t irr7 __attribute__((aligned(16)));
	uint32_t err_stat_reg __attribute__((aligned(16)));
	uint32_t resv6 __attribute__((aligned(16)));
	uint32_t lvt_cmci_reg __attribute__((aligned(16)));
	uint32_t icr0 __attribute__((aligned(16)));
	uint32_t icr1 __attribute__((aligned(16)));
	uint32_t lvt_timer_reg __attribute__((aligned(16)));
	uint32_t lvt_thermal_reg __attribute__((aligned(16)));
	uint32_t lvt_preformance_reg __attribute__((aligned(16)));
	uint32_t lvt_lint0_reg __attribute__((aligned(16)));
	uint32_t lvt_lint1_reg __attribute__((aligned(16)));
	uint32_t lvt_err_reg __attribute__((aligned(16)));
	uint32_t init_count_reg __attribute__((aligned(16)));
	uint32_t cur_count_reg __attribute__((aligned(16)));
	uint32_t resv7 __attribute__((aligned(16)));
	uint32_t div_conf_reg __attribute__((aligned(16)));
	uint32_t resv8 __attribute__((aligned(16)));
}__attribute__((packed));

struct ioapic_definition {
        uint32_t id;
        uint32_t address;
        uint32_t gsi;
        struct ioapic_definition *next;
};
// Backwards (last entry is the first one defined)
struct ioapic_definition *ioapic_list = NULL;

int Arc_DefineIOAPIC(uint32_t id, uint32_t address, uint32_t gsi) {
        struct ioapic_definition *def = (struct ioapic_definition *)Arc_SlabAlloc(sizeof(struct ioapic_definition));

        if (def == NULL) {
                return -1;
        }

        def->id = id;
        def->address = address;
        def->gsi = gsi;

        if (ioapic_list != NULL) {
                def->next = ioapic_list;
        }

        ioapic_list = def;

        return 0;
}

int Arc_InitAPIC() {
	register uint32_t eax;
	register uint32_t ebx;
	register uint32_t ecx;
	register uint32_t edx;

	__cpuid(0x01, eax, ebx, ecx, edx);

	if (((edx >> 9) & 1) == 0) {
		ARC_DEBUG(INFO, "No APIC on chip\n");
		return -1;
	}

	ARC_DEBUG(INFO, "Initializing APIC\n");

	uint64_t lapic_msr = _x86_RDMSR(0x1B);
	struct lapic_reg *reg = (struct lapic_reg *)(((lapic_msr >> 12) & 0x0000FFFFFFFFFFFF) << 12);

	if (((lapic_msr >> 8) & 1) == 1) {
		ARC_DEBUG(INFO, "BSP LAPIC\n");
	}

        // TODO: Uninitialize 8259A, start up IOAPICs

	lapic_msr |= (1 << 11);
	_x86_WRMSR(0x1B, lapic_msr);
        // TODO: Enable LAPIC reg->spurious_int_vector |= 1 << 8;

	// NOTE: Should be mapped as UC (for when we implement MTRRs)
	Arc_MapPageVMM((uintptr_t)reg, (uintptr_t)reg, ARC_VMM_CREAT_FLAG | 3);

	ARC_DEBUG(INFO, "LAPIC register at %p\n", reg);
	// NOTE: Ignore bits 31:27 of reg->lapic_id on P6 and Pentium processors
	uint8_t ver = reg->lapic_ver & 0xFF;
	ARC_DEBUG(INFO, "LAPIC ID: 0x%X (%s)\n", reg->lapic_id >> 28, ((reg->spurious_int_vector >> 8) & 1) ? "disabled" : "enabled");
	ARC_DEBUG(INFO, "\tVersion: %d (%s)\n", ver, ver < 0xA ? "82489DX discrete APIC" : "Integrated APIC");
	ARC_DEBUG(INFO, "\tMax LVT: %d+1\n", ((reg->lapic_ver >> 16) & 0xFF));
	ARC_DEBUG(INFO, "\tEOI-broadcast supression: %s\n", (reg->lapic_ver >> 24) & 1 ? "yes" : "no");

        ARC_DEBUG(INFO, "IOAPIC Definition:\n");
        struct ioapic_definition *def = ioapic_list;
        while (def != NULL) {
                ARC_DEBUG(INFO, "\tIOAPIC %d (0x%x, %d)\n", def->id, def->address, def->gsi);
                def = def->next;
        }

	ARC_DEBUG(INFO, "Successfully initialized APIC\n");

	return 0;
}
