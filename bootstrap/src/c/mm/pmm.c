/**
 * @file pmm.c
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
#include "mm/freelist.h"
#include <mm/pmm.h>
#include <global.h>

// Return 0: success
int init_pmm(struct multiboot_tag_mmap *mmap, uintptr_t bootstrap_end) {
	ARC_DEBUG(INFO, "Initializing PMM\n")

	int entries = (mmap->size - sizeof(struct multiboot_tag_mmap)) / mmap->entry_size;

	for (int i = 0; i < entries; i++) {
		struct multiboot_mmap_entry entry = mmap->entries[i];

		if ((entry.addr < bootstrap_end && entry.addr + entry.len < bootstrap_end) || entry.type != MULTIBOOT_MEMORY_AVAILABLE) {
			// Entry not suitable for a freelist table
			continue;
		}

		// This entry either contains the bootstrap_end or is located after
		ARC_DEBUG(INFO, "Entry %d suitable for freelist\n", i)

		if ((uint32_t)(entry.addr >> 32) > 0) {
			ARC_DEBUG(INFO, "\tEntry %d is above 32-bit address range, ignoring\n", i)
			continue;
		}

		void *base = (void *)((uint32_t)entry.addr);
		void *ciel = (void *)((uint32_t)entry.addr + (uint32_t)entry.len - 0x1000);

		if (entry.addr < bootstrap_end && entry.addr + entry.len > bootstrap_end) {
			// bootstrap_end contained is in this entry
			base = (void *)ALIGN(bootstrap_end, 0x1000);
		}

		ARC_DEBUG(INFO, "\tInitializing freelist 0x%"PRIXPTR" -> 0x%"PRIXPTR"\n", (uintptr_t)base, (uintptr_t)ciel)

		if (physical_mem.base == NULL) {
			Arc_InitializeFreelist(base, ciel, 0x1000, &physical_mem);
		} else {
			struct ARC_FreelistMeta b = { 0 };
			struct ARC_FreelistMeta c = { 0 };
			Arc_InitializeFreelist(base, ciel, 0x1000, &physical_mem);
			int err = Arc_ListLink(&physical_mem, &b, &c);

			if (err != 0) {
				ARC_DEBUG(ERR, "Failed to link lists A and B\n")
				// TODO: Do something, currently we hope this does not happen.
				//       Idealy pick the larger list and copy it into A.
			}
		}
	}

	ARC_DEBUG(INFO, "Initialized PMM\n")

	return 0;
}
