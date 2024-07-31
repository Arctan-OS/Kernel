/**
 * @file lapic.c
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
#include <arch/x86-64/apic/lapic.h>
#include <arch/x86-64/ctrl_regs.h>
#include <global.h>
#include <cpuid.h>
#include <arch/x86-64/pager.h>
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
        uint32_t isr[8] __attribute__((aligned(16)));
        uint32_t tmr[8] __attribute__((aligned(16)));
        uint32_t irr[8] __attribute__((aligned(16)));
        uint32_t err_stat_reg __attribute__((aligned(16)));
        uint32_t resv6 __attribute__((aligned(16)));
        uint32_t lvt_cmci_reg __attribute__((aligned(16)));
        uint32_t icr0 __attribute__((aligned(16))); // Lower
        uint32_t icr1 __attribute__((aligned(16))); // Upper
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

int lapic_eoi() {
        uint64_t lapic_msr = _x86_RDMSR(0x1B);
	struct lapic_reg *reg = (struct lapic_reg *)(((lapic_msr >> 12) & 0x0000FFFFFFFFFFFF) << 12);
	reg->eoi_reg = 0x0;
	return 0;
}

int lapic_ipi(uint8_t vector, uint8_t destination, uint32_t flags) {
	// Flags (bitwise)
	//     Offset | Description
	//     2:0    | Delivery Mode
	//     3      | Destination Mode
	//     5:4    | Destination Shorthand
	//     6      | Trigger Mode
	// NOTE: See Intel SDM Vol. 3 11.6.1 for information on the
	//       values of the above bit fields

	uint64_t lapic_msr = _x86_RDMSR(0x1B);
	struct lapic_reg *reg = (struct lapic_reg *)(((lapic_msr >> 12) & 0x0000FFFFFFFFFFFF) << 12);

	reg->icr0 = vector | ((flags & 0b111) << 8) | (((flags >> 3) & 1) << 11) |
		    (((flags >> 4) & 0b11) << 18) | (((flags >> 6) & 1) << 15) | (((flags & 0b111) != 0b101) << 14);
	reg->icr1 = destination << 24;

	return 0;
}

int lapic_ipi_poll() {
	// Returns the delivery status

	return 0;
}


int init_lapic() {
        register uint32_t eax;
        register uint32_t ebx;
        register uint32_t ecx;
        register uint32_t edx;

        __cpuid(0x01, eax, ebx, ecx, edx);

        if (((edx >> 9) & 1) == 0) {
                ARC_DEBUG(INFO, "No APIC on chip\n");
                return -1;
        }

        ARC_DEBUG(INFO, "Initializing LAPIC\n");

        uint64_t lapic_msr = _x86_RDMSR(0x1B);
        struct lapic_reg *reg = (struct lapic_reg *)(((lapic_msr >> 12) & 0x0000FFFFFFFFFFFF) << 12);

        if (((lapic_msr >> 8) & 1) == 1) {
                ARC_DEBUG(INFO, "BSP LAPIC\n");
        }

        lapic_msr |= (1 << 11);
        _x86_WRMSR(0x1B, lapic_msr);

	if (pager_map((uint64_t)reg, (uint64_t)reg, PAGE_SIZE, 1 << ARC_PAGER_RW | ARC_PAGER_PAT_UC) != 0) {
		ARC_DEBUG(ERR, "Failed to map LAPIC register\n");
		return -1;
	}

        ARC_DEBUG(INFO, "LAPIC register at %p\n", reg);
        // NOTE: Ignore bits 31:27 of reg->lapic_id on P6 and Pentium processors
        uint8_t ver = reg->lapic_ver & 0xFF;
        ARC_DEBUG(INFO, "LAPIC ID: 0x%X (%s)\n", reg->lapic_id >> 28, ((reg->spurious_int_vector >> 8) & 1) ? "disabled, enabling" : "enabled");
        if ((reg->spurious_int_vector >> 8) & 1) {
                // Enable LAPIC
                reg->spurious_int_vector |= 1 << 8;
        }
        ARC_DEBUG(INFO, "\tVersion: %d (%s)\n", ver, ver < 0xA ? "82489DX discrete APIC" : "Integrated APIC");
        ARC_DEBUG(INFO, "\tMax LVT: %d+1\n", ((reg->lapic_ver >> 16) & 0xFF));
        ARC_DEBUG(INFO, "\tEOI-broadcast supression: %s\n", (reg->lapic_ver >> 24) & 1 ? "yes" : "no");

        ARC_DEBUG(INFO, "Successfully initialized LAPIC\n");

        return 0;
}
