#include <stdint.h>
#include "include/global.h"
#include "include/interface/printf.h"
#include "include/arch/x86/idt.h"
#include "include/arch/x86/gdt.h"

int helper(void *mbi, uint32_t signature) {
	ARC_DEBUG(INFO, "Loaded\n");

	install_gdt();
	install_idt();

	return 0;
}
