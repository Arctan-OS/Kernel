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
#include <arch/x86-64/apic/apic.h>
#include <arch/x86-64/apic/lapic.h>
#include <mm/slab.h>
#include <fs/vfs.h>
#include <global.h>

int Arc_InitAPIC() {
	struct ARC_VFSNode *apic = Arc_GetNodeVFS("/dev/acpi/rsdt/apic", 0);

	if (apic == NULL) {
		return -1;
	}

	uint8_t data[256] = { 0 };

	int i = 0;
	while (Arc_HeadlessReadVFS(data, 256, i++, apic) > 0) {
		ARC_DEBUG(INFO, "%d: Type %d\n", i, data[0]);
	}

	Arc_InitLAPIC();
	return 0;
}
