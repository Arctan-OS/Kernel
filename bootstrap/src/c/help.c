#include <stdint.h>
#include <stddef.h>

int helper(uint32_t boot_info) {
	*((uint8_t *)0xB8000) = 'A';

	return 0;
}
