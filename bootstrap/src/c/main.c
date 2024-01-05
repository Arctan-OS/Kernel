#include <stdint.h>
#include "include/global.h"
#include "include/interface/printf.h"
#include "include/arch/x86/idt.h"
#include "include/arch/x86/gdt.h"
#include "include/arctan.h"
#include "include/multiboot/mbparse.h"
#include "include/mm/freelist.h"
#include "include/mm/pmm.h"

struct ARC_FreelistMeta physical_mem = { 0 };

int helper(void *mbi, uint32_t signature) {
	ARC_DEBUG(INFO, "Loaded\n");

	if (signature != 0x36D76289) {
		printf("System was not booted using a multiboot2 bootloader, stopping.\n");
		ARC_HANG
	}

	install_gdt();
	install_idt();

	read_mb2i(mbi);



	return 0;
}
