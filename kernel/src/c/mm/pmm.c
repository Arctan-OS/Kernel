#include <mm/pmm.h>
#include <global.h>
#include <temp/interface.h>

uint8_t *pmm_bmp = (uint8_t *)&__KERNEL_END__;

void bmp_set_range(uintptr_t base, size_t size) {
	uint64_t start_byte = (base /0x1000 / 8);
	uint8_t start_bit = ((base / 0x1000) % 8);

	uint64_t stop_byte = ((base + size) / 0x1000 / 8);
	uint8_t stop_bit = (((base + size) / 0x1000) % 8);

	for (uint64_t byte = start_byte; byte < stop_byte; byte++) {
		pmm_bmp[byte] = 0xFF;

		// if (byte == stop_byte - 1)
	// 		for (uint8_t i = 7; i > stop_bit; i--)
	// 			pmm_bmp[byte] &= ~(1 << i);
	}
}

int initialize_pmm(struct multiboot_tag_mmap *mmap) {
	// Create a bitmap structure at the end of the
	// kernel for managing physical memory.

	int entry_count = (mmap->size - ALIGN(sizeof(struct multiboot_tag_mmap), 8)) / sizeof(struct multiboot_mmap_entry);
	size_t total_mem_size = 0;

	for (int i = 0; i < entry_count; i++) {
		bmp_set_range(mmap->entries[i].addr, mmap->entries[i].len);
		printf("MMAP Interpreted entry %d(%d): 0x%8X, 0x%8X B\n", i, mmap->entries[i].type, mmap->entries[i].addr, mmap->entries[i].len);
		if (mmap->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE)
			total_mem_size += mmap->entries[i].len;
	}

	printf("%X\n", (total_mem_size / 0x1000) / 8);
	

	return 0;
}
