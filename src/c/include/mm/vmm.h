/**
 * @file vmm.h
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
#ifndef ARC_MM_VMM_H
#define ARC_MM_VMM_H

/// The overwrite flag.
#define ARC_VMM_OVERW_FLAG (1 << 31)
/// The create flag.
#define ARC_VMM_CREAT_FLAG (1 << 30)

#include <stdint.h>

// Input an HHDM address
// Returns HHDM address
// The first 12 bits of the flags parameter are identical to
// a page tables flags. The last bit specifies the create flag.
/**
 * Get the pointer to the next page table.
 *
 * Returns the HHDM address of the next table in the HHDM.
 *
 * The layout of flags is identical to Arc_MapPageVMM.
 *
 * @param uint64_t *parent - The page table in which to find the pointer.
 * @param int level - Level of the parent page table (4 for PML4).
 * @param uint64_t vaddr - The virtual address.
 * @param uint32_t flags - The flags which the page table should inherit and behavior flags.
 * */
uint64_t *Arc_GetPageTableVMM(uint64_t *parent, int level, uint64_t vaddr, uint32_t flags);

/**
 * Map the given physical address to the given virtual address
 *
 * The first 12 bits of the flag are identical to each tables'
 * entry's first 12 bits (see figure 4-11 of Intel® 64 and IA-32 Architectures
 * Software Developer’s Manual Volume 3 (3A, 3B, 3C, & 3D): System Programming Guide)
 *
 * Bit 31 is the overwrite flag. If set, if the supplied PTE already exists
 * then it will be overwritten. This flag also enables the behavior create flag.
 *
 * Bit 30 is the create flag. If set, if the chain of paging tables to the
 * PTE does not exist, it will be created. This flag will not enable the
 * behavior of the overwrite flag.
 *
 * @param uint64_t paddr - The physical page frame to map.
 * @param uint64_t vaddr - The virtual page to map \a paddr to.
 * @param uint32_t flags - The flags which the page tables should inherit and behavior flags.
 * for the function.
 * @return Error code (0: success).
 * */
int Arc_MapPageVMM(uint64_t paddr, uint64_t vaddr, uint32_t flags);

/**
 * Change the current page table's address.
 *
 * Sets internal pml4 which Arc_MapPageVMM and Arc_GetPageTableVMM
 * rely on.
 *
 * Sets CR3.
 *
 * @param uint64_t *pml4 - The address of the pml4 in the HHDM.
 * */
void Arc_SetPML4(uint64_t *pml4);

/**
 * Initialize the virtual memory manager.
 * */
void Arc_InitVMM();

#endif
