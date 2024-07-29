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

static uint64_t *pml4 = NULL;

uint64_t get_entry_bits(int level, uint32_t attributes) {
	// Level 0: Page
	// Level 1: PT
	// Level 2: PD
	// Level 3: PDP
	// Level 4: PML4

	uint64_t bits = 0;

	bits |= ((attributes >> (ARC_PAGER_PAT)) & 1) << 2; // PWT
	bits |= ((attributes >> (ARC_PAGER_PAT + 1)) & 1) << 3; // PCD

	switch (level) {
		case 1: {
			// PAT
			bits |= ((attributes >> (ARC_PAGER_PAT + 2)) & 1) << 7;

			break;
		}

		case 2: {
			// PAT
			bits |= ((attributes >> (ARC_PAGER_PAT + 2)) & 1) << 12;
			// 2MiB pages unless 4K specified
			if (((attributes >> ARC_PAGER_RESV1) & 1) == 1) {
				bits |= (!((attributes >> (ARC_PAGER_4K)) & 1)) << 7;
			}

			break;
		}

		case 3: {
			// PAT
			bits |= ((attributes >> (ARC_PAGER_PAT + 2)) & 1) << 12;
			// 1GiB pages unless 4K specified
 			if (((attributes >> ARC_PAGER_RESV0) & 1) == 1) {
				bits |= (!((attributes >> (ARC_PAGER_4K)) & 1)) << 7;
			}

			break;
		}

		// On 4, 5, bit 7 is reserved
	}

	bits |= ((attributes >> ARC_PAGER_US) & 1) << 2;
	bits |= ((attributes >> ARC_PAGER_RW) & 1) << 1;
	bits |= 1; // Present

	if ((Arc_BootMeta->paging_features & FLAGS_NO_EXEC) == 1) {
		bits |= (uint64_t)((attributes >> ARC_PAGER_NX) & 1) << 63;
	}

	return bits;
}

/**
 * Get the next page table
 * @param uint64_t *parent - The parent table.
 * @param int level - The level of the parent table (4 = PML4).
 * @param uintptr_t virtual - The base virtual address.
 * @param uint32_t attributes - Specified attributes.
 * @parma int *index - The 64-bit entry in the parent corresponding to the returned table.
 * @return a non-NULL pointer to the next page table
 * */
uint64_t *get_page_table(uint64_t *parent, int level, uintptr_t virtual, uint32_t attributes, int *index) {
	int shift = ((level - 1) * 9) + 12;
	*index = (virtual >> shift) & 0x1FF;

	uint64_t entry = parent[*index];

	uint64_t *address = (uint64_t *)ARC_PHYS_TO_HHDM(parent[*index] & 0x0000FFFFFFFFF000);

	bool present = entry & 1;
	bool only_4k = (attributes >> ARC_PAGER_4K) & 1;
	bool can_gib = (attributes >> ARC_PAGER_RESV0) & 1;
	bool can_mib = (attributes >> ARC_PAGER_RESV1) & 1;
	bool no_create = (attributes >> ARC_PAGER_RESV2) & 1;

	if (!present && (only_4k || (level == 4 && !can_gib) || (level == 3 && !can_mib) || level == 2) && !no_create) {
		// Only make a new table if:
		//     The current entry is not present AND:
		//         - Mapping is only 4K
		//         - Can't make a GiB page
		//         - Can't make a MiB page

		address = (uint64_t *)pmm_alloc();


		if (address == NULL) {
			return NULL;
		}

		memset(address, 0, PAGE_SIZE);

		parent[*index] = (uintptr_t)ARC_HHDM_TO_PHYS(address) | get_entry_bits(level, attributes);
	}

	if (address == (void *)ARC_HHDM_VADDR) {
		return NULL;
	}

	return address;
}

