#include <stdint.h>
#include <stddef.h>
#include "include/multiboot2.h"
#include "include/interface.h"
#include "include/global.h"

uint8_t *tags = NULL;
uint8_t *tags_end = NULL;
uint32_t cur_tag_sz = 0;

uint32_t get_tag() {
	// We reached the end, return -1
	if (tags >= tags_end || *tags == MULTIBOOT_TAG_TYPE_END)
		return -1;

	cur_tag_sz = ALIGN(*(uint32_t *)(tags + 4), 8); 

	// Return tag type
	return *(uint32_t*)(tags);
}

int helper(uint8_t *boot_info, uint32_t magic) {
	if (magic != 0x36D76289)
		return 1;

	uint32_t total_size = *(uint32_t *)(boot_info);
	tags_end = boot_info + total_size;
	tags = boot_info + 8;

	int tag = 0;
	while ((tag = get_tag()) > 0) {
		putn(tag, 10);
		putc(' ');
		putn(cur_tag_sz, 10);
		putc('\n');

		if (tag == MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME) {
			puts(tags + 8);
			putc('\n');
		}

		tags += cur_tag_sz;
	}

	return 0;
}