/**
 * @file idt.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is apart of Arctan.
 *
 * Arctan is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
*/
#include <arch/x86/ctrl_regs.h>
#include <arch/x86/io/port.h>
#include <global.h>
#include <arch/x86/idt.h>
#include <interface/printf.h>

struct idt_desc {
	uint16_t limit;
	uint64_t base;
}__attribute__((packed));
struct idt_desc idtr;

struct idt_entry {
	uint16_t offset1;
	uint16_t segment;
	uint8_t ist;
	uint8_t attrs;
	uint16_t offset2;
	uint32_t offset3;
	uint32_t reserved;
}__attribute__((packed));
static struct idt_entry idt_entries[256];

struct junction_args {
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t rsp;
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

void install_idt_gate(int i, uint64_t offset, uint16_t segment, uint8_t attrs) {
	idt_entries[i].offset1 = offset & 0xFFFF;
	idt_entries[i].offset2 = (offset >> 16) & 0xFFFF;
	idt_entries[i].offset3 = (offset >> 32) & 0xFFFFFFFF;
	idt_entries[i].segment = segment;
	idt_entries[i].attrs = attrs;
	idt_entries[i].reserved = 0;
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

// TEMP
char *alpha_numeric __attribute__((section(".data"))) = "  1234567890-=\b\tqwertyuiop[]\n asdfghjkl;'` \\zxcvbnm,./ *   ";
char *ALPHA_NUMERIC __attribute__((section(".data"))) = "  !@#$%^&*()_+\b\tQWERTYUIOP{}\n ASDFGHJKL:\"~ |ZXCVBNM<>? *   ";
uint8_t caps __attribute__((section(".data"))) = 0;

// TEMP
void handle_keyboard() {
        uint8_t scancode = inb(0x60);

        if (scancode == 0xAA) // Shift release
                caps = 0;

        if (scancode < 0x81) {
                switch (scancode) {
                // Implement handling for function keys (arrows, caps, shift, f keys, etc...)
                case 0x2A: // Shift
                        caps = 1;
                        break;

                case 0x3A: // Caps lock
                        caps = !caps;
                        break;

                default:
                        printf("%c", *((caps ? ALPHA_NUMERIC : alpha_numeric) + scancode));
                }
	}
}

void interrupt_junction(struct junction_args *args, int code) {
	// TEMP
	if (code == 33) {
		handle_keyboard();
		goto EOI;
	}

	// Dump registers
	ARC_DEBUG(ERR, "Received Interrupt %d, %s\n", code, exception_names[code]);
	ARC_DEBUG(ERR, "RAX: 0x%"PRIX64"\n", args->rax);
	ARC_DEBUG(ERR, "RBX: 0x%"PRIX64"\n", args->rbx);
	ARC_DEBUG(ERR, "RCX: 0x%"PRIX64"\n", args->rcx);
	ARC_DEBUG(ERR, "RDX: 0x%"PRIX64"\n", args->rdx);
	ARC_DEBUG(ERR, "RSI: 0x%"PRIX64"\n", args->rsi);
	ARC_DEBUG(ERR, "RDI: 0x%"PRIX64"\n", args->rdi);
	ARC_DEBUG(ERR, "RSP: 0x%"PRIX64"\n", args->rsp);
	ARC_DEBUG(ERR, "RBP: 0x%"PRIX64"\n", args->rbp);

	// If we enter none of the following conditions,
	// this is the return address
	uint64_t stack_elem = *(uint64_t *)args->rsp;

	// Handle error code if present
	switch (code) {
	case 21:
		goto fall_through;
	case 17:
		goto fall_through;
	case 14:
		_x86_getCR2();
		ARC_DEBUG(ERR, "CR2: 0x%"PRIX64"\n", _x86_CR2);
		_x86_getCR3();
		ARC_DEBUG(ERR, "CR3: 0x%"PRIX64"\n", _x86_CR3);
		goto fall_through;
	case 13:
		handle_gp(stack_elem);
		goto fall_through;
	case 12:
	case 11:
		goto fall_through;
		goto fall_through;
	case 10:
		goto fall_through;
	case 8: {
		// There is nothing we can do

fall_through:;
		// Pop the error code off the stack
		args->rsp += 8;
		// This should now be the return address
		stack_elem = *(uint64_t *)args->rsp;

		break;
	}
	}

	ARC_DEBUG(ERR, "Return address: 0x%"PRIX64"\n", stack_elem);

		for (;;);
EOI:
	// Send EOI
	if (code >= 8) {
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

extern void _idt_stub_33_();

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

	install_idt_gate(33, (uintptr_t)&_idt_stub_33_, 0x08, 0x8E);

	idtr.limit = sizeof(idt_entries) * 16 - 1;
	idtr.base = (uintptr_t)&idt_entries;

	_install_idt();

	outb(0x21, 0b11111101);
	outb(0xA1, 0b11111101);

	ARC_DEBUG(INFO, "Ported IDT to 64-bits\n");
}
