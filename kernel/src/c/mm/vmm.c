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
#include "arch/x86/ctrl_regs.h"
#include "arctan.h"
#include "mm/pmm.h"
#include <mm/vmm.h>
#include <global.h>

static uint64_t *pml4 = NULL;

// Input an HHDM address
// Returns HHDM address
// The first 12 bits of the flags parameter are identical to
// a page tables flags. The last bit specifies the create flag.
uint64_t *Arc_GetPageTable(uint64_t *parent, int level, uint64_t vaddr, uint32_t flags) {
	int shift = ((level - 1) * 9) + 12;
	int index = (vaddr >> shift) & 0x1FF;

	uint64_t entry = parent[index];
	uint64_t *address = (uint64_t *)ARC_PHYS_TO_HHDM(parent[index] & 0x0000FFFFFFFFF000);

	if (((entry & 1) == 0 || (void *)ARC_HHDM_TO_PHYS(address) == NULL)
	    && ((flags & ARC_VMM_OVERW_FLAG) != 0 || (flags & ARC_VMM_CREAT_FLAG) != 0)) {
		// Not present, overwrite / create flag set
		address = (uint64_t *)Arc_AllocPMM();
		parent[index] = (uintptr_t)ARC_HHDM_TO_PHYS(address) | (flags & 0xFFF);
	}

	return address;
}

extern void _invlpg(uint64_t vaddr);

// Maps given physical address to given virtual address
// with given flags. The first 12 bits of the flags is
// identical to those of a PTE. The upper bit specifies
// the overwrite flag. If set, this flag will overwrite
// the entry, even if it is present.
int Arc_MapPage(uint64_t paddr, uint64_t vaddr, uint32_t flags) {
	if (pml4 == NULL) {
		ARC_DEBUG(ERR, "No PML4 loaded\n");
		return 2;
	}

	uint64_t *pml3 = Arc_GetPageTable(pml4, 4, vaddr, flags);
	uint64_t *pml2 = Arc_GetPageTable(pml3, 3, vaddr, flags);
	uint64_t *pml1 = Arc_GetPageTable(pml2, 2, vaddr, flags);

	if (pml3 == NULL || pml2 == NULL || pml1 == NULL) {
		return 1;
	}

	int entry_idx = (vaddr >> 12) & 0x1FF;

	if ((pml1[entry_idx] & 1) == 1 && (flags & ARC_VMM_OVERW_FLAG) == 0) {
		// Cannot overwrite
		return -1;
	}

	pml1[entry_idx] = paddr | (flags & 0xFFF);

	_invlpg(vaddr);

	return 0;
}

void Arc_InitVMM() {
	_x86_getCR3();

	pml4 = (uint64_t *)ARC_PHYS_TO_HHDM(_x86_CR3);
}
