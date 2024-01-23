%if 0
/**
 * @file gdt.asm
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is apart of Arctan.
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

global _install_gdt
extern gdtr
_install_gdt:       lgdt [gdtr]                 ; Load GDTR
                    jmp 0x08:_gdt_set_cs        ; Set CS
_gdt_set_cs:        mov ax, 0x10                ; Set AX to 32-bit data offset
                    mov ds, ax                  ; Set DS to AX
                    mov fs, ax                  ; Set FS to AX
                    mov gs, ax                  ; Set GS to AX
                    mov ss, ax                  ; Set SS to AX
                    mov es, ax                  ; Set ES to AX
                    ret                         ; Return