int pager_map(uint64_t virtual, uint64_t physical, size_t size, uint32_t attributes) {
	if (size == 0 || pml4 == NULL) {
		return -1;
	}

	// Round up size to be page aligned
	size = ALIGN(size, PAGE_SIZE);

	top:;

	if (size == 0) {
		return 0;
	}

	uint32_t pass_attr = attributes;

	bool gib = (size >= ONE_GIB && !((attributes >> ARC_PAGER_4K) & 1)) && (Arc_BootMeta->paging_features & FLAGS_1GIB) != 0;
	bool mib = (size >= TWO_MIB && !((attributes >> ARC_PAGER_4K) & 1));

	// Signal if this function can create a new
	// GiB page
	pass_attr |= gib << ARC_PAGER_RESV0;
	// or a new 2MiB page entry
	pass_attr |= mib << ARC_PAGER_RESV1;

	int index = 0;

	uint64_t *pml3 = get_page_table(pml4, 4, virtual, pass_attr, &index);
	if (pml3 == NULL) {
		return -2;
	}

	uint64_t *pml2 = get_page_table(pml3, 3, virtual, pass_attr, &index);

	// Attempt to make a GiB page
	if (gib && (pml2 == NULL || ((attributes >> ARC_PAGER_OVW) & 1) == 1)) {
		// Can make a GiB page AND:
		//     - There is no pml2 table to overwrite
		//     - The overwrite flag is set
		pml3[index] |= physical | get_entry_bits(3, attributes);

		__asm__("invlpg [%0]" : : "r"(virtual) : );

		virtual += ONE_GIB;
		physical += ONE_GIB;
		size -= ONE_GIB;

		goto top;
	}

	if (pml2 == NULL) {
		return -3;
	}

	uint64_t *pml1 = get_page_table(pml2, 2, virtual, pass_attr, &index);

	// Attempt to make a 2MiB page
	if (mib && (pml1 == NULL || ((attributes >> ARC_PAGER_OVW) & 1) == 1)) {
		// Can make a 2MiB page AND:
		//     - There is no pml1 table to overwrite
		//     - The overwrite flag is set
		pml2[index] |= physical | get_entry_bits(2, attributes);

		__asm__("invlpg [%0]" : : "r"(virtual) : );

		virtual += TWO_MIB;
		physical += TWO_MIB;
		size -= TWO_MIB;

		goto top;
	}

	if (pml1 == NULL) {
		return -4;
	}

	index = ((uint64_t)virtual >> 12) & 0x1FF;

	if ((pml1[index] & 1) == 1 && ((attributes >> ARC_PAGER_OVW) & 1) == 0) {
		// Cannot overwrite
		// NOTE: This whole system of overwriting entries may cause memory
		//       leaks!
		return -5;
	}

	pml1[index] = physical | get_entry_bits(1, attributes);

	__asm__("invlpg [%0]" : : "r"(virtual) : );

	virtual += PAGE_SIZE;
	physical += PAGE_SIZE;
	size -= PAGE_SIZE;

	goto top;
}

static int pager_internal_unmap_recurse(uint64_t *table, int level, uintptr_t *virtual, size_t *size, bool fly) {
	if (table == NULL) {
		return -1;
	}

	int counter = -1;

	top:;

	int index = 0;
	uint64_t *entry = get_page_table(table, level, *virtual, 1 << ARC_PAGER_RESV2, &index);
	if (entry == NULL) {
		return -1;
	}

	if (*size == 0) {
		return counter;
	}

	// Detect if loop reached the end of the page table
	if (counter == -1) {
		counter = index;
	} else if (counter >= 512) {
		return 512;
	}

	// Check for leaves (pages)
	if (((table[index] >> 7) & 1) == 1 && (level == 3 || level == 2)) {
		// Large page, GiB or 2MiB depending on level has been found
		switch (level) {
			case 3: {
				table[index] = 0;
				__asm__("invlpg [%0]" : : "r"(*virtual) : );

				*virtual += ONE_GIB;
				*size -= ONE_GIB;
				counter++;
				goto top;
			}
			
			case 2: {
				table[index] = 0;
				__asm__("invlpg [%0]" : : "r"(*virtual) : );

				*virtual += TWO_MIB;
				*size -= TWO_MIB;
				counter++;
				goto top;
			}
		}
	}

	if (level == 1) {
		// 4K page has been found
		if (fly) {
			// Fly mapped, 4K page can be freed
			pmm_free(entry);
			table[index] = 0;
		} else {
			table[index] = 0;
		}

		__asm__("invlpg [%0]" : : "r"(*virtual) : );

		*virtual += PAGE_SIZE;
		*size -= PAGE_SIZE;
		counter++;
		goto top;
	}

	// Otherwsie page table has been found, in which case:
	if (pager_internal_unmap_recurse(entry, level - 1, virtual, size, fly) != 512) {
		// Check the page table to see if all entries are unused, in which case
		// the table can be freed and counter++;
		for (int i = 0; i < 512; i++) {
			if ((entry[i] & 1) == 1) {
				// An entry is present
				goto top;
			}
		}

		// There are no entries present
	}

	// Free the page table, go back up
	pmm_free(entry);
	table[index] = 0;
	counter++;

	goto top;
}

