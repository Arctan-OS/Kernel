#include "io/ctrl_reg.h"
#include <interrupts/idt.h>
#include <global.h>
#include <io/port.h>
#include <framebuffer/printf.h>
#include <inttypes.h>
#include <stdio.h>

struct idt_header {
	uint16_t limit;
	uint64_t base;
}__attribute__((packed));
struct idt_header idtr;

struct idt_entry {
	uint16_t offset1;
	uint16_t selector;
	uint8_t ist;
	uint8_t flags;
	uint16_t offset2;
	uint32_t offset3;
	uint32_t reserved;
}__attribute__((packed));
struct idt_entry idt_entries[256];

struct junction_args {
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t rsp;
	uint64_t code;
}__attribute__((packed));

static const char *exception_names[] __attribute__((section(".rodata")))= {
	"Division Error (#DE)",
	"Debug Exception (#DB)",
	"NMI",
	"Breakpoint (#BP)",
	"Overflow (#OF)",
	"BOUND Range Exceeded (#BR)",
	"Invalid Opcode (#UD)",
	"Device Not Available (No Math Coprocessor) (#NM)",
	"Double Fault (#DF)",
	"Coprocessor Segment Overrun (Reserved)",
	"Invalid TSS (#TS)",
	"Segment Not Present (#NP)",
	"Stack-Segment Fault (#SS)",
	"General Protection (#GP)",
	"Page Fault (#PF)",
	"Reserved",
	"x87 FPU Floating-Point Error (Math Fault) (#MF)",
	"Alignment Check (#AC)",
	"Machine Check (#MC)",
	"SIMD Floating-Point Exception (#XM)",
	"Virtualization Exception (#VE)",
	"Control Protection Exception (#CP)",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

void install_idt_gate(int i, uint64_t offset, uint16_t selector, uint8_t flags) {
	idt_entries[i].offset1 = offset & 0xFFFF;
	idt_entries[i].offset2 = (offset >> 16) & 0xFFFF;
	idt_entries[i].offset3 = (offset >> 32) & 0xFFFFFFFF;

	idt_entries[i].selector = selector;
	idt_entries[i].flags = flags;
	idt_entries[i].reserved = 0;
	idt_entries[i].ist = 0;
}

void interrupt_junction(uint64_t rsp) {
	// Cast structure pushed to stack into args
	struct junction_args *args = (struct junction_args *)(rsp);

	// Pop the interrupt args->code off the stack
	args->rsp += 8;

	// Dump registers
	printf("Received Interrupt %d, %s\n", args->code, exception_names[args->code]);
	printf("RAX: 0x%X\n", args->rax);
	printf("RBX: 0x%X\n", args->rbx);
	printf("RCX: 0x%X\n", args->rcx);
	printf("RDX: 0x%X\n", args->rdx);
	printf("RSI: 0x%X\n", args->rsi);
	printf("RDI: 0x%X\n", args->rdi);
	printf("RSP: 0x%X\n", args->rsp);
	printf("RBP: 0x%X\n", args->rbp);

	// If we enter none of the following conditions,
	uint64_t stack_elem = *(uint64_t *)args->rsp;
	// this is the return address

	// Handle error code if present
	switch (args->code) {
	case 21:
	case 17:
	case 14:
		read_cr2();
		printf("CR2: %"PRIX64"\n", *(uint64_t *)&cr2_reg);
		goto fall_through;
	case 13:
		//handle_gp(stack_elem);
		goto fall_through;
	case 12:
	case 11:
	case 10:
	case 8: {
fall_through:;
		// Pop the error code off the stack
		args->rsp += 8;
		// This should now be the return address
		stack_elem = *(uint64_t *)args->rsp;

		break;
	}
	}

	outb(0xE9, 'A' + args->code);

	for (;;);

	printf("Return address: 0x%"PRIX64"\n", stack_elem);

	// Send EOI
	if (args->code >= 8) {
		outb(0xA0, 0x20);
	}

	outb(0x20, 0x20);
}

extern void _install_idt();
extern void _idt_stub_0_();
extern void _idt_stub_1_();
extern void _idt_stub_2_();
extern void _idt_stub_3_();
extern void _idt_stub_4_();
extern void _idt_stub_5_();
extern void _idt_stub_6_();
extern void _idt_stub_7_();
extern void _idt_stub_8_();
extern void _idt_stub_9_();
extern void _idt_stub_10_();
extern void _idt_stub_11_();
extern void _idt_stub_12_();
extern void _idt_stub_13_();
extern void _idt_stub_14_();
extern void _idt_stub_15_();
extern void _idt_stub_16_();
extern void _idt_stub_17_();
extern void _idt_stub_18_();
extern void _idt_stub_19_();
extern void _idt_stub_20_();
extern void _idt_stub_21_();
extern void _idt_stub_22_();
extern void _idt_stub_23_();
extern void _idt_stub_24_();
extern void _idt_stub_25_();
extern void _idt_stub_26_();
extern void _idt_stub_27_();
extern void _idt_stub_28_();
extern void _idt_stub_29_();
extern void _idt_stub_30_();
extern void _idt_stub_31_();

void init_idt() {
	install_idt_gate(0, (uintptr_t)&_idt_stub_0_, 0x18, 0x8E);
	install_idt_gate(1, (uintptr_t)&_idt_stub_1_, 0x18, 0x8E);
	install_idt_gate(2, (uintptr_t)&_idt_stub_2_, 0x18, 0x8E);
	install_idt_gate(3, (uintptr_t)&_idt_stub_3_, 0x18, 0x8E);
	install_idt_gate(4, (uintptr_t)&_idt_stub_4_, 0x18, 0x8E);
	install_idt_gate(5, (uintptr_t)&_idt_stub_5_, 0x18, 0x8E);
	install_idt_gate(6, (uintptr_t)&_idt_stub_6_, 0x18, 0x8E);
	install_idt_gate(7, (uintptr_t)&_idt_stub_7_, 0x18, 0x8E);
	install_idt_gate(8, (uintptr_t)&_idt_stub_8_, 0x18, 0x8E);
	install_idt_gate(9, (uintptr_t)&_idt_stub_9_, 0x18, 0x8E);
	install_idt_gate(10, (uintptr_t)&_idt_stub_10_, 0x18, 0x8E);
	install_idt_gate(11, (uintptr_t)&_idt_stub_11_, 0x18, 0x8E);
	install_idt_gate(12, (uintptr_t)&_idt_stub_12_, 0x18, 0x8E);
	install_idt_gate(13, (uintptr_t)&_idt_stub_13_, 0x18, 0x8E);
	install_idt_gate(14, (uintptr_t)&_idt_stub_14_, 0x18, 0x8E);
	install_idt_gate(15, (uintptr_t)&_idt_stub_15_, 0x18, 0x8E);
	install_idt_gate(16, (uintptr_t)&_idt_stub_16_, 0x18, 0x8E);
	install_idt_gate(17, (uintptr_t)&_idt_stub_17_, 0x18, 0x8E);
	install_idt_gate(18, (uintptr_t)&_idt_stub_18_, 0x18, 0x8E);
	install_idt_gate(19, (uintptr_t)&_idt_stub_19_, 0x18, 0x8E);
	install_idt_gate(20, (uintptr_t)&_idt_stub_20_, 0x18, 0x8E);
	install_idt_gate(21, (uintptr_t)&_idt_stub_21_, 0x18, 0x8E);
	install_idt_gate(22, (uintptr_t)&_idt_stub_22_, 0x18, 0x8E);
	install_idt_gate(23, (uintptr_t)&_idt_stub_23_, 0x18, 0x8E);
	install_idt_gate(24, (uintptr_t)&_idt_stub_24_, 0x18, 0x8E);
	install_idt_gate(25, (uintptr_t)&_idt_stub_25_, 0x18, 0x8E);
	install_idt_gate(26, (uintptr_t)&_idt_stub_26_, 0x18, 0x8E);
	install_idt_gate(27, (uintptr_t)&_idt_stub_27_, 0x18, 0x8E);
	install_idt_gate(28, (uintptr_t)&_idt_stub_28_, 0x18, 0x8E);
	install_idt_gate(29, (uintptr_t)&_idt_stub_29_, 0x18, 0x8E);
	install_idt_gate(30, (uintptr_t)&_idt_stub_30_, 0x18, 0x8E);
	install_idt_gate(31, (uintptr_t)&_idt_stub_31_, 0x18, 0x8E);

	idtr.limit = sizeof(idt_entries) * 16 - 1;
	idtr.base = (uintptr_t)&idt_entries;
	_install_idt();

	printf("Initialized 64-bit IDT\n");
}
