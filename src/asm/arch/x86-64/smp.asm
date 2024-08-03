%if 0
/**
 * @file smp.asm
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
*/
%endif

AP_INFO_OFF equ (__AP_START_INFO__ - __AP_START_BEGIN__)
AP_PML4_OFF equ AP_INFO_OFF + 0
AP_ENTRY_OFF equ AP_INFO_OFF + 8
AP_FLAGS_OFF equ AP_INFO_OFF + 16
AP_GDT_SIZE_OFF equ AP_INFO_OFF + 20
AP_GDT_ADDR_OFF equ AP_INFO_OFF + 22
AP_GDT_TABLE_OFF equ AP_INFO_OFF + 30
AP_STACK_OFF equ AP_INFO_OFF + 54

section .rodata

bits 16
global __AP_START_BEGIN__
__AP_START_BEGIN__:
        cli
        cld

        mov ax, cs
        mov ds, ax
        mov fs, ax
        mov gs, ax
        mov es, ax
        mov ss, ax

        jmp $

bits 32
pm:
        jmp $

bits 64
lm:
        jmp $

global __AP_START_INFO__
__AP_START_INFO__:
        .pml4: dq 0x0
        .entry: dq 0x0
        .flags: dd 0x0
        .gdt:
                .header: dw 0x17
                         dq 0x0
                .table: dq 0x0, 0x00CF9A000000FFFF, 0x00CF92000000FFFF
        .stack: dd 0xFAFA
global __AP_START_END__
__AP_START_END__:
