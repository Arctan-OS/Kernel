bits 64

global set_cr0
set_cr0: 	push rax
		mov rax, [cr0_reg]
		mov cr0, rax
		pop rax
		ret
	
global read_cr0
read_cr0: 	push rax
		mov rax, cr0
		mov [cr0_reg], rax
		pop rax
		ret
	
global set_cr1
set_cr1: 	push rax
		mov rax, [cr1_reg]
		mov cr1, rax
		pop rax
		ret
	
global read_cr1
read_cr1: 	push rax
		mov rax, cr1
		mov [cr1_reg], rax
		pop rax
		ret
	
global set_cr2
set_cr2: 	push rax
		mov rax, [cr2_reg]
		mov cr2, rax
		pop rax
		ret
	
global read_cr2
read_cr2: 	push rax
		mov rax, cr2
		mov [cr2_reg], rax
		pop rax
		ret
	
global set_cr3
set_cr3: 	push rax
		mov rax, [cr3_reg]
		mov cr3, rax
		pop rax
		ret
	
global read_cr3
read_cr3: 	push rax
		mov rax, cr3
		mov [cr3_reg], rax
		pop rax
		ret
	
global set_cr4
set_cr4: 	push rax
		mov rax, [cr4_reg]
		mov cr4, rax
		pop rax
		ret
	
global read_cr4
read_cr4: 	push rax
		mov rax, cr4
		mov [cr4_reg], rax
		pop rax
		ret

section .bss
global cr0_reg
global cr1_reg
global cr2_reg
global cr3_reg
global cr4_reg
cr0_reg: 	resb 8
cr1_reg: 	resb 8
cr2_reg: 	resb 8
cr3_reg: 	resb 8
cr4_reg: 	resb 8
