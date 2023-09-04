#include <stdint.h>
#include <stddef.h>
#include "include/multiboot2.h"

uint32_t *tags = NULL;
uint32_t *tags_end = NULL;
uint32_t cur_tag_sz = 0;

int get_tag() {
	// We reached the end, return -1
	if (tags >= tags_end || *tags == MULTIBOOT_TAG_TYPE_END)
		return -1;

	
	cur_tag_sz = *(tags + 4);

	// Return tag type
	return (*tags);
}

int helper(uint32_t *boot_info, uint32_t magic) {
	if (magic != 0x36D76289)
		return 1;

	uint32_t total_size = *boot_info;
	tags_end = boot_info + total_size;
	tags = boot_info + 8;

	uint8_t *screen = (uint8_t *)0xB8000;
	int tag = 0;
	while ((tag = get_tag()) > 0) {
		*screen = 'A';
		screen += 2;

		tags += cur_tag_sz;
	}

	return 0;
}