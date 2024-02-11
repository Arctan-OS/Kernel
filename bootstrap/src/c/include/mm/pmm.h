/**
 * @file pmm.h
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
 * Simple PMM initializer.
*/
#ifndef ARC_MM_PMM_H
#define ARC_MM_PMM_H

#include <multiboot/multiboot2.h>
#include <mm/freelist.h>

/**
 * Initialize the PMM.
 *
 * Finds free regions of memory in the 32-bit address range
 * and initializes them into freelists.
 *
 * @param struct multiboot_tag_mmap *mmap - The MMAP tag provided by GRUB.
 * @param uintptr_t bootstrap_end - The highest address used by the bootstrapper.
 * @return Error code (0: success).
 * */
int init_pmm(struct multiboot_tag_mmap *mmap, uintptr_t bootstrap_end);

#endif
