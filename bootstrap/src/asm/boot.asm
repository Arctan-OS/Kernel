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
			dw 6
			dw 0
			dd 8
			
			; End Of Tags tag
			dw 0
			dw 0
			dd 8
boot_header_end:	

section .text

extern helper
extern pml4
extern install_gdt
global _entry
_entry:			mov esp, stack_end					; Setup stack
			mov ebp, esp						; Make sure base gets the memo
			push eax						; Push multiboot2 loader signature
			push ebx						; Push boot information
			call helper						; HELP!

			; mov eax, cr0
			; xor eax, 1 << 31
			; mov cr0, eax

			; mov ecx, 0xC0000080
			; rdmsr
			; or eax, 1 << 8
			; wrsmr

			; mov eax, cr4
			; or eax, 1 << 5
			; mov cr4, eax

			; mov eax, pml4
			; mov cr3, eax

			; mov eax, cr0
			; or eax, 1 << 31
			; mov cr0, eax

			; jmp 0x18:temp						; Long jump to kernel code
			jmp $

global outb
outb:			mov al, [esp + 8]
			mov dx, [esp + 4]
			out dx, al
			ret

global enable_paging_32
extern pml2_boot32
enable_paging_32:	mov eax, pml2_boot32
			mov cr3, eax

			mov eax, cr0
			or eax, (1 << 31)
			mov cr0, eax

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

temp:			jmp $

section .bss

global stack
global stack_end
stack:			resb STACK_SZ						; Reserve stack space
stack_end: