#include <stdint.h>
#include <stddef.h>

struct multiboot_info {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
}__attribute__((packed));

int helper(struct multiboot_info *boot_info) {
	*((uint8_t *)0xB8000) = 'A' + boot_info->mods_count;

	return 0;
}