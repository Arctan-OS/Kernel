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

#include "include/arctan.h"
#include "include/multiboot2.h"
#include "include/interface.h"
#include "include/global.h"
#include "include/gdt.h"
#include "include/elf.h"
#include "include/alloc.h"
#include "include/idt.h"

#include <cpuid.h>
#include <stdint.h>
#include <inttypes.h>

uint8_t *tags = NULL;
uint8_t *tags_end = NULL;
uint32_t cur_tag_sz = 0;

const char *kernel_module_name = "arctan-module.kernel.efi";

uint32_t kernel_phys_start = 0x0;
uint32_t kernel_phys_end = 0x0;
uint64_t kernel_vaddr = 0x0;
uint64_t kernel_elf_off = 0x0;

uint64_t mem_phys_first_free = 0x0;
uint64_t size_phys_first_free = 0x0;
uint32_t bootstrap_start = 0x0;
uint64_t memsize = 0;
extern uint8_t __BOOTSTRAP_END__; // Address should be aligned to 0x1000

// Temp
int framebuffer_width = 0;
int framebuffer_height = 0;
uint64_t *hhdm_pml4 = NULL;
void *hhdm_pml4_end = NULL;

struct multiboot_tag_framebuffer *framebuffer_tag = NULL;

const char *mem_types[] = {
	[MULTIBOOT_MEMORY_AVAILABLE] = "Available",
	[MULTIBOOT_MEMORY_ACPI_RECLAIMABLE] = "ACPI Reclaimable",
	[MULTIBOOT_MEMORY_BADRAM] = "Bad",
	[MULTIBOOT_MEMORY_NVS] = "NVS",
	[MULTIBOOT_MEMORY_RESERVED] = "Reserved",
};

void cpu_checks() {
	// Preform checks
	uint32_t __eax, __ebx, __ecx, __edx;

	__cpuid(0x80000001, __eax, __ebx, __ecx, __edx);

	// Check for LM
	if (((__edx >> 29) & 1) == 0) {
		printf("CPU not up to scratch! 0x%X 0x%X 0x%X 0x%X (0x80000001)\n", __eax, __ebx, __ecx, __edx);
		__asm__("hlt");
	}

	__cpuid(1, __eax, __ebx, __ecx, __edx);

	// Check for PAE
	if (((__edx >> 6) & 1) == 0) {
		printf("CPU not up to scratch! 0x%X 0x%X 0x%X 0x%X (0x01)\n", __eax, __ebx, __ecx, __edx);
		__asm__("hlt");
	}
}

