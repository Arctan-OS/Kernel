/**
 * @file initramfs.c
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

#include "mm/freelist.h"
#include "mm/vmm.h"
#include <fs/initramfs.h>
#include <global.h>

/**
 * CPIO binary file header.
 *
 * As described by https://www.systutorials.com/docs/linux/man/5-cpio/.
 * */
struct ARC_CPIOHeader {
	uint16_t magic;
	uint16_t dev;
	uint16_t ino;
	uint16_t mode;
	uint16_t uid;
	uint16_t gid;
	uint16_t nlink;
	uint16_t rdev;
	uint16_t mtime[2];
	uint16_t namesize;
	uint16_t filesize[2];
}__attribute__((packed));

int load_file(char *path, uint64_t *load_address) {
	uint32_t offset = 0;

	while (offset < initramfs_size) {
		struct ARC_CPIOHeader *header = (struct ARC_CPIOHeader *)(initramfs + offset);

		if (header->magic != 0070707) {
			// Header magic mismatch
			return 1;
		}

		uint16_t name_size = header->namesize + (header->namesize % 2);
		uint32_t file_size = (header->filesize[0] << 16) | header->filesize[1];

		char *name_base = (char *)(initramfs + offset + sizeof(struct ARC_CPIOHeader));
		uint8_t *file_data = (uint8_t *)(name_base + name_size);

		if (strcmp(name_base, path) != 0) {
			// Not the file we are looking for, go to next file
			offset += sizeof(struct ARC_CPIOHeader) + name_size + file_size;
			continue;
		}

		// Found file
		ARC_DEBUG(INFO, "Found file %s\n", name_base);

		int aligned_size = ALIGN(file_size, 0x1000);
		void *base = NULL;

		if (*load_address != 0) {
			uint64_t *pml4_old = pml4;

			for (int i = 0; i < aligned_size >> 12; i++) {
				void *page = Arc_ListAlloc(&physical_mem);

				if (base == NULL) {
					base = page;
				}

				pml4 = map_page(pml4, *load_address + (i << 12), (uintptr_t)page, 0);

				if (pml4 == NULL || (pml4 != pml4_old && pml4_old != NULL)) {
					// Mapping failure
					pml4 = pml4_old;
					return -2;
				}
			}

			memset(base, 0, aligned_size);
			memcpy(base, file_data, file_size);

			*load_address = (uint64_t)base;

			ARC_DEBUG(INFO, "Sucessfully mapped %s to 0x%"PRIX64"\n", name_base, *load_address);

			return 0;
		}

		void *last = NULL;

		for (int i = 0; i < aligned_size >> 12; i++) {
			void *page = Arc_ListAlloc(&physical_mem);

			if (base == NULL) {
				base = page;
				last = page;
			}

			if ((uintptr_t)page - (uintptr_t)last > 0x1000) {
				// TODO: Uh oh, have to go find another base
				//       and then try to find a contiguous set of
				//       pages
				ARC_DEBUG(ERR, "Allocation for %s is not contiguous, no implemented handler, unexpected behavior to happen\n");
			}

			last = page;
		}

		memset(base, 0, aligned_size);
		memcpy(base, file_data, file_size);

		ARC_DEBUG(INFO, "Sucessfully mapped %s to 0x%"PRIX64"\n", name_base, (uint64_t)base);

		return 0;
	}

	// File not found
	return -1;
}
