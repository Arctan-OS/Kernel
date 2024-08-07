/**
 * @file smp.h
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
#include <stdint.h>

struct ARC_ProcessorDescriptor {
	uint32_t processor;
	uint32_t acpi_uid;
	uint32_t acpi_flags;
	uint32_t bist;
	uint32_t model_info;
};

// NOTE: The index in Arc_ProcessorList corresponds to the LAPIC ID
//       acquired from CPUID 0x1, ebx >> 24
static struct ARC_ProcessorDescriptor Arc_ProcessorList[256];
static uint32_t Arc_ProcessorCounter;

int smp_hold(struct ARC_ProcessorDescriptor *processor);
int smp_list_aps();

int init_smp(uint32_t lapic, uint32_t acpi_uid, uint32_t acpi_flags, uint32_t version);
