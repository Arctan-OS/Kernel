#include <stdint.h>
#include <stddef.h>

int kernel_main() {
	// Do some stuff here
	*((uint8_t *)0xB8000) = 'B';

	while (1);

	return 0;
}