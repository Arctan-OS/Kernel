#include <stdint.h>
#include "include/interface/printf.h"
#include "include/arch/x86/idt.h"
#include "include/arch/x86/gdt.h"

int helper(void *mbi, uint32_t signature) {
	printf("Huh?\n");

	install_gdt();
	install_idt();

	__asm__("int 3");

	return 0;
}
