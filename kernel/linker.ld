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

ENTRY(kernel_main)

SECTIONS {
	. = 0xFFFFFFFF80000000;

	.text BLOCK(0x1000) : ALIGN(0x1000) {
		*(.text .text.*)
	}

	.rodata BLOCK(0x1000) : ALIGN(0x1000) {
		*(.rodata .rodata.*)
	}

	.data BLOCK(0x1000) : ALIGN(0x1000) {
		*(.data .data.*)
	}

	.bss BLOCK(0x1000) : ALIGN(0x1000) {
		*(COMMON)
		*(.bss .bss.*)
	}

	. = ALIGN(0x1000);
	__KERNEL_END__ = .;
}
