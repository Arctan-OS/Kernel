#include <mm/pmm.h>
#include <global.h>
#include <temp/interface.h>

int mmap_entry_count = 0;
struct arctan_mmap_entry {
	uint64_t floor;
	uint64_t ciel;
	uint64_t bit_rep_floor;
	uint64_t bit_rep_ciel;
};

struct arctan_mmap_entry *standard_table = NULL;
uint8_t *pmm_bmp = NULL;

uint64_t addr2bit(uint64_t addr) {
	for (int i = 0; i < mmap_entry_count; i++) {
		printf("ADDR: %X, FLOOR: %X, CIEL: %X\n", addr, standard_table[i].floor, standard_table[i].ciel);

		if (addr >= standard_table[i].floor && addr <= standard_table[i].ciel) {
			return ((addr - standard_table[i].floor) >> 12) + standard_table[i].bit_rep_floor;
		}
	}

	return DEAD_64;
}

uint64_t bit2addr(uint64_t bit) {
	int i = 0;
	for (; i < mmap_entry_count; i++)
		if (bit >= standard_table[i].bit_rep_ciel)
			bit -= standard_table[i].bit_rep_ciel;

	return (bit << 12) + standard_table[i].floor;
}

int initialize_pmm(struct multiboot_tag_mmap *mmap) {
	standard_table = (struct arctan_mmap_entry *)&__KERNEL_END__;

	mmap_entry_count = (mmap->size - ALIGN(sizeof(struct multiboot_tag_mmap), 8)) / sizeof(struct multiboot_mmap_entry);
	pmm_bmp = (uint8_t *)&__KERNEL_END__ + (mmap_entry_count * sizeof(struct arctan_mmap_entry));

	size_t total_mem_size = 0;

	uint64_t cur_rep_bit = 0;
	for (int i = 0; i < mmap_entry_count; i++) {
		if (mmap->entries[i].type != MULTIBOOT_MEMORY_AVAILABLE)
			continue;

		total_mem_size += mmap->entries[i].len;
		standard_table[i].floor = mmap->entries[i].addr;
		standard_table[i].ciel = mmap->entries[i].addr + mmap->entries[i].len;
		standard_table[i].bit_rep_floor = cur_rep_bit;
		cur_rep_bit += (mmap->entries[i].len) >> 12;
		standard_table[i].bit_rep_ciel = cur_rep_bit - 1;
	}

	printf("Need %d bytes represent 0x%X bytes of free RAM, BMP is at 0x%X\n", (total_mem_size / PAGE_SIZE) / 8, total_mem_size, pmm_bmp);
	
	// "memset(pmm_bmp, 0, bmp_size)"
	for (size_t i = 0; i < (total_mem_size / PAGE_SIZE) / 8; i++) {
		pmm_bmp[i] = 0x00;
	}

	printf("%X: %X\n", addr2bit(0x0), bit2addr(0));
	printf("%X: %X\n", addr2bit(0x1FF00FF00), bit2addr(addr2bit(0x1FF00FF00)));
	printf("%X: %X\n", addr2bit(0xABAB), bit2addr(addr2bit(0xABAB)));

	return 0;
}
