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

uint64_t kernel_vaddr = 0x0;
uint64_t kernel_phys_start = 0x0;
uint32_t kernel_phys_end = 0x0;

uint64_t mem_phys_first_free = 0x0;
uint64_t size_phys_first_free = 0x0;
uint32_t bootstrap_start = 0x0;
uint64_t memsize = 0;
extern uint8_t __BOOTSTRAP_END__; // Address should be aligned to 0x1000

// Temp
int framebuffer_width = 0;
int framebuffer_height = 0;
uint64_t *hhdm_pml4 = NULL;

struct multiboot_tag_framebuffer *framebuffer_tag = NULL;
extern struct ARC_BootMeta _boot_meta;

const char *mem_types[] = {
	[MULTIBOOT_MEMORY_AVAILABLE] = "Available",
	[MULTIBOOT_MEMORY_ACPI_RECLAIMABLE] = "ACPI Reclaimable",
	[MULTIBOOT_MEMORY_BADRAM] = "Bad",
	[MULTIBOOT_MEMORY_NVS] = "NVS",
	[MULTIBOOT_MEMORY_RESERVED] = "Reserved",
};

void cpu_checks() {
	// TODO: Check if CPUID is supported
	// TODO: Check if extended functions are supported

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
	struct multiboot_tag *current_tag = (struct multiboot_tag *)(boot_info);
	struct multiboot_tag *end = (struct multiboot_tag *)(current_tag + current_tag->type);
	struct multiboot_tag_mmap *mmap = NULL;

	current_tag += 8;

	while (current_tag < end && current_tag->type != 0) {
		switch (current_tag->type) {
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			printf("Framebuffer\n");
			break;
		}

		case MULTIBOOT_TAG_TYPE_MMAP: {
			struct multiboot_tag_mmap *data = (struct multiboot_tag_mmap *)current_tag;
			mmap = data;

			break;
		}
		}

		current_tag = (struct multiboot_tag *)((uintptr_t)current_tag + ALIGN(current_tag->size, 8));
	}

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

	// Assert kernel was found
	ASSERT(kernel_phys_start != 0)
	ASSERT(kernel_phys_end != 0)

	// Assert we have some free memory to map
	ASSERT(mem_phys_first_free != 0)
	ASSERT(size_phys_first_free != 0)

	framebuffer_width = framebuffer_tag->common.framebuffer_width;
	framebuffer_height = framebuffer_tag->common.framebuffer_height;

	printf("%"PRId32"x%"PRId32"x%"PRId32" 0x%"PRIX64"(%d)\n", framebuffer_width, framebuffer_height, framebuffer_tag->common.framebuffer_bpp, framebuffer_tag->common.framebuffer_addr, framebuffer_tag->common.framebuffer_type);
	printf("All is well, kernel module is located at 0x%X.\n", (uint32_t)kernel_phys_start);

	memset(hhdm_pml4, 0, 0x1000);

	hhdm_pml4 = (uint64_t *)alloc();
	uint64_t page_count = (uint64_t)(memsize >> 12) + 1;
	uint64_t hhdm_pml1_count = (uint64_t)(page_count >> 9) + 1;
	uint64_t hhdm_pml2_count = (uint64_t)(hhdm_pml1_count >> 9) + 1;
	uint64_t hhdm_pml3_count = (uint64_t)(hhdm_pml2_count >> 9) + 1;

	uint64_t hhdm_pml4_ei = ((ARC_HHDM_VADDR >> 39) & 0x1FF);
	uint64_t hhdm_pml3_ei = ((ARC_HHDM_VADDR >> 30) & 0x1FF);
	uint64_t hhdm_pml2_ei = ((ARC_HHDM_VADDR >> 21) & 0x1FF);

	printf("Need %"PRId64" page table(s), %"PRId64" page directory(s), and %"PRId64" page directory pointer(s) for the HHDM\n", hhdm_pml1_count, hhdm_pml2_count, hhdm_pml3_count);
	printf("HHDM offset 0x%"PRIX64" (PML4[%lu]PML3[%lu]PML2[%lu])\n", ARC_HHDM_VADDR, hhdm_pml4_ei, hhdm_pml3_ei, hhdm_pml2_ei);

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
				// TODO: Account for the original offset in hhdm_pml2_ei
				pd_table[pt] = ((uintptr_t)pt_table) | 3;
			}

			// Change count
			hhdm_pml1_count -= pt_count;

			// Link PD into PDP table
			// TODO: Account for the original offset in hhdm_pml3_ei
			pdp_table[pd] = ((uintptr_t)pd_table) | 3;
		}

		// Change count
		hhdm_pml2_count -= pd_count;

		// Link PDP into PML4
		hhdm_pml4[pdp + hhdm_pml4_ei] = ((uintptr_t)pdp_table) | 3;
	}

	uint64_t *bpml3 = (uint64_t *)alloc();
	uint64_t *bpml2 = (uint64_t *)alloc();
	uint64_t *bpml1 = (uint64_t *)alloc();

	hhdm_pml4[0] = ((uintptr_t)bpml3) | 3;
	bpml3[0] = ((uintptr_t)bpml2) | 3;
	bpml2[0] = ((uintptr_t)bpml1) | 3;

	for (int i = 0; i < 512; i++) {
		bpml1[i] = (i << 12) | 3;
	}

	kernel_vaddr = load_elf(hhdm_pml4, kernel_phys_start);

	printf("HHDM PML4 Located at: 0x%"PRIX64"\n", (uint64_t)hhdm_pml4);

	for (int i = 0; i < framebuffer_height; i++) {
		for (int j = 0; j < framebuffer_width; j++) {
			*((uint32_t *)framebuffer_tag->common.framebuffer_addr + (i * framebuffer_width) + j) = 0x0043210FF;
		}
	}

	_boot_meta.mb2i = (uint32_t)boot_info;
	_boot_meta.first_free = (uint32_t)alloc();

	return (uint32_t)hhdm_pml4;
}
