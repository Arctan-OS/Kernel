#include "include/multiboot2.h"
#include "include/interface.h"
#include "include/global.h"
#include "include/gdt.h"
#include "include/elf.h"
#include "include/alloc.h"

#include <cpuid.h>
#include <stdint.h>

uint8_t *tags = NULL;
uint8_t *tags_end = NULL;
uint32_t cur_tag_sz = 0;

const char *kernel_module_name = "arctan-module.kernel.efi";

uint32_t kernel_phys_start = 0x0;
uint32_t kernel_phys_end = 0x0;
uint64_t *kernel_info = NULL;

uint64_t mem_phys_first_free = 0x0;
uint64_t size_phys_first_free = 0x0;
uint32_t bootstrap_start = 0x0;
size_t memsize = 0;
extern uint8_t __BOOTSTRAP_END__; // Address should be aligned to 0x1000

// Temp
int framebuffer_width = 0;
int framebuffer_height = 0;
uint64_t kernel_vaddr = 0;
uint64_t *hhdm_pml4 = NULL;

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
		printf(" CPU not up to scratch! 0x%X 0x%X 0x%X 0x%X (0x80000001)\n", __eax, __ebx, __ecx, __edx);
		__asm__("hlt");
	}

	__cpuid(1, __eax, __ebx, __ecx, __edx);

	// Check for PAE
	if (((__edx >> 6) & 1) == 0) {
		printf(" CPU not up to scratch! 0x%X 0x%X 0x%X 0x%X (0x01)\n", __eax, __ebx, __ecx, __edx);
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

				kernel_info = load_elf(info->mod_start);
				printf("%s\n", ((kernel_info[0] != 0) ? "Parsed Kernel ELF!" : "Failed to parse Kernel ELF"));
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

				printf("\tEntry %d: @ 0x%8X%8X, 0x%8X%8X B, Type: %s (%d)\n", i, (uint32_t)(entry->addr >> 32), (uint32_t)entry->addr, (uint32_t)(entry->len >> 32), (uint32_t)entry->len, mem_types[entry->type], entry->type);
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
			if (framebuffer_tag == NULL) {
				framebuffer_tag = (struct multiboot_tag_framebuffer *)tags;
				tags = boot_info + 8;

				continue;
			}
		}
		}

		tags += cur_tag_sz;
	} while (tag);

	init_allocator(mmap->entries, (mmap->size - 8) / mmap->entry_size, kernel_phys_end);
}

int helper(uint8_t *boot_info, uint32_t magic) {
	if (magic != 0x36D76289) {
		return 1;
	}

	cpu_checks();
	install_gdt();
	read_tags(boot_info);
	
	// Assert the kernel exists (should not be located at 0x0000)
	ASSERT(kernel_phys_start != 0)
	ASSERT((kernel_phys_start & 0xFFF) == 0) // Ensure kernel is page aligned
	ASSERT(kernel_phys_end != 0)

	// Assert we have some free memory to map
	ASSERT(mem_phys_first_free != 0)
	ASSERT(size_phys_first_free != 0)

	printf("%dx%dx%d %8X%8X(%d)\n", framebuffer_tag->common.framebuffer_width, framebuffer_tag->common.framebuffer_height, framebuffer_tag->common.framebuffer_bpp, (uint32_t)(framebuffer_tag->common.framebuffer_addr >> 32), (uint32_t)(framebuffer_tag->common.framebuffer_addr), 0, framebuffer_tag->common.framebuffer_type);

	printf("All is well, kernel module is located at 0x%8X.\nGoing to poke into free RAM at 0x%8X.\n", (uint32_t)kernel_phys_start);

	framebuffer_width = framebuffer_tag->common.framebuffer_width;
	framebuffer_height = framebuffer_tag->common.framebuffer_height;
	kernel_vaddr = kernel_info[1];

	size_t page_count = memsize >> 12;
	size_t hhdm_pml1_count = page_count / 512;

	uint64_t *pml1_base = NULL;

	for (size_t i = 0; i < hhdm_pml1_count; i++) {
		uint64_t *pml1 = (uint64_t *)alloc();

		if (pml1_base == NULL) {
			pml1_base = pml1;
		}

		for (int j = 0; j < 512; j++) {
			pml1[j] = ((i * 512 + j) << 12) | 3;
		}
	}

	size_t hhdm_pml2_count = hhdm_pml1_count / 512;

	uint64_t *pml2_base = NULL;

	for (size_t i = 0; i < hhdm_pml2_count; i++) {
		uint64_t *pml2 = (uint64_t *)alloc();

		if (pml2_base == NULL) {
			pml2_base = pml2;
		}

		for (int j = 0; j < 512; j++) {
			pml2[j] = (((uintptr_t)pml1_base + ((i * 512 + j) << 12))) | 3;
		}
	}

	size_t hhdm_pml3_count = hhdm_pml2_count / 512;

	hhdm_pml4 = (uint64_t *)alloc();
	uint64_t *pml3_base = NULL;

	for (size_t i = 0; i < hhdm_pml3_count; i++) {
		uint64_t *pml3 = (uint64_t *)alloc();
		hhdm_pml4[i] = (uintptr_t)pml3 | 3;

		if (pml3_base == NULL) {
			pml3_base = pml3;
		}

		for (int j = 0; j < 512; j++) {
			pml3[j] = (((uintptr_t)pml2_base + ((i * 512 + j) << 12))) | 3;
		}
	}

	size_t kernel_count = ((kernel_phys_end - kernel_phys_start) >> 12);

	uint64_t *kpml1 = (uint64_t *)alloc();

	for (size_t i = 0; i < 512; i++) {
		if (i <= kernel_count) {
			kpml1[i] = (kernel_phys_start + (i << 12)) | 3;

			continue;
		}

		kpml1[i] = ((uintptr_t)alloc()) | 3;
	}

	int kpml4_ei = (kernel_info[0] >> 39) & 0x1FF;
	int kpml3_ei = (kernel_info[0] >> 30) & 0x1FF;
	int kpml2_ei = (kernel_info[0] >> 21) & 0x1FF;

	uint64_t *kpml3 = (uint64_t *)alloc();
	uint64_t *kpml2 = (uint64_t *)alloc();

	hhdm_pml4[kpml4_ei] = ((uintptr_t)kpml3) | 3;
	kpml3[kpml3_ei] = ((uintptr_t)kpml2) | 3;
	kpml2[kpml2_ei] = ((uintptr_t)kpml1) | 3;

	return 0;
}
