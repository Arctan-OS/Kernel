/**
 * @file elf.c
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
#include "global.h"
#include "mm/freelist.h"
#include <elf/elf.h>
#include <mm/vmm.h>
#include <string.h>

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11

static const char *section_types[] = {
	[SHT_NULL] = "NULL",
	[SHT_PROGBITS] = "PROGBITS",
	[SHT_SYMTAB] = "SYMTAB",
	[SHT_STRTAB] = "STRTAB",
	[SHT_RELA] = "RELA",
	[SHT_HASH] = "HASH",
	[SHT_DYNAMIC] = "DYNAMIC",
	[SHT_NOTE] = "NOTE",
	[SHT_NOBITS] = "NOBITS",
	[SHT_REL] = "REL",
	[SHT_SHLIB] = "SHLIB",
	[SHT_DYNSYM] = "DYNSYM",
};

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint32_t Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef uint64_t Elf64_Sxword;

struct Elf64_Ehdr {
	unsigned char e_ident[16]; /* ELF identification */
	Elf64_Half e_type; /* Object file type */
	Elf64_Half e_machine; /* Machine type */
	Elf64_Word e_version; /* Object file version */
	Elf64_Addr e_entry; /* Entry point address */
	Elf64_Off e_phoff; /* Program header offset */
	Elf64_Off e_shoff; /* Section header offset */
	Elf64_Word e_flags; /* Processor-specific flags */
	Elf64_Half e_ehsize; /* ELF header size */
	Elf64_Half e_phentsize; /* Size of program header entry */
	Elf64_Half e_phnum; /* Number of program header entries */
	Elf64_Half e_shentsize; /* Size of section header entry */
	Elf64_Half e_shnum; /* Number of section header entries */
	Elf64_Half e_shstrndx; /* Section name string table index */
}__attribute__((packed));

struct Elf64_Shdr {
	Elf64_Word sh_name; /* Section name */
	Elf64_Word sh_type; /* Section type */
	Elf64_Xword sh_flags; /* Section attributes */
	Elf64_Addr sh_addr; /* Virtual address in memory */
	Elf64_Off sh_offset; /* Offset in file */
	Elf64_Xword sh_size; /* Size of section */
	Elf64_Word sh_link; /* Link to other section */
	Elf64_Word sh_info; /* Miscellaneous information */
	Elf64_Xword sh_addralign; /* Address alignment boundary */
	Elf64_Xword sh_entsize; /* Size of entries, if section has table */
}__attribute__((packed));

struct Elf64_Sym {
	Elf64_Word st_name; /* Symbol name */
	unsigned char st_info; /* Type and Binding attributes */
	unsigned char st_other; /* Reserved */
	Elf64_Half st_shndx; /* Section table index */
	Elf64_Addr st_value; /* Symbol value */
	Elf64_Xword st_size; /* Size of object (e.g., common) */
}__attribute__((packed));

struct Elf64_Rel {
	Elf64_Addr r_offset; /* Address of reference */
	Elf64_Xword r_info; /* Symbol index and type of relocation */
}__attribute__((packed));

struct Elf64_Rela {
	Elf64_Addr r_offset; /* Address of reference */
	Elf64_Xword r_info; /* Symbol index and type of relocation */
	Elf64_Sxword r_addend; /* Constant part of expression */
}__attribute__((packed));

struct Elf64_Phdr {
	Elf64_Word p_type; /* Type of segment */
	Elf64_Word p_flags; /* Segment attributes */
	Elf64_Off p_offset; /* Offset in file */
	Elf64_Addr p_vaddr; /* Virtual address in memory */
	Elf64_Addr p_paddr; /* Reserved */
	Elf64_Xword p_filesz; /* Size of segment in file */
	Elf64_Xword p_memsz; /* Size of segment in memory */
	Elf64_Xword p_align; /* Alignment of segment */
}__attribute__((packed));

// TODO: Support PIE objects
// Return e_entry: success
// Return 0: not ELF
// Return 1: mapping failed
uint64_t load_elf(uint64_t *pml4, void *file) {
	uint64_t *old_pml4 = pml4;
	struct Elf64_Ehdr *header = (struct Elf64_Ehdr *)file;

	if (header->e_ident[0] != 0x7F || header->e_ident[1] != 'E' ||
	    header->e_ident[2] != 'L' || header->e_ident[3] != 'F') {
		// Memory is not of type ELF, error
		return 0;
	}

	ARC_DEBUG(INFO, "-----------\n")
	ARC_DEBUG(INFO, "Loading ELF\n")

	ARC_DEBUG(INFO, "Entry at: 0x%"PRIX64"\n", header->e_entry);

	struct Elf64_Shdr *section_headers = ((struct Elf64_Shdr *)((uintptr_t)file + header->e_shoff));
	char *str_table_base = (char *)((uintptr_t)file + section_headers[header->e_shstrndx].sh_offset);

	for (int i = 0; i < header->e_shnum; i++) {
		struct Elf64_Shdr section = section_headers[i];

		if (section.sh_type == SHT_NULL) {
			// Section is NULL, just ignore
			continue;
		}

		// Get the physical address of the section in the file
		uint64_t paddr_file = (uintptr_t)file + section.sh_offset;
		// Get the virtual address at which the section wants to be loaded
		uint64_t vaddr = section.sh_addr;
		// Calculate the number of pages used by the section
		// If the section size is less than < 0x1000, don't bother rounding up
		int highest_address = (section.sh_size >= 0x1000) ? ALIGN(section.sh_size, 0x1000) / 0x1000 : 0;

		ARC_DEBUG(INFO, "Section %d \"%s\" of type %s\n", i, (str_table_base + section.sh_name), section_types[section.sh_type]);
		ARC_DEBUG(INFO, "\tOffset: 0x%"PRIX64" Size: 0x%"PRIX64" B, 0x%"PRIX64":0x%"PRIX64"\n", section.sh_offset, section.sh_size, paddr_file, vaddr);

		if (section.sh_type != SHT_PROGBITS && section.sh_type != SHT_NOBITS) {
			// If the section isn't needed by the program, ignore it
			continue;
		}

		// Page count == 0, but it is a section we need?
		// Re-calculate without the check if the size < 0
		highest_address = ALIGN(section.sh_size, 0x1000) / 0x1000;

		ARC_DEBUG(INFO, "\tNeed to map %d page(s)\n", highest_address);

		// Map into memory
		for (int j = 0; j < highest_address; j++) {
			uint64_t paddr = paddr_file + (j << 12);

			if (section.sh_type == SHT_NOBITS) {
				// Section is not present in file, allocate
				// memory for it
				paddr = (uintptr_t)Arc_ListAlloc(&physical_mem);
				memset((void *)paddr, 0, 0x1000);

				ARC_DEBUG(INFO, "\tSection is of type NOBITS, allocated 0x%"PRIX64" for it\n", paddr);
			}

			pml4 = map_page(pml4, vaddr + (j << 12), paddr, 0);

			if (pml4 == NULL || (pml4 != old_pml4 && old_pml4 != NULL)) {
				ARC_DEBUG(ERR, "Mapping failed\n");
				return 1;
			}
		}
	}

	ARC_DEBUG(INFO, "-----------\n")

	return header->e_entry;
}
