#include <stdint.h>
#include "include/global.h"
#include "include/interface/printf.h"
#include "include/arch/x86/idt.h"
#include "include/arch/x86/gdt.h"
#include "include/arctan.h"
#include "include/multiboot/mbparse.h"
#include "include/mm/freelist.h"
#include "include/mm/vmm.h"
#include "include/multiboot/multiboot2.h"
#include "include/arch/x86/cpuid.h"
#include "include/elf/elf.h"

struct ARC_FreelistMeta physical_mem = { 0 };
uint64_t page_count = 0;
void *kernel_elf = NULL;
uint64_t *pml4 = NULL;

int helper(void *mbi, uint32_t signature) {
	ARC_DEBUG(INFO, "Loaded\n");

	if (signature != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printf("System was not booted using a multiboot2 bootloader, stopping.\n");
		ARC_HANG
	}

	check_features();

	install_gdt();
	install_idt();

	read_mb2i(mbi);

	// Identity map first MB
	for (int i = 0; i < 512; i++) {
		pml4 = map_page(pml4, i << 12, i << 12, 1);

		if (pml4 == NULL) {
			ARC_DEBUG(ERR, "Mapping failed\n")
			ARC_HANG
		}
	}

	// Create HHDM
	for (uint64_t i = 0; i < page_count; i++) {
		pml4 = map_page(pml4, (i << 12) + ARC_HHDM_VADDR, i << 12, 1);

		if (pml4 == NULL) {
			ARC_DEBUG(ERR, "Mapping failed\n")
			ARC_HANG
		}
	}

	// Map kernel
	load_elf(pml4, kernel_elf);

	return 0;
}
