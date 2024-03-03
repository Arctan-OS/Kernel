/*
    Arctan - Operating System Kernel
    Copyright (C) 2023  awewsomegamer

    This file is part of Arctan.

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

    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(0x1000);

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(0x1000);

    __DRIVERS0_START = .;

    .drivers.0 : {
        *(.drivers.0)
    } :rodata

    __DRIVERS1_START = .;

    .drivers.1 : {
        *(.drivers.1)
    } :rodata

    __DRIVERS2_START = .;
    .drivers.2 : {
        *(.drivers.2)
    } :rodata

    __DRIVERS3_START = .;
    .drivers.3 : {
        *(.drivers.3)
    } :rodata

    __DRIVERS_END = .;

    . = ALIGN(0x1000);

    .data : {
        *(.data .data.*)
    } :data

    . = ALIGN(0x1000);

    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    } :data

    . = ALIGN(0x1000);

    __KERNEL_END__ = .;

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
