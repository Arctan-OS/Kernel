/**
 * @file pager.c
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
#include <arch/x86-64/pager.h>
#include <arch/x86-64/ctrl_regs.h>
#include <mm/pmm.h>
#include <global.h>
#include <lib/util.h>

#define ADDRESS_MASK 0x000FFFFFFFFFF000
// Flags for Arc_BootMeta->paging_features
#define FLAGS_NO_EXEC (1 << 0)
// NOTE: Since PML4 is used 2MiB pages
//       ought to be supported (I have not
//       found a way to test for them)
#define FLAGS_1GIB (1 << 1)

#define ONE_GIB 0x40000000
#define TWO_MIB 0x200000

#define ARC_PAGER_PAT 0
#define ARC_PAGER_US  3
#define ARC_PAGER_OVW 4
#define ARC_PAGER_NX  5
#define ARC_PAGER_4K  6

static uint64_t *pml4 = NULL;

uint64_t *get_page_table(uint64_t *parent, int level, uint64_t virtual, uint32_t attributes) {
	int shift = ((level - 1) * 9) + 12;
	int index = (virtual >> shift) & 0x1FF;

	uint64_t entry = parent[index];
	uint64_t *address = (uint64_t *)ARC_PHYS_TO_HHDM(parent[index] & 0x0000FFFFFFFFF000);

	if (((entry & 1) == 0 || (void *)ARC_HHDM_TO_PHYS(address) == NULL)) {
		// Not present, overwrite / create flag set
		address = (uint64_t *)Arc_AllocPMM();
		// TODO: Attributes
		parent[index] = (uintptr_t)ARC_HHDM_TO_PHYS(address) | 0b11;
	}

	return address;
}

int Arc_MapPager(uint64_t virtual, uint64_t physical, size_t size, uint32_t attributes) {
	// Attributes (parentheses mark default)
	//  Bit:
	//   2:0 PAT
	//   3 1=User Page (0)=Supervisor
	//   4 1=Overwrite + Create (0)=Create
	//   5 (1)=Not executable (0)=Executable
	//   6 1=Use only 4KiB pages (0)=Best fit pages

	if (size == 0 || pml4 == NULL) {
		return -1;
	}

	// Round up size to be page aligned
	size = ALIGN(size, PAGE_SIZE);

	top:;

	if (size == 0) {
		return 0;
	}

	uint64_t *pml3 = get_page_table(pml4, 4, virtual, attributes);

	if (pml3 == NULL) {
		return -2;
	}

	int pml3_e = (virtual >> 30) & 0x1FF;

	if ((Arc_BootMeta->paging_features & FLAGS_1GIB) != 0 && ((attributes >> ARC_PAGER_4K) & 1) == 0
	    && size >= ONE_GIB && ((pml3[pml3_e] & 1) == 0 || ((attributes >> ARC_PAGER_OVW) & 1) == 1)) {
		// TODO: Add attributes
		pml3[pml3_e] = physical | 0b10000011; // PS | RW | P

		size -= ONE_GIB;
		virtual += ONE_GIB;
		physical += ONE_GIB;

		goto top;
	}


	uint64_t *pml2 = get_page_table(pml3, 3, virtual, attributes);

	if (pml2 == NULL) {
		return -3;
	}

	int pml2_e = (virtual >> 21) & 0x1FF;

	if (((attributes >> ARC_PAGER_4K) & 1) == 0 && size >= TWO_MIB
	    && ((pml2[pml2_e] & 1) == 0 || ((attributes >> ARC_PAGER_OVW) & 1) == 1)) {
		pml2[pml2_e] = physical | 0b10000011; // PS | RW | P

		size -= TWO_MIB;
		virtual += TWO_MIB;
		physical += TWO_MIB;

		goto top;
	}

	uint64_t *pml1 = get_page_table(pml2, 2, virtual, attributes);

	int entry_idx = ((uint64_t)virtual >> 12) & 0x1FF;

	if ((pml1[entry_idx] & 1) == 1 && ((attributes >> ARC_PAGER_OVW) & 1) == 0) {
		// Cannot overwrite
		return -3;
	}

	// TODO: Attributes
	pml1[entry_idx] = physical | 0b11;

	__asm__("invlpg [%0]" : : "r"(virtual) : );

	size -= PAGE_SIZE;
	virtual += PAGE_SIZE;
	physical += PAGE_SIZE;

	goto top;
}

int Arc_UnmapPager(uint64_t virtual, size_t size) {

	return 0;
}

int Arc_SetAttrsPager(uint64_t virtual, size_t size, uint32_t attributes) {
	// Set the attributes of the given region

	return 0;
}

void Arc_InitPager() {
	ARC_DEBUG(INFO, "Initializing pager\n");

	_x86_getCR3();
	pml4 = (uint64_t *)ARC_PHYS_TO_HHDM(_x86_CR3);

	ARC_DEBUG(INFO, "Initialized pager (%p)\n", pml4);
}
