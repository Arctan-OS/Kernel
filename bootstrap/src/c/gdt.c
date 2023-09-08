#include "include/gdt.h"
#include "include/global.h"
#include "include/interface.h"

struct gdt_header {
	uint16_t size;
	uint32_t base;
}__attribute__((packed));

struct gdt_entry {
	uint16_t limit;
	uint16_t base1;
	uint8_t base2;
	uint8_t access;
	uint8_t flags_limit;
	uint8_t base3;
}__attribute__((packed));

struct gdt_header gdtr;
struct gdt_entry gdt_entries[16];

void set_gdt_gate(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	gdt_entries[i].base1 = (base      ) & 0xFFFF;
	gdt_entries[i].base2 = (base >> 16) & 0xFF;
	gdt_entries[i].base3 = (base >> 24) & 0xFF;

	gdt_entries[i].access = access;
	
	gdt_entries[i].limit = (limit) & 0xFFFF;
	gdt_entries[i].flags_limit = (flags & 0x0F) << 4 | ((limit >> 16) & 0x0F);
}

extern void _install_gdt();
void install_gdt() {
	set_gdt_gate(0, 0, 0, 0, 0);
	set_gdt_gate(1, 0, 0xFFFFFFFF, 0b10011010, 0b00001100); // Kernel Code
	set_gdt_gate(2, 0, 0xFFFFFFFF, 0b10010110, 0b00001100); // Kernel Data

	gdtr.size = sizeof(gdt_entries) * 8 - 1;
	gdtr.base = (uintptr_t)&gdt_entries;

	_install_gdt();

	printf("Installed GDT\n");
}