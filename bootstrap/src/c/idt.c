#include "include/idt.h"
#include "include/global.h"
#include "include/interface.h"

struct idt_desc {
	uint16_t limit;
	uint32_t base;
}__attribute__((packed));
struct idt_desc idtr;

struct idt_entry {
	uint16_t offset1;
	uint16_t segment;
	uint8_t zero;
	uint8_t attrs;
	uint16_t offset2;
}__attribute__((packed));
struct idt_entry idt_entries[256];

void install_idt_gate(int i, uint32_t offset, uint16_t segment, uint8_t attrs) {
	idt_entries[i].offset1 = offset & 0xFFFF;
	idt_entries[i].offset2 = (offset >> 16) & 0xFFFF;
	idt_entries[i].segment = segment;
	idt_entries[i].attrs = attrs;
	idt_entries[i].zero = 0;
}

void interrupt_junction() {
	printf("Interrupt");
	for (;;);
}

extern void _install_idt();
extern void _idt_stub_0_();

void install_idt() {
	install_idt_gate(0, (uintptr_t)_idt_stub_0_, 0x08, 0b1000110);

	idtr.limit = sizeof(idt_entries) * 8 - 1;
	idtr.base = (uintptr_t)&idt_entries;

	// Initialize the PIC
	outb(0x20, 0x11);
	outb(0x80, 0x00);
	outb(0xA0, 0x11);
	outb(0x80, 0x00);

	outb(0x21, 0x20);
	outb(0x80, 0x00);
	outb(0xA1, 0x28);
	outb(0x80, 0x00);

	outb(0x21, 0x04);
	outb(0x80, 0x00);
	outb(0xA1, 0x02);
	outb(0x80, 0x00);

	outb(0x21, 0x01);
	outb(0x80, 0x00);
	outb(0xA1, 0x01);
	outb(0x80, 0x00);

	// Mask all IRQs
	outb(0x21, 0xFF);
	outb(0xA1, 0xFF);

	_install_idt();

	printf("Installed IDT\n");
}
