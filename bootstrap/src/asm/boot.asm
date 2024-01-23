%if 0
/**
 * @file boot.asm
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

MAGIC				equ 0xE85250D6
ARCH				equ 0
LENGTH				equ (boot_header - boot_header_end)
CHECKSUM			equ -(MAGIC + ARCH + LENGTH)
STACK_SZ			equ 0x1000						; 4 KiB of stack should be fine for now

section .mb2header

align 8
boot_header:		dd MAGIC
					dd ARCH
					dd LENGTH
					dd CHECKSUM
align 8		
					; Module Align tag
					dw 0x6
					dw 0x0
					dd 0x8
align 8
					; Framebuffer Request tag
					dw 0x5
					dw 0x0
					dd 0x20
					dd 0x0 							; Allow bootloader to pick width
					dd 0x0 							; Allow bootloader to pick height
					dd 0x0 							; Allow bootloader to pick bpp
align 8			
					; End Of Tags tag
					dw 0x0
					dw 0x0
					dd 0x8
boot_header_end:	

section .text

extern helper
extern pml4
extern _kernel_station
global _entry
_entry:				mov esp, _stack_end					; Setup stack
					mov ebp, esp						; Make sure base gets the memo
					push eax							; Push multiboot2 loader signature
					push ebx							; Push boot information
					call helper							; HELP!

					mov eax, cr4
					or eax, 1 << 5
					mov cr4, eax

					mov eax, dword [pml4]
					mov cr3, eax

					mov ecx, 0xC0000080
					rdmsr
					or eax, 1 << 8
					wrmsr

					mov eax, cr0
					or eax, 1 << 31
					mov cr0, eax

					jmp 0x18:_kernel_station

section .bss

global _boot_meta
_boot_meta:
.mbi_struct:			resb 4								; Pointer to MBI Structure (Physical)
.first_free:			resb 4								; Pointer to the the first free address after HHDM (Physical)
.state:					resb 8								; Pointer to previous kernel state (Virtual)

global _stack
global _stack_end
_stack:				resb STACK_SZ						; Reserve stack space
_stack_end:
