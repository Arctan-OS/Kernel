/**
 * @file vmm.h
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
#ifndef ARC_MM_VMM_H
#define ARC_MM_VMM_H

#define ARC_VMM_OVERW_FLAG (1 << 31)
#define ARC_VMM_CREAT_FLAG (1 << 30)

/**
 * @file vmm.c
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

#include <stdint.h>

// Input an HHDM address
// Returns HHDM address
// The first 12 bits of the flags parameter are identical to
// a page tables flags. The last bit specifies the create flag.
uint64_t *Arc_GetPageTable(uint64_t *parent, int level, uint64_t vaddr, uint32_t flags);

// Maps given physical address to given virtual address
// with given flags. The first 12 bits of the flags is
// identical to those of a PTE. The upper bit specifies
// the overwrite flag. If set, this flag will overwrite
// the entry, even if it is present.
int Arc_MapPage(uint64_t paddr, uint64_t vaddr, uint32_t flags);

void Arc_InitVMM();

#endif
