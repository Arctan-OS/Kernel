/**
 * @file vmm.c
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
#include <arch/x86-64/ctrl_regs.h>
#include <arctan.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <global.h>
#include <cpuid.h>

static uint64_t *pml4 = NULL;

#define PAGE_ATTRIBUTE(n, val) (uint64_t)((uint64_t)(val & 0b111) << (n * 8))

uint64_t *Arc_GetPageTableVMM(uint64_t *parent, int level, uint64_t vaddr, uint32_t flags) {
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

int Arc_MapPageVMM(uint64_t paddr, uint64_t vaddr, uint32_t flags) {
	if (pml4 == NULL) {
		ARC_DEBUG(ERR, "No PML4 loaded\n");
		return 2;
	}

	uint64_t *pml3 = Arc_GetPageTableVMM(pml4, 4, vaddr, flags);
	uint64_t *pml2 = Arc_GetPageTableVMM(pml3, 3, vaddr, flags);
	uint64_t *pml1 = Arc_GetPageTableVMM(pml2, 2, vaddr, flags);

	if (pml3 == NULL || pml2 == NULL || pml1 == NULL) {
		return 1;
	}

	int entry_idx = (vaddr >> 12) & 0x1FF;

	if ((pml1[entry_idx] & 1) == 1 && (flags & ARC_VMM_OVERW_FLAG) == 0) {
		// Cannot overwrite
		return -1;
	}

	pml1[entry_idx] = paddr | (flags & 0xFFF);

	__asm__("invlpg [%0]" : : "r"(vaddr) : );

	return 0;
}

int Arc_UnmapPageVMM(uint64_t vaddr) {
        return 0;
}

void Arc_SetPML4(uint64_t *new_pml4) {
	pml4 = new_pml4;

	_x86_CR3 = (uintptr_t)ARC_HHDM_TO_PHYS(pml4);
	_x86_setCR3();
}

void Arc_InitVMM() {
	ARC_DEBUG(INFO, "Initializing VMM\n");
	_x86_getCR3();
	pml4 = (uint64_t *)ARC_PHYS_TO_HHDM(_x86_CR3);

        register uint32_t eax;
        register uint32_t ebx;
        register uint32_t ecx;
        register uint32_t edx;

        __cpuid(0x1, eax, ebx, ecx, edx);

        if (((edx >> 16) & 1) == 1) {
                ARC_DEBUG(INFO, "PATs present, initializing\n");

                uint64_t msr = _x86_RDMSR(0x277);

                ARC_DEBUG(INFO, "Previous PATs: 0x%016"PRIX64"\n", msr);
                msr = 0;
                msr |= PAGE_ATTRIBUTE(0, 0x06); // WB
                msr |= PAGE_ATTRIBUTE(1, 0x00); // UC
                msr |= PAGE_ATTRIBUTE(2, 0x07); // UC-
                msr |= PAGE_ATTRIBUTE(3, 0x01); // WC
                msr |= PAGE_ATTRIBUTE(4, 0x04); // WT
                msr |= PAGE_ATTRIBUTE(5, 0x05); // WP
                ARC_DEBUG(INFO, "Current PATs : 0x%016"PRIX64"\n", msr);

                _x86_WRMSR(0x277, msr);
        }

	ARC_DEBUG(INFO, "Initialized VMM (%p)\n", pml4);
}
