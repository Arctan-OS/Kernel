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

#include "mm/freelist.h"
#include "multiboot2.h"
#include <mm/pmm.h>
#include <global.h>
#include <util.h>
#include <framebuffer/printf.h>
#include <inttypes.h>

struct ARC_FreelistMeta free_pages = { 0 };

int initialize_pmm(struct multiboot_tag_mmap *mmap) {
//	int entries = (mmap->size - sizeof(struct multiboot_tag_mmap)) / mmap->entry_size;

	struct ARC_FreelistMeta a = { 0 };

//	for (int i = 0; i < entries; i++) {
//		struct multiboot_mmap_entry entry = mmap->entries[i];

//		if (entry.type != MULTIBOOT_MEMORY_AVAILABLE) {
//			continue;
//		}

//		void *base = (void *)entry.addr;
//		void *ciel = (void *)(entry.addr + entry.len);

//		if (base <= (void *)arc_boot_meta->first_free && ciel > (void *)arc_boot_meta->first_free) {
//			base = (void *)arc_boot_meta->first_free;
//		}

//		if (i == 0) {
//			Arc_InitializeFreelist(base, ciel, PAGE_SIZE, &free_pages);
//		} else {
//			Arc_InitializeFreelist(base, ciel, PAGE_SIZE, &a);
//			Arc_ListLink(&free_pages, &a, &free_pages);
//		}
//	}

	return 0;
}



/*
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
size_t next_free_page = 0;
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

	size_t page_count = ALIGN(size, PAGE_SIZE) / PAGE_SIZE;
	
	size_t page_base = next_free_page;
	size_t page_top = 0;

	while (page_base + page_top < bmp_length) {
		// Check if page is allocated
		uint8_t bmp_value = (pmm_bmp[(page_base + page_top) / 8] >> ((page_base + page_top) % 8)) & 1;
		
		// If so, advance
		if (bmp_value == 1) {
			page_top = 0;
			page_base++;

			continue;
		}

		// Found free section
		if (page_top >= page_count) {
			// Mark pages as used
			for (size_t i = page_base; i < page_base + page_top; i++) {
				pmm_bmp[i / 8] |= 1 << (i % 8);
			}

			break;
		}

		// Above conditions not met, assume free page, increment pages found
		// advance
		page_top++;
	}
	
	// Could not find free section
	if (page_top < page_count) {
		return NULL;
	}

	next_free_page += page_top;
	
	return (void *)bit2addr(page_base);
}

void *pmm_free(void *address, size_t size) {
	if (size == 0) {
		return NULL;
	}
	
	size_t page_index = addr2bit((uint64_t)address);
	size_t page_count = ALIGN(size, PAGE_SIZE) / PAGE_SIZE;

	for (size_t i = page_index; i < page_index + page_count; i++) {
		pmm_bmp[i / 8] &= ~(1 << (i % 8));
	}

	next_free_page = page_index;

	return address;
}

int initialize_pmm(struct multiboot_tag_mmap *mmap) {
	standard_table = alloc_pages(kernel_heap_pool, 1);

	mmap_entry_count = (mmap->size - ALIGN(sizeof(struct multiboot_tag_mmap), 8)) / sizeof(struct multiboot_mmap_entry);

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

	bmp_length = ((free_ram_size >> 12) + 1) / 8;
	pmm_bmp = (uint8_t *)alloc_pages(kernel_heap_pool, (bmp_length >> 12) + 1);

	printf("Need %d bytes represent 0x%X bytes of free RAM, BMP is at 0x%X\n", bmp_length, free_ram_size, pmm_bmp);
	printf("%d bytes of physical RAM present in memory map.\n", total_mem_size);
	
	memset(pmm_bmp, 0, bmp_length);
	
	return 0;
}
*/
