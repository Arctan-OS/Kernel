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
#include <mm/vmm.h>
#include <mm/freelist.h>

void memset(void *mem, uint8_t value, size_t size) {
	for (size_t i = 0; i < size; i++) {
		*(uint8_t *)(mem + i) = value;
	}
}

// Return NULL: error
uint64_t *create_table(uint64_t *parent, uint64_t vaddr, int level) {
	if (parent == NULL) {
		return NULL;
	}

	int shift = ((level - 1) * 9) + 12;

	if ((parent[(vaddr >> shift) & 0x1FF] & 1) == 1) {
		// Entry already exists
		return (uint64_t *)(parent[(vaddr >> shift) & 0x1FF] & 0x0000FFFFFFFFF000);
	}

	uint64_t *table = (uint64_t *)Arc_ListAlloc(&physical_mem);

	if (table == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate new PML%d table for virtual address 0x%"PRIX64"\n", level, vaddr)
		return NULL;
	}

	memset(table, 0, 0x1000);

	parent[(vaddr >> shift) & 0x1FF] = (uintptr_t)table | 3;
	return table;
}

// Return PML4: success
// Return NULL: failure
uint64_t *map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr, int overwrite) {
	if (pml4 == NULL) {
		pml4 = (uint64_t *)Arc_ListAlloc(&physical_mem);
		memset(pml4, 0, 0x1000);
	}

	// TODO: Check that the sign extension is correct

	paddr &= 0x0000FFFFFFFFF000;
	vaddr &= 0x0000FFFFFFFFF000;

	int err = 0;

	uint64_t *pml3;
	uint64_t *pml2;
	uint64_t *pml1;

	err += (pml3 = create_table(pml4, vaddr, 4)) == NULL;
	err += (pml2 = create_table(pml3, vaddr, 3)) == NULL;
	err += (pml1 = create_table(pml2, vaddr, 2)) == NULL;

	if (err > 0) {
		// One or more of the pointers are NULL, can't continue
		ARC_DEBUG(ERR, "One or more errors encountered while getting / creating PML3, PML2, and PML1 tables\n")
		return NULL;
	}

	if ((pml4[(vaddr >> 39) & 0x1FF] & 1) == 0) {
		pml4[(vaddr >> 39) & 0x1FF] = ((uintptr_t)pml3) | 3;
	}

	if ((pml3[(vaddr >> 30) & 0x1FF] & 1) == 0) {
		pml3[(vaddr >> 30) & 0x1FF] = ((uintptr_t)pml2) | 3;
	}

	if ((pml2[(vaddr >> 21) & 0x1FF] & 1) == 0) {
		pml2[(vaddr >> 21) & 0x1FF] = ((uintptr_t)pml1) | 3;
	}

	if ((pml1[(vaddr >> 12) & 0x1FF] & 1) == 1 && !overwrite) {
		// Cannot overwrite already existing entry
		ARC_DEBUG(ERR, "Cannot overwrite 0x%"PRIX64":0x%"PRIX64"\n", vaddr, paddr)
		return NULL;
	}

	pml1[(vaddr >> 12) & 0x1FF] = paddr | 3;

	return pml4;
}
