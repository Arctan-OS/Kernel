[bits 32]

MAGIC    equ 0x1BADB002
FLAGS    equ 0					; No flags
CHECKSUM equ -(MAGIC + FLAGS)			
MODETYPE equ 0					; Linear framebuffer
WIDTH	 equ 1024				; 1024 pixels in width
HEIGHT   equ 768				; 768 pixels in height
BPP      equ 16					; 16 bit color

boot_header:		.dword MAGIC
			.dword FLAGS
			.dword CHECKSUM


[extern helper]
_entry:			call helper
			jmp $ 			; Hang