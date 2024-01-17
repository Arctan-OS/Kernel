/*
    Arctan - Operating System Kernel
    Copyright (C) 2023  awewsomegamer

    This file is apart of Arctan.

    Arctan is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <arctan.h>
#include <global.h>
#include <arch/x86/io/port.h>
#include <stdint.h>
#include <interface/printf.h>
#include <arch/x86/ctrl_regs.h>
#include <multiboot/mbparse.h>

#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>

#include <interface/terminal.h>

struct ARC_BootMeta *Arc_BootMeta = NULL;
struct ARC_TermMeta main_terminal = { 0 };
char main_terminal_mem[120 * 120] = { 0 };

int kernel_main(struct ARC_BootMeta *boot_meta) {
	ARC_DEBUG(INFO, "%p\n", main_terminal_mem);

	for (int i = 0; i < 120 * 120; i++) {
		if (main_terminal_mem[i] != 0) {
			ARC_DEBUG(INFO, "NON ZERO @ %d, VALUE %02X\n", i, (uint8_t)main_terminal_mem[i]);
		}
	}

	if (boot_meta == NULL) {
		ARC_DEBUG(ERR, "Pointer to boot information was not passed correctly, cannot continue\n");
		ARC_HANG
	}

	for (;;);

	Arc_BootMeta = boot_meta;

	main_terminal.term_width = 120;
	main_terminal.term_height = 120;
	main_terminal.term_mem = main_terminal_mem;

	main_terminal.cx = 0;
	main_terminal.cy = 0;

	ARC_DEBUG(INFO, "Sucessfully entered long mode\n")

	install_gdt();
	install_idt();

	parse_mbi();

	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");

	for (int i = 0; i < 600; i++) {
		for (int y = 0; y < main_terminal.fb_height; y++) {
			for (int x = 0; x < main_terminal.fb_width; x++) {
				*((uint32_t *)main_terminal.framebuffer + (y * main_terminal.fb_width) + x) = (x * y * i / 300) & 0x3FFF;
			}
		}
	}

	for (;;) {
		Arc_TermDraw(&main_terminal);
	}

	return 0;
}
