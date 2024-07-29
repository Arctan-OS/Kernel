/**
 * @file ioapic.c
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
#include <arch/x86-64/apic/ioapic.h>
#include <global.h>
#include <mm/allocator.h>
#include <arch/x86-64/pager.h>
#include <mm/vmm.h>
#include <stdint.h>

struct ioapic_register {
        /// Register select
        uint32_t ioregsel __attribute__((aligned(16)));
        /// Data
        uint32_t iowin __attribute__((aligned(16)));
}__attribute__((packed));

struct ioapic_redir_tbl {
        uint8_t int_vec;
        uint8_t del_mod : 3;
        uint8_t dest_mod : 1;
        uint8_t del_stat : 1;
        uint8_t int_pol : 1;
        uint8_t irr : 1;
        uint8_t trigger : 1;
        uint8_t mask : 1;
        uint64_t resv0 : 39;
        uint8_t destination;
}__attribute__((packed));

uint32_t read_register(struct ioapic_register *ioapic, uint32_t reg) {
	ioapic->ioregsel = reg;
	return ioapic->iowin;
}

int write_register(struct ioapic_register *ioapic, uint32_t reg, uint32_t value) {
	ioapic->ioregsel = reg;
	ioapic->iowin = value;

	return 0;
}

int write_redir_tbl(struct ioapic_register *ioapic, int table_idx, struct ioapic_redir_tbl *table) {
	int low_dword_i = (table_idx * 2) + 0x10;
	int high_dword_i = low_dword_i + 1;

	uint64_t value = *(uint64_t *)table;

	write_register(ioapic, low_dword_i, value & UINT32_MAX);
	write_register(ioapic, high_dword_i, (value >> 32) & UINT32_MAX);

	return 0;
}

uint64_t read_redir_tbl(struct ioapic_register *ioapic, int table_idx) {
	int low_dword_i = (table_idx * 2) + 0x10;
	int high_dword_i = low_dword_i + 1;

	return (uint64_t)(read_register(ioapic, low_dword_i) | ((uint64_t)read_register(ioapic, high_dword_i) << 32));
}

int init_ioapic(uint32_t address, uint32_t gsi) {
	struct ioapic_register *ioapic = (struct ioapic_register *)((uintptr_t)address);

	int map_res = pager_map((uintptr_t)ioapic, (uintptr_t)ioapic, PAGE_SIZE, 1 << ARC_PAGER_4K | ARC_PAGER_PAT_UC);

	if (map_res != 0 && map_res != -5) {
		ARC_DEBUG(ERR, "Mapping failed\n");
		return -1;
	}

	struct ioapic_redir_tbl table = {
	        .mask = 0,
		.destination = 0,
		.dest_mod = 0,
		.del_mod = 0,
		.int_vec = 33,
		.int_pol = 0,
		.trigger = 0,
        };

	write_redir_tbl(ioapic, 1, &table);

	return 0;
}