int pager_unmap(uint64_t virtual, size_t size) {
	if (size == 0) {
		return -1;
	}

	size = ALIGN(size, PAGE_SIZE);

	pager_internal_unmap_recurse(pml4, 4, &virtual, &size, 0);

	if (size != 0) {
		return -2;
	}

	return 0;
}

int pager_fly_map(uintptr_t virtual, size_t size, uint32_t attributes) {
	(void)virtual;
	(void)size;

	attributes |= 1 << ARC_PAGER_4K;
	size = ALIGN(size, PAGE_SIZE);

	for (size_t i = 0; i < size; i += PAGE_SIZE, virtual += PAGE_SIZE) {
		int index = 0;

		uint64_t *pml3 = get_page_table(pml4, 4, virtual, attributes, &index);
		if (pml3 == NULL) {
			return -1;
		}

		uint64_t *pml2 = get_page_table(pml3, 3, virtual, attributes, &index);
		if (pml2 == NULL) {
			return -1;
		}

		uint64_t *pml1 = get_page_table(pml2, 2, virtual, attributes, &index);
		if (pml1 == NULL) {
			return -1;
		}

		index = ((uint64_t)virtual >> 12) & 0x1FF;

		if ((pml1[index] & 1) == 1 && ((attributes >> ARC_PAGER_OVW) & 1) == 0) {
			// Cannot overwrite
			// NOTE: This whole system of overwriting entries may cause memory
			//       leaks!
			return -3;
		}

		pml1[index] = ARC_HHDM_TO_PHYS(pmm_alloc()) | get_entry_bits(1, attributes);

		__asm__("invlpg [%0]" : : "r"(virtual) : );
	}

	return 0;
}

int pager_fly_unmap(uintptr_t virtual, size_t size) {
	if (size == 0) {
		return -1;
	}

	size = ALIGN(size, PAGE_SIZE);

	pager_internal_unmap_recurse(pml4, 4, &virtual, &size, 1);

	if (size != 0) {
		return -2;
	}

	return 0;
}

int pager_set_attr(uint64_t virtual, size_t size, uint32_t attributes) {
	// Set the attributes of the given region
	if (size == 0) {
		return -1;
	}

	size = ALIGN(size, PAGE_SIZE);

	while (virtual < virtual + size) {
		int index = 0;
		uint64_t *pml3 = get_page_table(pml4, 4, virtual, 1 << ARC_PAGER_RESV2, &index);
		if (pml3 == NULL) {
			return -2;
		}

		uint64_t *pml2 = get_page_table(pml3, 3, virtual, 1 << ARC_PAGER_RESV2, &index);

		if ((pml3[index] & 1) == 1 && ((pml3[index] >> 7) & 1) == 1) {
			// GiB page
			uintptr_t address = (uintptr_t)(pml3[index] & ADDRESS_MASK);
			pml3[index] = address | get_entry_bits(3, attributes);

			__asm__("invlpg [%0]" : : "r"(virtual) : );

			virtual += ONE_GIB;
			continue;
		}

		if (pml2 == NULL) {
			return -3;
		}

		uint64_t *pml1 = get_page_table(pml2, 2, virtual, 1 << ARC_PAGER_RESV2, &index);

		if (((pml2[index] >> 7) & 1) == 1) {
			// MiB page
			uintptr_t address = (uintptr_t)(pml2[index] & ADDRESS_MASK);
			pml2[index] = address | get_entry_bits(2, attributes);

			__asm__("invlpg [%0]" : : "r"(virtual) : );

			virtual += TWO_MIB;
			continue;
		}

		if (pml1 == NULL) {
			return -4;
		}

		// KiB page
		index = ((uint64_t)virtual >> 12) & 0x1FF;

		if ((pml1[index] & 1) == 0) {
			return -5;
		}

		uintptr_t address = (uintptr_t)(pml1[index] & ADDRESS_MASK);
		pml1[index] = address | get_entry_bits(1, attributes);

		__asm__("invlpg [%0]" : : "r"(virtual) : );

		virtual += PAGE_SIZE;
	}

	return 0;

}

void init_pager() {
	ARC_DEBUG(INFO, "Initializing pager\n");

	_x86_getCR3();
	pml4 = (uint64_t *)ARC_PHYS_TO_HHDM(_x86_CR3);

	ARC_DEBUG(INFO, "Initialized pager (%p)\n", pml4);
}
