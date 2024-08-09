/**
 * @file idt.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is part of Arctan.
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
 * The file which handles the 64-bit IDT.
*/
#include <arch/x86-64/ctrl_regs.h>
#include <global.h>
#include <arch/x86-64/idt.h>
#include <interface/printf.h>
#include <lib/util.h>
#include <arch/x86-64/apic/lapic.h>
#include <mp/smp.h>

static ARC_GenericSpinlock panic_lock;

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

struct interrupt_frame {
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
}__attribute__((packed));

STATIC_ASSERT(sizeof(struct interrupt_frame) == 40 , "Interrupt frame wrong size");

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
	idt_entries[i].ist = 1;
	idt_entries[i].reserved = 0;
}

void handle_gp(int error_code) {
	if (error_code == 0) {
		printf("#GP may have been caused by one of the following:\n");
		printf("\tAn operand of the instruction\n");
		printf("\tA selector from a gate which is the operand of the instruction\n");
		printf("\tA selector from a TSS involved in a task switch\n");
		printf("\tIDT vector number\n");

		return;
	}

	printf("Error code 0x%02X\n", error_code);
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

void interrupt_junction(struct ARC_Registers *regs, int code) {
	uint64_t error_code = 0x0;

	switch (code) {
		case 8:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 17:
		case 21:
			error_code = *(uint64_t *)regs->rsp;
			regs->rsp += 0x8;
			// RSP is now on RIP
			break;
	}

	struct interrupt_frame *frame = (struct interrupt_frame *)regs->rsp;

	if (code >= 32) {
		switch (code) {
			case 32: {
				int id = lapic_get_id();

				if (id == -1) {
					break;
				}

				struct ARC_ProcessorDescriptor *processor = &Arc_ProcessorList[id];

				if ((processor->flags & 1) == 1) {
					// Context switch
					struct ARC_Registers current = { 0 };

					current.rax = regs->rax;
					current.rbx = regs->rbx;
					current.rcx = regs->rcx;
					current.rdx = regs->rdx;
					current.rsi = regs->rsi;
					current.rdi = regs->rdi;
					current.rsp = frame->rsp;
					current.rbp = regs->rbp;
					current.rip = regs->rip;
					current.r8 = regs->r8;
					current.r9 = regs->r9;
					current.r10 = regs->r10;
					current.r11 = regs->r11;
					current.r12 = regs->r12;
					current.r13 = regs->r13;
					current.r14 = regs->r14;
					current.r15 = regs->r15;
					current.cs = frame->cs;
					current.rip = frame->rip;
					current.rflags = frame->rflags;
					current.ss = frame->ss;

					regs->rax = processor->registers.rax;
					regs->rbx = processor->registers.rbx;
					regs->rcx = processor->registers.rcx;
					regs->rdx = processor->registers.rdx;
					regs->rsi = processor->registers.rsi;
					regs->rdi = processor->registers.rdi;
					frame->rsp = processor->registers.rsp;
					regs->rbp = processor->registers.rbp;
					regs->rip = processor->registers.rip;
					regs->r8 = processor->registers.r8;
					regs->r9 = processor->registers.r9;
					regs->r10 = processor->registers.r10;
					regs->r11 = processor->registers.r11;
					regs->r12 = processor->registers.r12;
					regs->r13 = processor->registers.r13;
					regs->r14 = processor->registers.r14;
					regs->r15 = processor->registers.r15;
					frame->cs = processor->registers.cs;
					frame->rip = processor->registers.rip;
					frame->rflags = processor->registers.rflags;
					frame->ss = processor->registers.ss;

					smp_context_write(processor, &current);
					processor->flags &= ~1;
				}

				if (((processor->flags >> 1) & 1) == 1) {
					processor->registers.rax = regs->rax;
					processor->registers.rbx = regs->rbx;
					processor->registers.rcx = regs->rcx;
					processor->registers.rdx = regs->rdx;
					processor->registers.rsi = regs->rsi;
					processor->registers.rdi = regs->rdi;
					processor->registers.rsp = frame->rsp;
					processor->registers.rbp = regs->rbp;
					processor->registers.rip = regs->rip;
					processor->registers.r8 = regs->r8;
					processor->registers.r9 = regs->r9;
					processor->registers.r10 = regs->r10;
					processor->registers.r11 = regs->r11;
					processor->registers.r12 = regs->r12;
					processor->registers.r13 = regs->r13;
					processor->registers.r14 = regs->r14;
					processor->registers.r15 = regs->r15;
					processor->registers.cs = frame->cs;
					processor->registers.rip = frame->rip;
					processor->registers.rflags = frame->rflags;
					processor->registers.ss = frame->ss;

					processor->flags &= ~(1 << 1);
				}

				mutex_unlock(&processor->register_lock);

				mutex_lock(&processor->timer_lock);

				if (processor->timer_mode == ARC_LAPIC_TIMER_ONESHOT) {
					lapic_refresh_timer(processor->timer_ticks);
				}

				if (((processor->flags >> 2) & 1) == 1) {
					lapic_setup_timer(32, processor->timer_mode);
					lapic_refresh_timer(processor->timer_ticks);

					processor->flags &= ~(1 << 2);
				}

				mutex_unlock(&processor->timer_lock);

				break;
			}
			case 33: {
				handle_keyboard();
				break;
			}
		}

		goto EOI;
	}

	spinlock_lock(&panic_lock);

	int id = lapic_get_id();

	// Dump registers
	printf("Received Interrupt %d (%s) from LAPIC %d\n", code, exception_names[code], id);
	printf("RAX: 0x%016"PRIx64"\n", regs->rax);
	printf("RBX: 0x%016"PRIx64"\n", regs->rbx);
	printf("RCX: 0x%016"PRIx64"\n", regs->rcx);
	printf("RDX: 0x%016"PRIx64"\n", regs->rdx);
	printf("RSI: 0x%016"PRIx64"\n", regs->rsi);
	printf("RDI: 0x%016"PRIx64"\n", regs->rdi);
	printf("RSP: 0x%016"PRIx64"\tSS: %"PRIx64"\n", regs->rsp, frame->ss);
	printf("RBP: 0x%016"PRIx64"\n", regs->rbp);
	printf("R8 : 0x%016"PRIx64"\n", regs->r8);
	printf("R9 : 0x%016"PRIx64"\n", regs->r9);
	printf("R10: 0x%016"PRIx64"\n", regs->r10);
	printf("R11: 0x%016"PRIx64"\n", regs->r11);
	printf("R12: 0x%016"PRIx64"\n", regs->r12);
	printf("R13: 0x%016"PRIx64"\n", regs->r13);
	printf("R14: 0x%016"PRIx64"\n", regs->r14);
	printf("R15: 0x%016"PRIx64"\n", regs->r15);
	printf("RFLAGS: 0x016%"PRIx64"\n", frame->rflags);

	// Handle error code if present
	switch (code) {
		case 21:
			goto fall_through;
		case 17:
			goto fall_through;
		case 14:
			_x86_getCR2();
			printf("CR2: 0x%016"PRIx64"\n", _x86_CR2);
			_x86_getCR3();
			printf("CR3: 0x%016"PRIx64"\n", _x86_CR3);
			goto fall_through;
		case 13:
			handle_gp(error_code);
			goto fall_through;
		case 12:
			goto fall_through;
		case 11:
			goto fall_through;
		case 10:
			goto fall_through;
		case 8:
			fall_through:
			break;

	}

	printf("Return address: 0x%x:0x%016"PRIx64"\n", frame->cs, frame->rip);

	memset(Arc_MainTerm.framebuffer, 0, Arc_MainTerm.fb_width * Arc_MainTerm.fb_height * (Arc_MainTerm.fb_bpp / 8));
	term_draw(&Arc_MainTerm);

	spinlock_unlock(&panic_lock);

	ARC_HANG;
EOI:
	lapic_eoi();
}

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
extern void _idt_stub_32_();
extern void _idt_stub_33_();
extern void _idt_stub_34_();
extern void _idt_stub_35_();
extern void _idt_stub_36_();
extern void _idt_stub_37_();
extern void _idt_stub_38_();
extern void _idt_stub_39_();
extern void _idt_stub_40_();
extern void _idt_stub_41_();
extern void _idt_stub_42_();
extern void _idt_stub_43_();
extern void _idt_stub_44_();
extern void _idt_stub_45_();
extern void _idt_stub_46_();
extern void _idt_stub_47_();
extern void _idt_stub_48_();
extern void _idt_stub_49_();
extern void _idt_stub_50_();

void init_idt() {
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

	install_idt_gate(32, (uintptr_t)&_idt_stub_32_, 0x08, 0x8E);
	install_idt_gate(33, (uintptr_t)&_idt_stub_33_, 0x08, 0x8E);
	install_idt_gate(34, (uintptr_t)&_idt_stub_34_, 0x08, 0x8E);
	install_idt_gate(35, (uintptr_t)&_idt_stub_35_, 0x08, 0x8E);
	install_idt_gate(36, (uintptr_t)&_idt_stub_36_, 0x08, 0x8E);
	install_idt_gate(37, (uintptr_t)&_idt_stub_37_, 0x08, 0x8E);
	install_idt_gate(38, (uintptr_t)&_idt_stub_38_, 0x08, 0x8E);
	install_idt_gate(39, (uintptr_t)&_idt_stub_39_, 0x08, 0x8E);
	install_idt_gate(40, (uintptr_t)&_idt_stub_40_, 0x08, 0x8E);
	install_idt_gate(41, (uintptr_t)&_idt_stub_41_, 0x08, 0x8E);
	install_idt_gate(42, (uintptr_t)&_idt_stub_42_, 0x08, 0x8E);
	install_idt_gate(43, (uintptr_t)&_idt_stub_43_, 0x08, 0x8E);
	install_idt_gate(44, (uintptr_t)&_idt_stub_44_, 0x08, 0x8E);
	install_idt_gate(45, (uintptr_t)&_idt_stub_45_, 0x08, 0x8E);
	install_idt_gate(46, (uintptr_t)&_idt_stub_46_, 0x08, 0x8E);
	install_idt_gate(47, (uintptr_t)&_idt_stub_47_, 0x08, 0x8E);
	install_idt_gate(48, (uintptr_t)&_idt_stub_48_, 0x08, 0x8E);
	install_idt_gate(49, (uintptr_t)&_idt_stub_49_, 0x08, 0x8E);
	install_idt_gate(50, (uintptr_t)&_idt_stub_50_, 0x08, 0x8E);

	idtr.limit = sizeof(idt_entries) * 16 - 1;
	idtr.base = (uintptr_t)&idt_entries;

	_install_idt();

	init_static_spinlock(&panic_lock);

	ARC_DEBUG(INFO, "Ported IDT to 64-bits\n");
}
