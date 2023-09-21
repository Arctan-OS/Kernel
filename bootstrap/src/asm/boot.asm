bits 32

MAGIC			equ 0xE85250D6
ARCH			equ 0
LENGTH			equ (boot_header_end - boot_header_end)
CHECKSUM		equ -(MAGIC + ARCH + LENGTH)
STACK_SZ		equ 0x1000						; 4 KiB of stack should be fine for now

section .mb2header

align 8
boot_header:		dd MAGIC
			dd ARCH
			dd LENGTH
			dd CHECKSUM
		
			; Module Align tag
			dw 0x6
			dw 0x0
			dd 0x8

			; Framebuffer Request Tag
			dw 0x5
			dw 0x0
			dd 0x20
			dd 0x0 							; Allow bootloader to pick width
			dd 0x0 							; Allow bootloader to pick height
			dd 0x0 							; Allow bootloader to picl bpp
			
			; End Of Tags tag
			dw 0x0
			dw 0x0
			dd 0x8
boot_header_end:	

section .text

extern helper
extern pml4
global _entry
_entry:			mov esp, stack_end					; Setup stack
			mov ebp, esp						; Make sure base gets the memo

			push eax						; Push multiboot2 loader signature
			push ebx						; Push boot information
			call helper						; HELP!

			; PAE
			mov edx, cr4
			or edx, 1 << 5
			mov cr4, edx

			; Point to table
			mov eax, pml4
			mov cr3, eax

			; LME
			mov ecx, 0xC0000080
			rdmsr
			or eax, 1 << 8
			wrmsr
			
			; Enable paging
			mov eax, cr0
			or eax, 1 << 31
			mov cr0, eax

			jmp 0x18:temp						; Long jump to kernel code

global outb
outb:			mov al, [esp + 8]
			mov dx, [esp + 4]
			out dx, al
			ret

global _install_gdt
extern gdtr
_install_gdt:		lgdt [gdtr]
			jmp 0x08:_gdt_set_cs
_gdt_set_cs:		mov ax, 0x10
			mov ds, ax
			mov fs, ax
			mov gs, ax
			mov ss, ax
			mov es, ax
			ret

bits 64

temp:			mov ax, 0x20
			mov ds, ax
			mov fs, ax
			mov gs, ax
			mov ss, ax
			mov es, ax
			jmp $


bits 32
section .bss

global stack
global stack_end
stack:			resb STACK_SZ						; Reserve stack space
stack_end: