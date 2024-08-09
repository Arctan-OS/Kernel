/**
 * @file gdt.c
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
 * Change out the GDT to better suit 64-bit mode, remove no longer needed 32-bit
 * segments.
*/
#include <global.h>
#include <arch/x86-64/gdt.h>

struct gdt_header {
	uint16_t size;
	uint64_t base;
}__attribute__((packed));
struct gdt_header gdtr;

struct gdt_entry {
	uint16_t limit;
	uint16_t base1;
	uint8_t base2;
	uint8_t access;
	uint8_t flags_limit;
	uint8_t base3;
}__attribute__((packed));

struct tss_entry {
	uint16_t limit;
	uint16_t base1;
	uint8_t base2;
	uint8_t access;
	uint8_t flags_limit;
	uint8_t base3;
	uint32_t base4;
	uint32_t resv;
}__attribute__((packed));

struct tss {
	uint32_t resv0;
	uint32_t rsp0_low;
	uint32_t rsp0_high;
	uint32_t rsp1_low;
	uint32_t rsp1_high;
	uint32_t rsp2_low;
	uint32_t rsp2_high;
	uint32_t resv1;
	uint32_t resv2;
	uint32_t ist1_low;
	uint32_t ist1_high;
	uint32_t ist2_low;
	uint32_t ist2_high;
	uint32_t ist3_low;
	uint32_t ist3_high;
	uint32_t ist4_low;
	uint32_t ist4_high;
	uint32_t ist5_low;
	uint32_t ist5_high;
	uint32_t ist6_low;
	uint32_t ist6_high;
	uint32_t ist7_low;
	uint32_t ist7_high;
	uint32_t resv3;
	uint32_t resv4;
	uint16_t resv5;
	uint16_t io_port_bmp_off;
}__attribute__((packed));

struct tss task_state_segments[ARC_MAX_PROCESSORS];

struct gdt_entry_container {
	struct gdt_entry gdt[8];
	struct tss_entry tss[ARC_MAX_PROCESSORS];
	int next_tss;
}__attribute__((packed));

struct gdt_entry_container gdt_entries;

void set_gdt_gate(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	gdt_entries.gdt[i].base1 = (base      ) & 0xFFFF;
	gdt_entries.gdt[i].base2 = (base >> 16) & 0xFF;
	gdt_entries.gdt[i].base3 = (base >> 24) & 0xFF;

	gdt_entries.gdt[i].access = access;

	gdt_entries.gdt[i].limit = (limit) & 0xFFFF;
	gdt_entries.gdt[i].flags_limit = (flags & 0x0F) << 4 | ((limit >> 16) & 0x0F);
}

void set_tss_gate(int i, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	gdt_entries.tss[i].base1 = (base      ) & 0xFFFF;
	gdt_entries.tss[i].base2 = (base >> 16) & 0xFF;
	gdt_entries.tss[i].base3 = (base >> 24) & 0xFF;
	gdt_entries.tss[i].base4 = (base >> 32) & UINT32_MAX;

	gdt_entries.tss[i].access = access;

	gdt_entries.tss[i].limit = (limit) & 0xFFFF;
	gdt_entries.tss[i].flags_limit = (flags & 0x0F) << 4 | ((limit >> 16) & 0x0F);
}

extern void _install_tss(uint32_t index);
int create_tss(void *ist, void *rsp) {
	int index = gdt_entries.next_tss++;

	set_tss_gate(index, (uint64_t)&task_state_segments[index], sizeof(struct tss) - 1, 0x89, 0x0);

	task_state_segments[index].ist1_low = (((uintptr_t)ist) & UINT32_MAX);
	task_state_segments[index].ist1_high = (((uintptr_t)ist) >> 32) & UINT32_MAX;
	task_state_segments[index].rsp0_low = (((uintptr_t)rsp) & UINT32_MAX);
	task_state_segments[index].rsp0_high = (((uintptr_t)rsp) >> 32) & UINT32_MAX;

	_install_tss(sizeof(gdt_entries.gdt) + (index * sizeof(struct tss_entry)));

	return sizeof(gdt_entries.gdt) + (index * sizeof(struct tss_entry));
}

void init_gdt() {
	set_gdt_gate(0, 0, 0, 0, 0);
	set_gdt_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xA); // Kernel Code 64
	set_gdt_gate(2, 0, 0xFFFFFFFF, 0x92, 0xC); // Kernel Data 32 / 64
	set_gdt_gate(3, 0, 0xFFFFFFFF, 0xF2, 0xC); // User Data 32 / 64
	set_gdt_gate(4, 0, 0xFFFFFFFF, 0xFA, 0xA); // User Code 64

	gdt_entries.next_tss = 0;

	gdtr.size = sizeof(gdt_entries) * 8 - 1;
	gdtr.base = (uintptr_t)&gdt_entries;

	_install_gdt();

	ARC_DEBUG(INFO, "Installed GDT\n");
}
