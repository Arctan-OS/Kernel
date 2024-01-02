;    Arctan - Operating System Kernel
;    Copyright (C) 2023  awewsomegamer
;
;    This file is apart of Arctan.
;
;    Arctan is free software; you can redistribute it and/or
;    modify it under the terms of the GNU General Public License
;    as published by the Free Software Foundation; version 2
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program; if not, write to the Free Software
;    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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
global _entry
_entry:				mov esp, stack_end					; Setup stack
					mov ebp, esp						; Make sure base gets the memo
					push eax							; Push multiboot2 loader signature
					push ebx							; Push boot information
					call helper							; HELP!
					jmp $

bits 32
section .bss

global _boot_meta
_boot_meta:
.mbi_struct:			resb 4								; Pointer to MBI Structure (Physical)
.first_free:			resb 4								; Pointer to the the first free address after HHDM (Physical)
.state:					resb 8								; Pointer to previous kernel state (Virtual)

global stack
global stack_end
stack:				resb STACK_SZ						; Reserve stack space
stack_end:
