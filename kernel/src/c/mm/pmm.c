#include <mm/pmm.h>
#include <global.h>
#include <temp/interface.h>

uint8_t *pmm_bmp = (uint8_t *)&__KERNEL_END__;

// void bmp_set_range(uintptr_t base, size_t size) {
// 	uint64_t start_byte = (base /0x1000 / 8);
// 	uint8_t start_bit = ((base / 0x1000) % 8);

// 	uint64_t stop_byte = ((base + size) / 0x1000 / 8);
// 	uint8_t stop_bit = (((base + size) / 0x1000) % 8);

// 	for (uint64_t byte = start_byte; byte < stop_byte; byte++) {
// 		pmm_bmp[byte] = 0xFF;

// 		if (byte == stop_byte - 1)
// 			for (uint8_t i = 7; i > stop_bit; i--)
// 				pmm_bmp[byte] &= ~(1 << i);
// 	}
// }

int initialize_pmm(struct multiboot_tag_mmap *mmap) {
	// Create a bitmap structure at the end of the
	// kernel for managing physical memory.

	int entry_count = (mmap->size - ALIGN(sizeof(struct multiboot_tag_mmap), 8)) / sizeof(struct multiboot_mmap_entry);
	size_t total_mem_size = 0;

	for (int i = 0; i < entry_count; i++)
		total_mem_size += mmap->entries[i].len;

	printf("Need %d bytes represent %d bytes of free RAM, BMP is at 0x%X\n", (total_mem_size / 0x1000) / 8, total_mem_size, pmm_bmp);
	
	// "memset(pmm_bmp, 0, bmp_size)"
	for (size_t i = 0; i < (total_mem_size / 0x1000) / 8; i++)
		pmm_bmp[i] = 0x00;

	int cur_page = 0;
	for (int i = 0; i < entry_count; i++) {
		printf("MMAP Interpreted entry %d(%d): 0x%8X, 0x%8X B\n", i, mmap->entries[i].type, mmap->entries[i].addr, mmap->entries[i].len);

		int page_count = ALIGN(mmap->entries[i].len / 0x1000, 0x1000);
		
		// Memory is not available, set bits
		if (mmap->entries[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
			pmm_bmp[page_count] |= 0xFF & (0xFF << ((cur_page + page_count) % 8));

			if (page_count == 1)
				goto next_page;

			pmm_bmp[((cur_page + page_count - 1) >> 3)] = 0xFF & ~(0xFF << ((cur_page + page_count) % 8));

			if (page_count == 2)
				goto next_page;

			for (int p = 1; p < page_count; p++) {
				pmm_bmp[cur_page + p] = 0xFF;
			}
		}

		next_page:;
		cur_page += page_count;
	}

	return 0;
}
