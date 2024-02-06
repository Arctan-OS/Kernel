%if 0
/**
 * @file ctrl_regs.asm
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
bits 32
section .text

%macro GET_DATA 2
    push eax
    push esi
    lea esi, [rel %1]
    mov eax, %2
    mov [esi], eax
    pop esi
    pop eax
    ret
%endmacro

%macro SET_DATA 2
    push eax
    push esi
    lea esi, [rel %1]
    mov eax, [esi]
    mov %2, eax
    pop esi
    pop eax
    ret
%endmacro

global _x86_getCR0
_x86_getCR0:    GET_DATA _x86_CR0, cr0

global _x86_setCR0
_x86_setCR0:    SET_DATA _x86_CR0, cr0

global _x86_getCR1
_x86_getCR1:    GET_DATA _x86_CR1, cr1

global _x86_setCR1
_x86_setCR1:    SET_DATA _x86_CR1, cr1

global _x86_getCR2
_x86_getCR2:    GET_DATA _x86_CR2, cr2

global _x86_setCR2
_x86_setCR2:    SET_DATA _x86_CR2, cr2

global _x86_getCR3
_x86_getCR3:    GET_DATA _x86_CR3, cr3

global _x86_setCR3
_x86_setCR3:    SET_DATA _x86_CR3, cr3

global _x86_getCR4
_x86_getCR4:    GET_DATA _x86_CR4, cr4

global _x86_setCR4
_x86_setCR4:    SET_DATA _x86_CR4, cr4

section .bss
global _x86_CR0
global _x86_CR1
global _x86_CR2
global _x86_CR3
global _x86_CR4
_x86_CR0:   resb 8
_x86_CR1:   resb 8
_x86_CR2:   resb 8
_x86_CR3:   resb 8
_x86_CR4:   resb 8
