#include <stdint.h>
#include <stddef.h>

struct multiboot2_info {

}__attribute__((packed));

int helper(struct multiboot2_info *boot_info, uint32_t magic) {
	if (magic != 0x36D76289)
		return 1;

	// *((uint8_t *)0xB8000) = 'A' + boot_info->mods_count;

	return 0;
}