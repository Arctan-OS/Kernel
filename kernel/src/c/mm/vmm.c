#include <mm/vmm.h>

#define VMM_RANGE_TABLE_MID (VMM_RANGE_TABLE_MAX / 2)

#define VMM_RANGE_FLAG_USED (1 << 0)

struct vmm_range {
	uint64_t base;
	size_t length;
	uint8_t flags;
};

static struct vmm_range range_table[VMM_RANGE_TABLE_MAX];
int allocations = 0;

void *vmm_allocate(size_t size) {
	int i = 0;
	uint64_t free_space = 0;

	for (; i < (allocations / 2) + (allocations % 2); i++) {
		struct vmm_range range = range_table[VMM_RANGE_TABLE_MID + i];

check_hemisphere:
		if ((range.flags & VMM_RANGE_FLAG_USED) == 0 && range.length >= size) {
			free_space = range.base;
			range.flags |= VMM_RANGE_FLAG_USED;

			break;
		}

		// Reached a brand new, untouched entry
		// Check complementary ranges for free
		// space.
		if (range.length == 0) {
			free_space = 0x0;
		}

		// Keep track of free space.
		

		// Flip i around and check the other
		// hemisphere.
		i *= -1;

		if (i < 0) {
			goto check_hemisphere;
		}
	}

	if (allocations == 0) {
		range_table[VMM_RANGE_TABLE_MID].base = free_space;
		range_table[VMM_RANGE_TABLE_MID].length = ALIGN(size, PAGE_SIZE);
	}

	if ((allocations % 2) == 1) {
		// Insert on the left hand side
	} else {
		// Insert on the right hand side
	}

	allocations++;

	return NULL;
}

void *vmm_use(void *address, size_t size) {
	int i = 0;

	for (; i < allocations; i++) {
		struct vmm_range range = range_table[VMM_RANGE_TABLE_MID + i];
		if (range.length == 0) {
			break;
		}
	}

	if (allocations == 0) {
		range_table[VMM_RANGE_TABLE_MID].base = (uintptr_t)address;
		range_table[VMM_RANGE_TABLE_MID].length = ALIGN(size, PAGE_SIZE);

		return address;
	}

	if ((allocations % 2) == 1) {
		// Insert on the left hand side

	} else {
		// Insert on the right hand side
	}

	allocations++;

	return address;
}

void *vmm_free(void *address, size_t size) {


	return NULL;
}

void initialize_vmm() {



}