void read_tags(uint8_t *boot_info) {
	struct multiboot_tag_mmap *mmap = NULL;

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

			printf("Basic Memory Info:\n	Lower: 0x%X KB\n	Upper: 0x%X KB\n", info->mem_lower, info->mem_upper);

			break;
		}

		case MULTIBOOT_TAG_TYPE_MODULE: {
			struct multiboot_tag_module *info = (struct multiboot_tag_module *)tags;

			printf("Module: \"%s\" (0x%X -> 0x%X)\n", info->cmdline, info->mod_start, info->mod_end);

			if (strcmp(info->cmdline, kernel_module_name) == 0) {
				printf("! FOUND KERNEL !\n");

				kernel_phys_start = info->mod_start;
				kernel_phys_end = info->mod_end;

				uint64_t *kernel_info = load_elf(info->mod_start);
				kernel_elf_off = kernel_info[0];
				kernel_vaddr = kernel_info[1];

				printf("%s offset is %"PRIX64"\n", ((kernel_info[0] != 0) ? "Parsed Kernel ELF!" : "Failed to parse Kernel ELF"), kernel_info[0]);
			}

			break;
		}

		// SAVE THIS STRUCTURE
		// PASS IT TO 64-BIT KERNEL
		case MULTIBOOT_TAG_TYPE_MMAP: {
			mmap = (struct multiboot_tag_mmap *)tags;

			printf("Detailed Memory Map (Version: %d):\n", mmap->entry_version);

			int i = 1;
			for (uint8_t *entry_base = tags + 16; entry_base < tags + cur_tag_sz; entry_base += mmap->entry_size, i++) {
				struct multiboot_mmap_entry *entry = (struct multiboot_mmap_entry *)entry_base;

				if (entry->type == MULTIBOOT_MEMORY_AVAILABLE && entry->len > size_phys_first_free) {
					mem_phys_first_free = entry->addr;
					size_phys_first_free = entry->len;
				}

				printf("\tEntry %d: @ 0x%16"PRIX64", 0x%16"PRIX64" B, Type: %s (%d)\n", i, entry->addr, entry->len, mem_types[entry->type], entry->type);
				memsize += entry->len;
			}

			break;
		}

		case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: {
			struct multiboot_tag_load_base_addr *info = (struct multiboot_tag_load_base_addr *)tags;

			bootstrap_start = info->load_base_addr;

			printf("Entry: 0x%X, 0x%X\n", info->load_base_addr , &__BOOTSTRAP_END__);

			break;
		}

		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			//if (framebuffer_tag == NULL) {
				framebuffer_tag = (struct multiboot_tag_framebuffer *)tags;
			//	tags = boot_info + 8;
			//
			//	continue;
			//}
		}
		}

		tags += cur_tag_sz;
	} while (tag);

	init_allocator(mmap->entries, (mmap->size - 16) / mmap->entry_size, kernel_phys_end);
}

