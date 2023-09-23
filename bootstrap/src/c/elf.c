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
#define PT_LOOS 0x60000000
#define PT_HIOS 0x6FFFFFFF
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7FFFFFFF

static const char *pt_names[] = {
	[PT_NULL] = "NULL",
	[PT_LOAD] = "LOAD",
	[PT_DYNAMIC] = "DYNAMIC",
	[PT_INTERP] = "INTERP",
	[PT_SHLIB] = "SHLIB",
	[PT_PHDR] = "PHDR",
	// [PT_LOOS] = "LOOS",
	// [PT_HIOS] = "HIOS",
	// [PT_LOPROC] = "LOPROC",
	// [PT_HIPROC] = "HIPROC"
};

uint64_t info[2];

uint64_t *load_elf(uint32_t elf_addr) {
	struct elf_header *elf_header = (struct elf_header *)((uintptr_t)elf_addr);
	struct program_header *prog_header = (struct program_header *)((uintptr_t)(elf_addr + elf_header->e_phoff));

	info[0] = prog_header->p_offset;
	info[1] = prog_header->p_vaddr;

	printf("Entry: %8X%8X\n", (uint32_t)(elf_header->e_entry >> 32), (uint32_t)(elf_header->e_entry));

	for (int i = 0; i < elf_header->e_phnum; i += elf_header->e_phentsize) {
		printf("Program Header %d, Type \"%s\", P:%8X%8X, V:%8X%8X\n", i, pt_names[prog_header->p_type], (uint32_t)(prog_header->p_paddr >> 32),
									(uint32_t)(prog_header->p_paddr), (uint32_t)(prog_header->p_vaddr >> 32), (uint32_t)(prog_header->p_vaddr));
	}

	return info;
}