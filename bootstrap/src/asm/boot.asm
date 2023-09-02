[bits 32]

MAGIC    		equ 0x1BADB002						; Multiboot2
; ARCH    		equ 0
; LENGTH			equ (boot_header_end - boot_header)
FLAGS			equ 1
CHECKSUM 		equ -(MAGIC + FLAGS)			

align 8
boot_header:		dd MAGIC
			dd FLAGS
			; dd ARCH
			; dd LENGTH
			dd CHECKSUM

			dw 6							; Align modules to page boundaries (Type)
			dw 0							; Required (Flags = 0)
			dd 8							; Tag is 8 bytes in size

			dw 0
			dw 0
			dd 8
boot_header_end:


[extern helper]
[global _entry]
_entry:			push ebx
			call helper
			jmp $ 							; Hang