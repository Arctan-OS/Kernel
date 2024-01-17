#include <arch/x86/idt.h>
#include <global.h>

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

struct junction_args {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t code;
}__attribute__((packed));

static const char *exception_names[] = {
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

void install_idt_gate(int i, uint32_t offset, uint16_t segment, uint8_t attrs) {
	idt_entries[i].offset1 = offset & 0xFFFF;
	idt_entries[i].offset2 = (offset >> 16) & 0xFFFF;
	idt_entries[i].segment = segment;
	idt_entries[i].attrs = attrs;
	idt_entries[i].zero = 0;
}

void handle_gp(int error_code) {
	if (error_code == 0) {
		ARC_DEBUG(ERR, "#GP may have been caused by one of the following:\n")
		ARC_DEBUG(ERR, "\tAn operand of the instruction\n")
		ARC_DEBUG(ERR, "\tA selector from a gate which is the operand of the instruction\n")
		ARC_DEBUG(ERR, "\tA selector from a TSS involved in a task switch\n")
		ARC_DEBUG(ERR, "\tIDT vector number\n");

		return;
	}

	ARC_DEBUG(ERR, "Error code 0x%02X\n", error_code);
}

void interrupt_junction(uint32_t esp) {
	// Cast structure pushed to stack into args
	struct junction_args *args = (struct junction_args *)(esp);

	// Pop the interrupt args->code off the stack
	args->esp += 4;

	// Dump registers
	ARC_DEBUG(ERR, "Received Interrupt %d, %s\n", args->code, exception_names[args->code]);
	ARC_DEBUG(ERR, "EAX: 0x%X\n", args->eax);
	ARC_DEBUG(ERR, "EBX: 0x%X\n", args->ebx);
	ARC_DEBUG(ERR, "ECX: 0x%X\n", args->ecx);
	ARC_DEBUG(ERR, "EDX: 0x%X\n", args->edx);
	ARC_DEBUG(ERR, "ESI: 0x%X\n", args->esi);
	ARC_DEBUG(ERR, "EDI: 0x%X\n", args->edi);
	ARC_DEBUG(ERR, "ESP: 0x%X\n", args->esp);
	ARC_DEBUG(ERR, "EBP: 0x%X\n", args->ebp);

	// If we enter none of the following conditions,
	// this is the return address
	uint32_t stack_elem = *(uint32_t *)args->esp;

	// Handle error code if present
	switch (args->code) {
	case 21:
		goto fall_through;
	case 17:
		goto fall_through;
	case 14:
		goto fall_through;
	case 13:
		handle_gp(stack_elem);
		goto fall_through;
	case 12:
		goto fall_through;
	case 11:
		goto fall_through;
	case 10:
		goto fall_through;
	case 8: {
fall_through:;
		// Pop the error code off the stack
		args->esp += 4;
		// This should now be the return address
		stack_elem = *(uint32_t *)args->esp;

		break;
	}
	}

	ARC_DEBUG(ERR, "Return address: 0x%X\n", stack_elem);

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

void install_idt() {
	install_idt_gate(0, (uintptr_t)&_idt_stub_0_, 0x08, 0x8E);
	install_idt_gate(1, (uintptr_t)&_idt_stub_1_, 0x08, 0x8E);
	install_idt_gate(2, (uintptr_t)&_idt_stub_2_, 0x08, 0x8E);
	install_idt_gate(3, (uintptr_t)&_idt_stub_3_, 0x08, 0x8E);
	install_idt_gate(4, (uintptr_t)&_idt_stub_4_, 0x08, 0x8E);
	install_idt_gate(5, (uintptr_t)&_idt_stub_5_, 0x08, 0x8E);
	install_idt_gate(6, (uintptr_t)&_idt_stub_6_, 0x08, 0x8E);
	install_idt_gate(7, (uintptr_t)&_idt_stub_7_, 0x08, 0x8E);
	install_idt_gate(8, (uintptr_t)&_idt_stub_8_, 0x08, 0x8E);
	install_idt_gate(9, (uintptr_t)&_idt_stub_9_, 0x08, 0x8E);
	install_idt_gate(10, (uintptr_t)&_idt_stub_10_, 0x08, 0x8E);
	install_idt_gate(11, (uintptr_t)&_idt_stub_11_, 0x08, 0x8E);
	install_idt_gate(12, (uintptr_t)&_idt_stub_12_, 0x08, 0x8E);
	install_idt_gate(13, (uintptr_t)&_idt_stub_13_, 0x08, 0x8E);
	install_idt_gate(14, (uintptr_t)&_idt_stub_14_, 0x08, 0x8E);
	install_idt_gate(15, (uintptr_t)&_idt_stub_15_, 0x08, 0x8E);
	install_idt_gate(16, (uintptr_t)&_idt_stub_16_, 0x08, 0x8E);
	install_idt_gate(17, (uintptr_t)&_idt_stub_17_, 0x08, 0x8E);
	install_idt_gate(18, (uintptr_t)&_idt_stub_18_, 0x08, 0x8E);
	install_idt_gate(19, (uintptr_t)&_idt_stub_19_, 0x08, 0x8E);
	install_idt_gate(20, (uintptr_t)&_idt_stub_20_, 0x08, 0x8E);
	install_idt_gate(21, (uintptr_t)&_idt_stub_21_, 0x08, 0x8E);
	install_idt_gate(22, (uintptr_t)&_idt_stub_22_, 0x08, 0x8E);
	install_idt_gate(23, (uintptr_t)&_idt_stub_23_, 0x08, 0x8E);
	install_idt_gate(24, (uintptr_t)&_idt_stub_24_, 0x08, 0x8E);
	install_idt_gate(25, (uintptr_t)&_idt_stub_25_, 0x08, 0x8E);
	install_idt_gate(26, (uintptr_t)&_idt_stub_26_, 0x08, 0x8E);
	install_idt_gate(27, (uintptr_t)&_idt_stub_27_, 0x08, 0x8E);
	install_idt_gate(28, (uintptr_t)&_idt_stub_28_, 0x08, 0x8E);
	install_idt_gate(29, (uintptr_t)&_idt_stub_29_, 0x08, 0x8E);
	install_idt_gate(30, (uintptr_t)&_idt_stub_30_, 0x08, 0x8E);
	install_idt_gate(31, (uintptr_t)&_idt_stub_31_, 0x08, 0x8E);

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

	ARC_DEBUG(INFO, "Installed IDT\n");
}
