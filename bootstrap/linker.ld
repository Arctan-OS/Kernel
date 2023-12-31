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

ENTRY(_entry)

PHDRS {
      text PT_LOAD;
      rodata PT_LOAD;
      data PT_LOAD;
}

SECTIONS {
	. = 1M;

	.text : {
		*(.mb2header)
		*(.text .text.*)
	} :text

    . = ALIGN(0x1000);

	.rodata : {
		*(.rodata .rodata.*)
	} :rodata

    . = ALIGN(0x1000);

	.data : {
		*(.data .data.*)
	} :data

	.bss : {
		*(COMMON)
		*(.bss .bss.*)
	} :data

	__BOOTSTRAP_END__ = .;
}
