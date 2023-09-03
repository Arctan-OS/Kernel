bits 32

MAGIC			equ 0xE85250D6
ARCH			equ 0
LENGTH			equ (boot_header_end - boot_header_end)
CHECKSUM		equ -(MAGIC + ARCH + LENGTH)

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

[extern helper]
[global _entry]
_entry:			push eax
			push ebx
			call helper
			jmp $ 							; Hang