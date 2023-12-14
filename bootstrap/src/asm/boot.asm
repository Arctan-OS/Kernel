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
					mov dword [mbi_struct], ebx			; Save MBI Structure pointer
					call helper							; HELP!
					push eax							; Save PML4 address
					mov eax, cr4						; Read CR4
					or eax, 1 << 5						; Set PAE bit
					mov cr4, eax						; Write CR4
					pop eax								; Point EAX to PML4[0]
					mov cr3, eax						; Point CR3
					mov ecx, 0xC0000080					; Code for EFER MSR
					rdmsr								; Read EFER
					or eax, 1 << 8						; Set LME
					wrmsr								; Write EFER
					mov eax, cr0						; Read CR0
					or eax, 1 << 31						; Set PG bit
					mov cr0, eax						; Set CR0
					jmp 0x18:kernel_station				; Goto to station, set CS to 64-bit code offset

global outb
outb:				mov al, [esp + 8]					; Get 8-bit data
					mov dx, [esp + 4]					; Get 16-bit port
					out dx, al							; Write data to port
					ret									; Return

global _install_gdt
extern gdtr
_install_gdt:		lgdt [gdtr]					; Load GDTR
					jmp 0x08:_gdt_set_cs		; Set CS
_gdt_set_cs:		mov ax, 0x10				; Set AX to 32-bit data offset
					mov ds, ax					; Set DS to AX
					mov fs, ax					; Set FS to AX
					mov gs, ax					; Set GS to AX
					mov ss, ax					; Set SS to AX
					mov es, ax					; Set ES to AX
					ret							; Return

global _install_idt
extern idtr
_install_idt:		cli
					lidt [idtr]
					sei
					ret

%macro PUSH_ALL 0
					push esp
					push ebp
					push edi
					push esi
					push edx
					push ecx
					push ebx
					push eax
%endmacro

%macro POP_ALL 0
					pop eax
					pop ebx
					pop ecx
					pop edx
					pop esi
					pop edi
					pop ebp
					pop esp
%endmacro

extern interrupt_junction
%macro common_idt_stub 1
global _idt_stub_%1_
_idt_stub_%1_:		push %1
					PUSH_ALL
					push esp
					call interrupt_junction
					pop esp
					POP_ALL
					iret
%endmacro

common_idt_stub 0
common_idt_stub 1
common_idt_stub 2
common_idt_stub 3
common_idt_stub 4
common_idt_stub 5
common_idt_stub 6
common_idt_stub 7
common_idt_stub 8
common_idt_stub 9
common_idt_stub 10
common_idt_stub 11
common_idt_stub 12
common_idt_stub 13
common_idt_stub 14
common_idt_stub 15
common_idt_stub 16
common_idt_stub 17
common_idt_stub 18
common_idt_stub 19
common_idt_stub 20
common_idt_stub 21
common_idt_stub 22
common_idt_stub 23
common_idt_stub 24
common_idt_stub 25
common_idt_stub 26
common_idt_stub 27
common_idt_stub 28
common_idt_stub 29
common_idt_stub 30
common_idt_stub 31

bits 64

extern framebuffer_width
extern framebuffer_height
extern kernel_vaddr
extern hhdm_pml4_end

kernel_station:		mov ax, 0x20					; Set AX to 64-bit data offset
					mov ds, ax						; Set DS to AX
					mov fs, ax						; Set FS to AX
					mov gs, ax						; Set GS to AX
					mov ss, ax						; Set SS to AX
					mov es, ax						; Set ES to AX

					mov edi, dword [mbi_struct]		; Pass the pointer of MBI Structure
					mov esi, dword [hhdm_pml4_end]  ; Pass the pointer of the HHDM_PML4's end
					mov rax, qword [kernel_vaddr]
					call rax						; Call to kernel

					jmp $							; Spin

bits 32
section .bss

mbi_struct:			resb 4								; Pointer to MBI Structure

global stack
global stack_end
stack:				resb STACK_SZ						; Reserve stack space
stack_end:
