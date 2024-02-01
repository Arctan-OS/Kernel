/**
 * @file sse.c
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

#include <arch/x86/ctrl_regs.h>
#include <global.h>
#include <arch/x86/sse.h>

extern void _osxsave_support();

uint8_t fxsave_space[512] __attribute__((aligned(16), section(".data")));

void init_sse(int ecx, int edx) {
	// Check for SSE2
	if (((edx >> 25) & 1) == 0 && ((edx >> 26) & 1) == 0
	    && (ecx & 1) == 0 && ((ecx >> 9) & 1) == 0) {
		ARC_DEBUG(INFO, "No SSE/SSE2/SSE3/SSSE3 extensions\n");
		return;
	}

	_x86_getCR0();
	_x86_CR0 &= ~(1 << 2); // Disable x87 FPU emulation
	_x86_CR0 |= (1 << 1); //
	_x86_setCR0();

	_x86_getCR4();
	_x86_CR4 |= (1 << 9); // OSFXSR
	_x86_CR4 |= (1 << 10); // OSXMMEXCPT
	_x86_setCR4();

	if (((ecx >> 27) & 1) == 1) {
		_x86_CR4 |= (1 << 18); // OSXSAVE support
		_x86_setCR4();
		_osxsave_support();
	}

	// Set MXCSR Register
	ARC_DEBUG(INFO, "Enabled SSE support\n");
}
