bits 32
section .text

MAGIC    		equ 0xE85250D6						; Multiboot2
										; No header found??? https://tenor.com/view/guh-gif-25116077
ARCH    		equ 0
LENGTH			equ (boot_header_end - boot_header)
CHECKSUM 		equ -(MAGIC + ARCH)			


align 8
boot_header:		dd MAGIC
			dd ARCH
			dd LENGTH
			dd CHECKSUM

			; dw 6							; Align modules to page boundaries (Type)
			; dw 0							; Required (Flags = 0)
			; dd 8							; Tag is 8 bytes in size

			dw 0
			dw 0
			dd 8
boot_header_end:	
			db "HEADER END"


[extern helper]
[global _entry]
_entry:			push ebx
			call helper
			jmp $ 							; Hang