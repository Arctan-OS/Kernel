/**
 * @file elf.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
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
#include <loaders/elf.h>
#include <global.h>
#include <arch/pager.h>
#include <lib/util.h>
#include <mm/allocator.h>
#include <mm/pmm.h>

static void *elf_load64(void *page_tables, struct ARC_File *file) {
	ARC_DEBUG(INFO, "Loading 64-bit ELF file (%p)\n", file);

	struct Elf64_Ehdr *header = (struct Elf64_Ehdr *)alloc(sizeof(*header));

	if (header == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate header\n");
		return NULL;
	}

	vfs_seek(file, 0, SEEK_SET);
	vfs_read(header, 1, sizeof(*header), file);

	if (header->e_ident[ELF_EI_CLASS] != ELF_CLASS_64) {
		free(header);
		return NULL;
	}

	uint32_t section_count = header->e_shnum;
	struct Elf64_Shdr *section_headers = (struct Elf64_Shdr *)alloc(sizeof(*section_headers) * section_count);

	if (section_headers == NULL) {
		free(header);
		ARC_DEBUG(ERR, "Failed to allocate section header\n");
		return NULL;
	}

	vfs_seek(file, header->e_shoff, SEEK_SET);
	vfs_read(section_headers, 1, sizeof(*section_headers) * section_count, file);

	uint64_t entry_addr = header->e_entry;

	ARC_DEBUG(INFO, "Entry address: 0x%"PRIx64"\n", entry_addr);

	ARC_DEBUG(INFO, "Mapping sections (%d sections):\n", section_count);

	uint64_t phys_address = 0;
	for (uint32_t i = 0; i < section_count; i++) {
		struct Elf64_Shdr section_header = section_headers[i];

		uintptr_t load_base = section_header.sh_addr;
		size_t load_size = section_header.sh_size;

		ARC_DEBUG(INFO, "\t%3d: 0x%016"PRIx64", 0x%016"PRIx64" B | Type: %d\n", i, load_base, load_size, section_header.sh_type);

		if (load_base == 0 || load_size == 0) {
			ARC_DEBUG(INFO, "\t\tSkipping as load_base or size is 0\n");
			continue;
		}

		if (load_base + load_size >= ARC_HHDM_VADDR) {
			free(header);
			ARC_DEBUG(ERR, "\tCan't load, would overlap with kernel\n");
			return NULL;
		}

		size_t loaded = 0;
		while (loaded < load_size) {
			uint64_t load_addr = load_base + loaded;
			uint64_t jank = load_addr % PAGE_SIZE;
			size_t copy_size = min(PAGE_SIZE - jank, load_size - loaded);

			if (jank != 0) {
				if (section_header.sh_type != ELF_SHT_NOBITS) {
					vfs_seek(file, section_header.sh_offset + loaded, SEEK_SET);
					vfs_read((void *)(phys_address + jank), 1, copy_size, file);
				} else {
					memset((void *)(phys_address + jank), 0, copy_size);
				}

				loaded += copy_size;
				continue;
			}

			phys_address = (uintptr_t)pmm_alloc();

			if (phys_address == 0) {
				// Fail
				ARC_DEBUG(ERR, "Failed to allocate new page, quiting load\n");
				entry_addr = 0;
				break;
			}

			if (pager_map(page_tables, load_addr, ARC_HHDM_TO_PHYS(phys_address), PAGE_SIZE, 1 << ARC_PAGER_US) != 0) {
				// Fail
				ARC_DEBUG(ERR, "Failed to map in new page\n");
				pmm_free((void *)phys_address);
				entry_addr = 0;
				break;
			}

			if (section_header.sh_type != ELF_SHT_NOBITS) {
				vfs_seek(file, section_header.sh_offset + loaded, SEEK_SET);
				vfs_read((void *)phys_address, 1, copy_size, file);
			} else {
				memset((void *)phys_address, 0, copy_size);
			}

			loaded += copy_size;
		}
	}

	free(header);
	free(section_headers);

	return (void *)entry_addr;
}

void *load_elf(void *page_tables, struct ARC_File *file) {
	if (file == NULL) {
		ARC_DEBUG(ERR, "No file given to load\n");
		return NULL;
	}

	return elf_load64(page_tables, file);
}