int helper(uint8_t *boot_info, uint32_t magic) {
	if (magic != 0x36D76289) {
		return 1;
	}

	cpu_checks();
	install_gdt();
	install_idt();
	read_tags(boot_info);

	// Assert the kernel exists (should not be located at 0x0000)
	ASSERT(kernel_phys_start != 0)
	ASSERT((kernel_phys_start & 0xFFF) == 0) // Ensure kernel is page aligned
	ASSERT(kernel_phys_end != 0)

	// Assert we have some free memory to map
	ASSERT(mem_phys_first_free != 0)
	ASSERT(size_phys_first_free != 0)

	framebuffer_width = framebuffer_tag->common.framebuffer_width;
	framebuffer_height = framebuffer_tag->common.framebuffer_height;

	printf("%"PRId32"x%"PRId32"x%"PRId32" 0x%"PRIX64"(%d)\n", framebuffer_width, framebuffer_height, framebuffer_tag->common.framebuffer_bpp, framebuffer_tag->common.framebuffer_addr, framebuffer_tag->common.framebuffer_type);
	printf("All is well, kernel module is located at 0x%X.\n", (uint32_t)kernel_phys_start);

	hhdm_pml4 = (uint64_t *)alloc();
	memset(hhdm_pml4, 0, 0x1000);

	uint64_t page_count = (uint64_t)(memsize >> 12) + 1;
	uint64_t hhdm_pml1_count = (uint64_t)(page_count >> 9) + 1;
	uint64_t hhdm_pml2_count = (uint64_t)(hhdm_pml1_count >> 9) + 1;
	uint64_t hhdm_pml3_count = (uint64_t)(hhdm_pml2_count >> 9) + 1;

	int hhdm_pml4_ei = (ARC_HHDM_VADDR >> 39) & 0x1FF;
	int hhdm_pml3_ei = (ARC_HHDM_VADDR >> 30) & 0x1FF;
	int hhdm_pml2_ei = (ARC_HHDM_VADDR >> 21) & 0x1FF;
	int hhdm_pml1_ei = (ARC_HHDM_VADDR >> 12) & 0x1FF;

	printf("Need %"PRId64" page table(s), %"PRId64" page directory(s), and %"PRId64" page directory pointer(s) for the HHDM\n", hhdm_pml1_count, hhdm_pml2_count, hhdm_pml3_count);
	printf("HHDM offset 0x%"PRIX64" (PML4[%d]PML3[%d]PML2[%d]PML1[%d])\n", ARC_HHDM_VADDR, hhdm_pml4_ei, hhdm_pml3_ei, hhdm_pml2_ei, hhdm_pml1_ei);

	uint64_t page_address = 0x0;
	for (uint64_t pdp = 0; pdp < hhdm_pml3_count; pdp++) {
		// Allocate table
		uint64_t *pdp_table = (uint64_t *)alloc();
		memset(pdp_table, 0, 0x1000);

		// Get limited count
		int pd_count = min(512, hhdm_pml2_count);

		for (int pd = 0; pd < pd_count; pd++) {
			// Allocate table
			uint64_t *pd_table = (uint64_t *)alloc();
			memset(pd_table, 0, 0x1000);

			// Get limited count
			int pt_count = min(512, hhdm_pml1_count);

			for (int pt = 0; pt < pt_count; pt++) {
				// ALlocate PT
				uint64_t *pt_table = (uint64_t *)alloc();
				memset(pt_table, 0, 0x1000);

				// Fill out PT
				for (int i = 0; i < 512; i++) {
					pt_table[i] = (page_address) | 3;
					page_address += 0x1000;
				}

				// Link PT into PD
				pd_table[pd > 0 ? pt : pt + hhdm_pml2_ei] = ((uintptr_t)pt_table) | 3;
			}

			// Change count
			hhdm_pml1_count -= pt_count;

			// Link PD into PDP table
			pdp_table[pdp > 0 ? pd : hhdm_pml3_ei + pd] = ((uintptr_t)pd_table) | 3;
		}

		// Change count
		hhdm_pml2_count -= pd_count;

		// Link PDP into PML4
		hhdm_pml4[hhdm_pml4_ei + pdp] = ((uintptr_t)pdp_table) | 3;
	}

	uint64_t *bpml3 = (uint64_t *)alloc();
	uint64_t *bpml2 = (uint64_t *)alloc();
	uint64_t *bpml1 = (uint64_t *)alloc();

	*bpml3 = ((uintptr_t)bpml2) | 3;
	*bpml2 = ((uintptr_t)bpml1) | 3;

	for (int i = 0; i < 512; i++) {
		*bpml1 = (i << 12) | 3;
	}

	hhdm_pml4[0] = ((uintptr_t)bpml3) | 3;

	uint64_t kernel_count = ((kernel_phys_end - kernel_phys_start - kernel_elf_off) >> 12);

	uint64_t *kpml1 = (uint64_t *)alloc();
	uint64_t *kpml1_2 = (uint64_t *)alloc();

	for (size_t i = 0; i < 512; i++) {
		if (i <= kernel_count) {
			kpml1[i] = (kernel_phys_start + kernel_elf_off + (i << 12)) | 3;

			continue;
		}

		kpml1[i] = ((uintptr_t)alloc()) | 3;
		kpml1_2[i] = ((uintptr_t)alloc()) | 3;
	}

	int kpml4_ei = (kernel_vaddr >> 39) & 0x1FF;
	int kpml3_ei = (kernel_vaddr >> 30) & 0x1FF;
	int kpml2_ei = (kernel_vaddr >> 21) & 0x1FF;

	printf("Mapping kernel to PML4[%d] PML3[%d] PML2[%d]\n", kpml4_ei, kpml3_ei, kpml2_ei);

	uint64_t *kpml3 = (uint64_t *)alloc();
	uint64_t *kpml2 = (uint64_t *)alloc();

	hhdm_pml4_end = (void *)kpml2;

	hhdm_pml4[kpml4_ei] = ((uintptr_t)kpml3) | 3;
	kpml3[kpml3_ei] = ((uintptr_t)kpml2) | 3;
	kpml2[kpml2_ei] = ((uintptr_t)kpml1) | 3;
	kpml2[kpml2_ei + 1] = ((uintptr_t)kpml1_2) | 3;

	printf("HHDM PML4 Located at: 0x%"PRIX64"\n", (uint64_t)hhdm_pml4);

	return (uint32_t)hhdm_pml4;
}
