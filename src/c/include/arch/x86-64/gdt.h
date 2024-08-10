/**
 * @file gdt.h
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
#ifndef ARC_ARCH_X86_64_GDT_H
#define ARC_ARCH_X86_64_GDT_H

/**
 * Load GDTR
 * */
extern void _install_gdt();

/**
 * Creates a TSS gate in the GDT.
 *
 * The function allocates a TSS structure and initializes it with
 * the given values. This initialized TSS is then placed in the next
 * free TSS descriptor within the GDT structure. The segment of the TSS
 * is returned.
 *
 * NOTE: The TSS allocated in using mm/allocator.h, this allocator needs
 *       to be initialized before any TSS can be created.
 * NOTE: Only IST1 and RSP0 need to be given as that those are the only
 *       stack switches that are planned in the kernel (as of 10/8/2024).
 * @param void *ist - The pointer to the top of the IST1 stack
 * @param void *rsp - The pointer to the top of the kernel's stack
 * @return the segment within the GDT that the TSS has been loaded into.
 * */
int create_tss(void *ist, void *rsp);

/**
 * Initialize the GDT
 *
 *
 * Initialize and load the GDT with basic gate descriptors for
 * kernel code, kernel data, user code, user data.
 *
 * NOTE: This function should only be called by the BSP, all other
 *       processors should call _install_gdt() to load the GDTR.
 * */

void init_gdt();

#endif
