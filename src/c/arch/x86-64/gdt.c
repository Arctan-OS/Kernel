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
static struct gdt_entry gdt_entries[16];

void set_gdt_gate(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	gdt_entries[i].base1 = (base      ) & 0xFFFF;
	gdt_entries[i].base2 = (base >> 16) & 0xFF;
	gdt_entries[i].base3 = (base >> 24) & 0xFF;

	gdt_entries[i].access = access;

	gdt_entries[i].limit = (limit) & 0xFFFF;
	gdt_entries[i].flags_limit = (flags & 0x0F) << 4 | ((limit >> 16) & 0x0F);
}

extern void _install_gdt();
void Arc_InstallGDT() {
	set_gdt_gate(0, 0, 0, 0, 0);
	set_gdt_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xA); // Kernel Code 64
	set_gdt_gate(2, 0, 0xFFFFFFFF, 0x92, 0xC); // Kernel Data 32 / 64
	set_gdt_gate(3, 0, 0xFFFFFFFF, 0xF2, 0xC); // User Data 32 / 64
	set_gdt_gate(4, 0, 0xFFFFFFFF, 0xFA, 0xA); // User Code 64

	gdtr.size = sizeof(gdt_entries) * 8 - 1;
	gdtr.base = (uintptr_t)&gdt_entries;

	_install_gdt();

	ARC_DEBUG(INFO, "Installed GDT\n");
}
