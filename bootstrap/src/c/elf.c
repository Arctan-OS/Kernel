/*
    Arctan - Operating System Kernel
    Copyright (C) 2023  awewsomegamer

    This file is apart of Arctan.

    Arctan is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "include/elf.h"
#include "include/interface.h"
#include "include/global.h"

struct elf_header {
	uint8_t  e_ident[16]; /* ELF identification */
	uint16_t e_type; /* Object file type */
	uint16_t e_machine; /* Machine type */
	uint32_t e_version; /* Object file version */
	uint64_t e_entry; /* Entry point address */
	uint64_t e_phoff; /* Program header offset */
	uint64_t e_shoff; /* Section header offset */
	uint32_t e_flags; /* Processor-specific flags */
	uint16_t e_ehsize; /* ELF header size */
	uint16_t e_phentsize; /* Size of program header entry */
	uint16_t e_phnum; /* Number of program header entries */
	uint16_t e_shentsize; /* Size of section header entry */
	uint16_t e_shnum; /* Number of section header entries */
	uint16_t e_shstrndx; /* Section name string table index */
}__attribute__((packed));

struct program_header {
	uint32_t p_type; /* Type of segment */
	uint32_t p_flags; /* Segment attributes */
	uint64_t p_offset; /* Offset in file */
	uint64_t p_vaddr; /* Virtual address in memory */
	uint64_t p_paddr; /* Reserved */
	uint64_t p_filesz; /* Size of segment in file */
	uint64_t p_memsz; /* Size of segment in memory */
	uint64_t p_align; /* Alignment of segment */
}__attribute__((packed));

#define PT_NULL 0 
#define PT_LOAD 1 
#define PT_DYNAMIC 2
#define PT_INTERP 3 
#define PT_NOTE 4
#define PT_SHLIB 5 
#define PT_PHDR 6 

static const char *pt_names[] = {
	[PT_NULL] = "NULL",
	[PT_LOAD] = "LOAD",
	[PT_DYNAMIC] = "DYNAMIC",
	[PT_INTERP] = "INTERP",
	[PT_SHLIB] = "SHLIB",
	[PT_PHDR] = "PHDR",
};

uint64_t info[2];

uint64_t *load_elf(uint32_t elf_addr) {
	struct elf_header *elf_header = (struct elf_header *)((uintptr_t)elf_addr);
	struct program_header *prog_header = (struct program_header *)((uintptr_t)(elf_addr + elf_header->e_phoff));

	info[0] = prog_header->p_offset;
	info[1] = elf_header->e_entry;

	printf("Entry: %"PRIX64"\n", (elf_header->e_entry));

	for (int i = 0; i < elf_header->e_phnum; i += elf_header->e_phentsize) {
		printf("Program Header %d, Type \"%s\", P:%"PRIX64", V:%"PRIX64", Offset: %"PRIX64"\n", i, pt_names[prog_header->p_type], prog_header->p_paddr, prog_header->p_vaddr, prog_header->p_offset);
	}

	return info;
}
