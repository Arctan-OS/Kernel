#include "../include/elf/elf.h"
#include "../include/mm/vmm.h"

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

// Return 0: success
// Return 1: not ELF
// Return 2: mapping failed
int load_elf(uint64_t *pml4, void *file) {
	struct Elf64_Ehdr *header = (struct Elf64_Ehdr *)file;

	if (header->e_ident[0] != 0x7F || header->e_ident[1] != 'E' ||
	    header->e_ident[2] != 'L' || header->e_ident[3] != 'F') {
		return 1;
	}

	ARC_DEBUG(INFO, "-----------\n")
	ARC_DEBUG(INFO, "Loading ELF\n")

	for (int i = 0; i < header->e_phnum; i++) {
		struct Elf64_Phdr prog_header = ((struct Elf64_Phdr *)((uintptr_t)file + header->e_phoff))[i];

		uint64_t paddr = (uintptr_t)file + prog_header.p_offset;
		uint64_t vaddr = prog_header.p_vaddr;
		uint64_t page_count = ALIGN(prog_header.p_memsz, 0x1000) / 0x1000;

		ARC_DEBUG(INFO, "Header %d of type %s (%d)\n", i, pt_names[prog_header.p_type], prog_header.p_flags)
		ARC_DEBUG(INFO, "\tOffset: 0x%"PRIX64" Size: 0x%"PRIX64" (0x%"PRIX64":0x%"PRIX64")\n", prog_header.p_offset, prog_header.p_memsz, paddr, vaddr)

		for (uint64_t j = 0; j < page_count; j++) {
			pml4 = map_page(pml4, vaddr + (j << 12), paddr + (j << 12), 1);

			if (pml4 == NULL) {
				ARC_DEBUG(ERR, "Mapping failed\n")
				return 2;
			}
		}
	}

	ARC_DEBUG(INFO, "-----------\n")

	return 0;
}
