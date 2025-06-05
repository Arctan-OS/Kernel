/*
    Arctan-OS/Kernel - Operating System Kernel
    Copyright (C) 2023  awewsomegamer

    This file is part of Arctan-OS/Kernel.

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

ENTRY(_kernel_entry)

PHDRS {
      text PT_LOAD FLAGS((1 << 0) | (1 << 2));
      rodata PT_LOAD FLAGS((1 << 2));
      data PT_LOAD FLAGS((1 << 1) | (1 << 2));
}

SECTIONS {
    . = 0xFFFFFFFF80000000;

    __KERNEL_START__ = .;

    .text : {
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

    . = ALIGN(0x1000);

    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    } :data

    __KERNEL_END__ = .;

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
