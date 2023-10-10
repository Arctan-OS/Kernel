#include <mm/pmm.h>
#include <global.h>
#include <temp/interface.h>

int mmap_entry_count = 0;
struct arctan_mmap_entry {
	uint64_t floor;
	uint64_t ciel;
	uint64_t bit_rep_floor;
	uint64_t bit_rep_ciel;
	uint8_t type;
};

struct arctan_mmap_entry *standard_table = NULL;
uint8_t *pmm_bmp = NULL;

uint64_t addr2bit(uint64_t addr) {
	for (int i = 0; i < mmap_entry_count; i++) {
		if (addr >= standard_table[i].floor && addr < standard_table[i].ciel && standard_table[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
			return ((addr - standard_table[i].floor) >> 12) + standard_table[i].bit_rep_floor;
		}
	}

	return DEAD_64;
}

uint64_t bit2addr(uint64_t bit) {
	for (int i = 0; i < mmap_entry_count; i++) {
		if (bit >= standard_table[i].bit_rep_floor && bit < standard_table[i].bit_rep_ciel && standard_table[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
			return ((bit - standard_table[i].bit_rep_floor) << 12) + standard_table[i].floor;
		}
	}

	return DEAD_64;
}

int initialize_pmm(struct multiboot_tag_mmap *mmap) {
	standard_table = (struct arctan_mmap_entry *)&__KERNEL_END__;

	mmap_entry_count = (mmap->size - ALIGN(sizeof(struct multiboot_tag_mmap), 8)) / sizeof(struct multiboot_mmap_entry);
	pmm_bmp = (uint8_t *)&__KERNEL_END__ + (mmap_entry_count * sizeof(struct arctan_mmap_entry));

	size_t total_mem_size = 0;
	size_t free_ram_size = 0;

	uint64_t cur_rep_bit = 0;
	for (int i = 0; i < mmap_entry_count; i++) {
		if (mmap->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
			free_ram_size += mmap->entries[i].len;

			standard_table[i].bit_rep_floor = cur_rep_bit;
			cur_rep_bit += (mmap->entries[i].len) >> 12;
			standard_table[i].bit_rep_ciel = cur_rep_bit;
		}
			
		standard_table[i].floor = mmap->entries[i].addr;
		standard_table[i].ciel = mmap->entries[i].addr + mmap->entries[i].len;
		standard_table[i].type = mmap->entries[i].type;

		total_mem_size += mmap->entries[i].len;
	}

	printf("Need %d bytes represent 0x%X bytes of free RAM, BMP is at 0x%X\n", (free_ram_size / PAGE_SIZE) / 8, free_ram_size, pmm_bmp);
	printf("%d bytes of physical RAM present in memory map.\n", total_mem_size);
	
	// "memset(pmm_bmp, 0, bmp_size)"
	for (size_t i = 0; i < (free_ram_size / PAGE_SIZE) / 8; i++) {
		pmm_bmp[i] = 0x00;
	}

	printf("%X: %X\n", addr2bit(0x0), bit2addr(0));
	printf("%X: %X\n", addr2bit(0x100000000), bit2addr(addr2bit(0x100000000)));
	printf("%X: %X\n", addr2bit(0xABAB), bit2addr(addr2bit(0xABAB)));

	return 0;
}
