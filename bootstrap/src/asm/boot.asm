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
global _entry
_entry:			mov esp, stack_end					; Setup stack
			mov ebp, esp						; Make sure base gets the memo
			push eax						; Push multiboot2 loader signature
			push ebx						; Push boot information
			call helper						; HELP!
			jmp $ 							; Hang

global outb
outb:
	mov al, [esp + 8]
	mov dx, [esp + 4]
	out dx, al
	ret

section .bss

global stack
global stack_end
stack:			resb STACK_SZ						; Reserve stack space
stack_end: