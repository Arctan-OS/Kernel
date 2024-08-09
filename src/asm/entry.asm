%if 0
/**
 * @file entry.asm
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
bits 64

global _kernel_entry
extern kernel_main
_kernel_entry:
        cli
        mov rbp, __KERNEL_STACK__
        mov rsp, rbp
        call kernel_main
        jmp $

section .bss

        ;; 16 byte alignment is so SSE instructions
        ;; can use the stack without a #GP
align 16
        resb 0x4000
global __KERNEL_STACK__
__KERNEL_STACK__:
