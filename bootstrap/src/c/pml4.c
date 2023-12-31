#include "include/pml4.h"
#include "include/alloc.h"

// int level - 0 based level (PML4 = 3, PML3 = 2, PML2 = 1, PML1 = 0)
// Return 0: success
// Return 1: Mapping already present, did not overwrite
int map(uint64_t *table, uint64_t page_frame, uint64_t page, int overwrite, int level) {
	page &= 0xFFFFFFFFFFFF000;
	page_frame &= 0xFFFFFFFFFFFF000;

	int index = (page_frame >> (12 + 9 * level)) & 0x1FF;

	if (level == 0 && overwrite == 0 && (table[index] & 1) == 1) {
		return 1;
	}

	if (level > 0 && (table[index] & 1) == 1) {
		uint64_t *next = (uint64_t *)(table[index] & 0xFFFFFFFFF000);
		map(next, page_frame, page, overwrite, level - 1);
	} else if (level > 0) {
		uint64_t *next = (uint64_t *)alloc();
		*next |= 3;
		map(next, page_frame, page, overwrite, level - 1);
	}

	// Here, level = 0 and we are able to write
	table[index] = page_frame | 3;

	return 0;
}

// int level - 0 based level (PML4 = 3, PML3 = 2, PML2 = 1, PML1 = 0)
// Return 0: success
// Return 1: Mapping already present, did not overwrite
int map_table(uint64_t *table, uint64_t page_frame, uint64_t page, int overwrite, int level) {


	return 0;
}
