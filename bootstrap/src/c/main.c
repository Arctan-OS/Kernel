#include "include/multiboot2.h"
#include "include/interface.h"
#include "include/global.h"

uint8_t *tags = NULL;
uint8_t *tags_end = NULL;
uint32_t cur_tag_sz = 0;

const char *kernel_module_name = "arctan-module.kernel.efi";

uint32_t kernel_phys_start = 0x0;
uint32_t kernel_phys_end = 0x0;
uint64_t mem_phys_first_free = 0x0;
uint64_t size_phys_first_free = 0x0;

const char *mem_types[] = {
	[MULTIBOOT_MEMORY_AVAILABLE] = "Available",
	[MULTIBOOT_MEMORY_ACPI_RECLAIMABLE] = "ACPI Reclaimable",
	[MULTIBOOT_MEMORY_BADRAM] = "Bad",
	[MULTIBOOT_MEMORY_NVS] = "NVS",
	[MULTIBOOT_MEMORY_RESERVED] = "Reserved",
};

// Paging tables
// These should be temporary, to jump to 64-bit mode
uint64_t pml4[512] __attribute__((aligned(0x1000))); // PML4 Entries
uint64_t pml3[512] __attribute__((aligned(0x1000))); // Page Directory Pointer Entries
uint64_t pml2[512] __attribute__((aligned(0x1000))); // Page Directory Entries
uint64_t pml1[512] __attribute__((aligned(0x1000))); // Page Table Entries

int helper(uint8_t *boot_info, uint32_t magic) {
	if (magic != 0x36D76289)
		return 1;

	uint32_t total_size = *(uint32_t *)(boot_info);
	tags_end = boot_info + total_size;
	tags = boot_info + 8;


	int tag = 0;
	do {
		tag = *(uint32_t*)(tags);
		cur_tag_sz = ALIGN(*(uint32_t *)(tags + 4), 8); 

		switch (tag) {
		case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
			printf("Bootloader: %s\n", tags + 8);
			
			break;
		}

		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
			struct multiboot_tag_basic_meminfo *info = (struct multiboot_tag_basic_meminfo *)tags;
			
			printf("Basic Memory Info:\n	Lower: 0x%X KiB\n	Upper: 0x%X KiB\n", info->mem_lower, info->mem_upper);

			break;
		}

		case MULTIBOOT_TAG_TYPE_MODULE: {
			struct multiboot_tag_module *info = (struct multiboot_tag_module *)tags;

			printf("Module: \"%s\" (0x%X -> 0x%X)\n", info->cmdline, info->mod_start, info->mod_end);

			if (strcmp(info->cmdline, kernel_module_name) == 0) {
				printf("! FOUND KERNEL !\n");

				kernel_phys_start = info->mod_start;
				kernel_phys_end = info->mod_end;
			}

			break;
		}

		// SAVE THIS STRUCTURE
		// PASS IT TO 64-BIT KERNEL
		case MULTIBOOT_TAG_TYPE_MMAP: {
			struct multiboot_tag_mmap *info = (struct multiboot_tag_mmap *)tags;

			printf("Detailed Memory Map (Version: %d):\n", info->entry_version);

			int i = 1;
			for (uint8_t *entry_base = tags + 16; entry_base < tags + cur_tag_sz; entry_base += info->entry_size, i++) {
				struct multiboot_mmap_entry *entry = (struct multiboot_mmap_entry *)entry_base;

				if (entry->type == MULTIBOOT_MEMORY_AVAILABLE && entry->len > size_phys_first_free) {
					mem_phys_first_free = entry->addr;
					size_phys_first_free = entry->len;
				}


				printf("\tEntry %d: @ 0x%8X, 0x%8X B, Type: %s (%d)\n", i, (uint32_t)entry->addr, (uint32_t)entry->len, mem_types[entry->type], entry->type);
			}

			break;
		}
		}

		tags += cur_tag_sz;
	} while (tag);

	ASSERT(kernel_phys_start != 0)
	ASSERT(kernel_phys_end != 0)

	ASSERT(mem_phys_first_free != 0)
	ASSERT(size_phys_first_free != 0)

	printf("All is well, kernel module is located at 0x%8X.\nGoing to poke into free RAM at 0x%8X.\n", (uint32_t)kernel_phys_start, (uint32_t)mem_phys_first_free);


	pml4[(KMAP_ADDR >> 39) & 0xFF] = (uint64_t)&pml3 | 3; // Address of next entry | RW | P 
	pml3[(KMAP_ADDR >> 30) & 0xFF] = (uint64_t)&pml2 | 3; // Address of next entry | RW | P
	pml2[(KMAP_ADDR >> 21) & 0xFF] = (uint64_t)&pml2 | 3; // Address of next entry | RW | P

	if ((kernel_phys_start >= mem_phys_first_free) && ((kernel_phys_end - kernel_phys_start) < size_phys_first_free)) {
		// Kernel is located in our desired free memory
		uint64_t phys_addr = kernel_phys_start;
		for (int i = 0; i < 512; i++) {
			pml1[i] = phys_addr | 3; // Address | RW | P
			phys_addr += 0x1000; // Goto next page
		}

		printf("Ideal conditions met, kernel mapped to %4X%4X. %d bytes available\n", (uint32_t)(KMAP_ADDR >> 32), (uint32_t)KMAP_ADDR, 512 * 0x1000);
	} else {
		// Kernel is not located in our desired free memory

		printf("Guh\n");
	}

	return 0;
}