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

			; ; Framebuffer Request Tag
			; dw 0x5
			; dw 0x0
			; dd 0x20
			; dd 0x0 							; Allow bootloader to pick width
			; dd 0x0 							; Allow bootloader to pick height
			; dd 0x0 							; Allow bootloader to picl bpp
			
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
			mov eax, cr4						; Read CR4
			or eax, 1 << 5						; Set PAE bit
			mov cr4, eax						; Write CR4
			mov eax, pml4						; Point EAX to PML4[0]
			mov cr3, eax						; Point CR3
			mov ecx, 0xC0000080					; Code for EFER MSR
			rdmsr							; Read EFER
			or eax, 1 << 8						; Set LME
			wrmsr							; Write EFER
			mov eax, cr0						; Read CR0
			or eax, 1 << 31						; Set PG bit
			mov cr0, eax						; Set CR0
			jmp 0x18:kernel_station					; Goto to station, set CS to 64-bit code offset

global outb
outb:			mov al, [esp + 8]					; Get 8-bit data
			mov dx, [esp + 4]					; Get 16-bit port
			out dx, al						; Write data to port
			ret							; Return

global _install_gdt
extern gdtr
_install_gdt:		lgdt [gdtr]						; Load GDTR
			jmp 0x08:_gdt_set_cs					; Set CS
_gdt_set_cs:		mov ax, 0x10						; Set AX to 32-bit data offset
			mov ds, ax						; Set DS to AX
			mov fs, ax						; Set FS to AX
			mov gs, ax						; Set GS to AX
			mov ss, ax						; Set SS to AX
			mov es, ax						; Set ES to AX
			ret							; Return

bits 64

kernel_station:		mov ax, 0x20						; Set AX to 64-bit data offset
			mov ds, ax						; Set DS to AX
			mov fs, ax						; Set FS to AX
			mov gs, ax						; Set GS to AX
			mov ss, ax						; Set SS to AX
			mov es, ax						; Set ES to AX

			mov al, byte [0xFFFFFFFF80000000]
			; add rax, 'A'
			mov [0xB8000], al

			; mov rax, 0xFFFFFFFF80000000

			; call rax						; Call to kernel		Seems like kernel does a hop, skip,
										;				and a leap resulting in a triple fault.
			jmp $							; Spin


bits 32
section .bss

global stack
global stack_end
stack:			resb STACK_SZ						; Reserve stack space
stack_end: