#include <mm/pmm.h>
#include <global.h>
#include <temp/interface.h>

uint8_t *pmm_bmp = (uint8_t *)&__KERNEL_END__;

int initialize_pmm(struct multiboot_tag_mmap *mmap) {
	// Create a bitmap structure at the end of the
	// kernel for managing physical memory.

	int entry_count = (mmap->size - ALIGN(sizeof(struct multiboot_tag_mmap), 8)) / sizeof(struct multiboot_mmap_entry);
	size_t total_mem_size = 0;

	for (int i = 0; i < entry_count; i++)
		total_mem_size += mmap->entries[i].len;

	printf("Need %d bytes represent 0x%X bytes of free RAM, BMP is at 0x%X\n", (total_mem_size / PAGE_SIZE) / 8, total_mem_size, pmm_bmp);
	
	// "memset(pmm_bmp, 0, bmp_size)"
	for (size_t i = 0; i < (total_mem_size / PAGE_SIZE) / 8; i++)
		pmm_bmp[i] = 0x00;

	// ERROR:
	// The last entry in QEMU, seems to be causing a really large error.
	// This makes me think that the section of code where we calculate
	// the total size of all sections is incorrect.
	for (int i = 0; i < entry_count; i++) {
		printf("Interpreting MMAP Entry %d(%d): @ 0x%X, 0x%X bytes\n", i, mmap->entries[i].type, mmap->entries[i].addr, mmap->entries[i].len);

		if (mmap->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE)
			continue;

		int64_t section_idx = (mmap->entries[i].addr >> 12) / 8;
		int64_t section_len = (ALIGN(mmap->entries[i].len, PAGE_SIZE) / PAGE_SIZE);

		printf("\tWould set byte %d of BMP(%d, %d, %X)\n", section_idx, i, section_len, ALIGN(mmap->entries[i].len, PAGE_SIZE) / PAGE_SIZE);

		pmm_bmp[section_idx] |= (0xFF << ((mmap->entries[i].addr / PAGE_SIZE) % 8));
		section_len -= ((mmap->entries[i].addr / PAGE_SIZE) % 8) == 0 ? 8 : (mmap->entries[i].addr / PAGE_SIZE) % 8;
		section_idx++;

		if (section_len > 0) {
			while (section_len > 8) {
				if (section_len > 8) {
					printf("\tWould set byte %d of BMP(%d, %d, %X)\n", section_idx, i, section_len, ALIGN(mmap->entries[i].len, PAGE_SIZE) / PAGE_SIZE);
					
					pmm_bmp[section_idx++] |= 0xFF;

					section_len -= 8;
					continue;
				}
			}

			pmm_bmp[section_idx] |= 0xFF & ~(0xFF << ((section_len % 8) == 0 ? 8 : (section_len % 8)));
		}

		printf("\tMarked BMP bits as \"allocated\"\n");
	}

	return 0;
}
