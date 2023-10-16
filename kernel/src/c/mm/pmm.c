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

// Byte which contains the next free bit
uint64_t next_free_byte = 0;
uint8_t next_free_bit = 0;
uint64_t bmp_length = 0;

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

void *pmm_allocate(size_t size) {
	if (size == 0) {
		return NULL;
	}

	// Round up the number of pages we want to allocate
	size_t pages = ALIGN(((size + 0x1000) / 0x1000), PAGE_SIZE);
	size_t avl_bits = pages;
	uint64_t byte_offset = 0;

	while (avl_bits > 0) {
		if (next_free_byte >= bmp_length) {
			next_free_byte = 0;
			byte_offset = 0;
		}

		uint8_t cur_byte = pmm_bmp[next_free_byte + byte_offset];
		int shift = 0;

		while ((cur_byte >> shift) > 0)
			shift++;

		if (shift < 8) {
			avl_bits -= shift;
			byte_offset++;
		} else {
			next_free_byte++;
			byte_offset = 0;
			avl_bits = pages;
		}
	}

	void *base = (void *)((next_free_byte * 8 + next_free_bit) * PAGE_SIZE);

	// Mark pages number of bits as 1 at cur_byte_idx
	pmm_bmp[next_free_byte] |= 0xFF & (0xFF << next_free_bit);

	for (size_t i = 1; i < byte_offset; i++) {
		pmm_bmp[next_free_byte + i] = 0xFF;
	}

	pmm_bmp[next_free_byte + byte_offset] |= 0xFF & (0xFF >> 3);

	return base;
}

void *pmm_free(void *address, size_t size) {

	return NULL;
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

	bmp_length = (free_ram_size / PAGE_SIZE) / 8;
	printf("Need %d bytes represent 0x%X bytes of free RAM, BMP is at 0x%X\n", bmp_length, free_ram_size, pmm_bmp);
	printf("%d bytes of physical RAM present in memory map.\n", total_mem_size);
	
	// "memset(pmm_bmp, 0, bmp_size)"
	for (size_t i = 0; i < bmp_length; i++) {
		pmm_bmp[i] = 0x00;
	}

	// Causes triple fault
	printf("%X\n", pmm_allocate(0x1000));
	printf("%X\n", pmm_allocate(0x1000));

	return 0;
}
