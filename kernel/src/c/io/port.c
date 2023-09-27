#include <io/port.h>

inline void outb(uint16_t address, uint8_t value) {
	__asm__("out %0, %1" : : "d"(address), "a"(value) : );
}