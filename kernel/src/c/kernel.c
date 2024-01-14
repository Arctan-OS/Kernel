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

struct ARC_BootMeta *Arc_BootMeta = NULL;
struct multiboot_tag_framebuffer *global_framebuffer = NULL;
uint8_t *global_kernel_font = NULL;

int kernel_main(struct ARC_BootMeta *boot_meta) {
	if (boot_meta == NULL) {
		ARC_DEBUG(ERR, "Pointer to boot information was not passed correctly, cannot continue\n");
		ARC_HANG
	}

	Arc_BootMeta = boot_meta;

	ARC_DEBUG(INFO, "Sucessfully entered long mode\n")

	install_gdt();
	install_idt();

	parse_mbi();

	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");

	for (;;);

	return 0;
}
