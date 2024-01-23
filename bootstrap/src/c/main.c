#include <stdint.h>
#include <global.h>
#include <interface/printf.h>
#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>
#include <arctan.h>
#include <multiboot/mbparse.h>
#include <mm/freelist.h>
#include <mm/vmm.h>
#include <multiboot/multiboot2.h>
#include <arch/x86/cpuid.h>
#include <elf/elf.h>

struct ARC_FreelistMeta physical_mem = { 0 };
uint64_t highest_address = 0;
void *kernel_elf = NULL;
uint64_t *pml4 = NULL;
uint64_t kernel_entry = 0;

int helper(void *mbi, uint32_t signature) {
	ARC_DEBUG(INFO, "Loaded\n");

	if (signature != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printf("System was not booted using a multiboot2 bootloader, stopping.\n");
		ARC_HANG
	}

	_boot_meta.mb2i = (uintptr_t)mbi;

	check_features();

	install_gdt();
	install_idt();

	read_mb2i(mbi);

	// Identity map first 4MB
	for (int i = 0; i < 4 * 512; i++) {
		pml4 = map_page(pml4, i << 12, i << 12, 1);

		if (pml4 == NULL) {
			ARC_DEBUG(ERR, "Mapping failed\n")
			ARC_HANG
		}
	}

	// Create HHDM
	for (uint64_t i = 0; i <= highest_address; i += 0x1000) {
		pml4 = map_page(pml4, i + ARC_HHDM_VADDR, i, 1);

		if (pml4 == NULL) {
			ARC_DEBUG(ERR, "Mapping failed\n")
			ARC_HANG
		}
	}

	// Map kernel
	kernel_entry = load_elf(pml4, kernel_elf);

	_boot_meta.pmm_state = (uintptr_t)&physical_mem;

	return 0;
}
