/**
 * @file pager.h
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
#ifndef ARC_ARCH_X86_64_PAGER_H
#define ARC_ARCH_X86_64_PAGER_H

// * = Specific to Arctan
// Offset of page attributes
//  0: PWT, 1: PCD, 2: PAT
#define ARC_PAGER_PAT 0
// User / Supervisor bit
#define ARC_PAGER_US  3
// 1: Overwrite if entry is already present *
#define ARC_PAGER_OVW 4
// 1: Disable execution on entry
#define ARC_PAGER_NX  5
// 1: Use only 4K pages
#define ARC_PAGER_4K  6
// 1: Read and Write allowed
#define ARC_PAGER_RW  7

#include <stdint.h>
#include <stddef.h>

int pager_map(uint64_t virtual, uint64_t physical, size_t size, uint32_t attributes) ;
int pager_unmap(uint64_t virtual, size_t size);
int pager_set_attr(uint64_t virtual, size_t size, uint32_t attributes);

void init_pager();

#endif
