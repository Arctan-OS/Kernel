[bits 32]

FLAG_ALIGN_MODULES 	equ (1 << 0)
FLAG_MODS		equ (1 << 3)

MAGIC    		equ 0x1BADB002
FLAGS    		equ FLAG_ALIGN_MODULES | FLAG_MODS			; No flags
CHECKSUM 		equ -(MAGIC + FLAGS)			
MODETYPE 		equ 0							; Linear framebuffer
WIDTH	 		equ 1024						; 1024 pixels in width
HEIGHT   		equ 768							; 768 pixels in height
BPP      		equ 16							; 16 bit color

boot_header:		.dword MAGIC
			.dword FLAGS
			.dword CHECKSUM


[extern helper]
_entry:			call helper
			jmp $ 			; Hang