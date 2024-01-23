/**
 * @file cpuid.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is apart of Arctan.
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
#include <cpuid.h>
#include <arch/x86/cpuid.h>
#include <arch/x86/sse.h>
#include <global.h>

int check_features() {
	register uint32_t eax;
	register uint32_t ebx;
	register uint32_t ecx;
	register uint32_t edx;

	__cpuid(0x00, eax, ebx, ecx, edx);

	uint32_t max_basic_value = eax;

	__cpuid(0x01, eax, ebx, ecx, edx);

	if (((edx >> 6) & 1) == 0) {
		// PAE supported
		ARC_DEBUG(ERR, "PAE not supported\n")
		ARC_HANG
	}

	if (((edx >> 9) & 1) == 1) {
		// APIC On Chip
		// Commands to be submitted somewhere between 0xFFFE0000 and 0xFFFE0FFF
		ARC_DEBUG(INFO, "APIC On Chip\n")
	}

	init_sse(ecx, edx);

	__cpuid(0x80000000, eax, ebx, ecx, edx);

	uint32_t max_extended_value = eax;

	if (max_extended_value < 0x80000001) {
		// Extended functions not supported
		ARC_DEBUG(ERR, "No CPUID extended functions")
		ARC_HANG
	}

	__cpuid(0x80000001, eax, ebx, ecx, edx);

	if (((edx >> 29) & 1) == 0) {
		// No LM support, for booting Arctan, cannot continue
		ARC_DEBUG(ERR, "No LM support\n")
		ARC_HANG
	}

	return 0;
}

int enable_features() {
	return 0;
}
