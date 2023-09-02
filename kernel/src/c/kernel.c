#include <stdint.h>
#include <stddef.h>

int kernel_main() {
	// Do some stuff here
	*((uint8_t *)0xB8000) = 'B';

	return 0;
}