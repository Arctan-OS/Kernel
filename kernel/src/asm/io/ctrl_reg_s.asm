;    Arctan - Operating System Kernel
;    Copyright (C) 2023  awewsomegamer
;
;    This file is apart of Arctan.
;   
;    Arctan is free software; you can redistribute it and/or
;    modify it under the terms of the GNU General Public License
;    as published by the Free Software Foundation; version 2
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program; if not, write to the Free Software
;    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

bits 64
section .text

global set_cr0
set_cr0: 	push rax
		push rsi
		lea rsi, [rel cr0_reg]
	 	mov rax, [rsi]
		pop rsi
		mov cr0, rax
		pop rax
		ret
	
global read_cr0
read_cr0: 	push rax
		mov rax, cr0
		push rsi
		lea rsi, [rel cr0_reg]
	 	mov [rsi], rax
		pop rsi
		pop rax
		ret
	
global set_cr1
set_cr1: 	push rax
		push rsi
		lea rsi, [rel cr1_reg]
	 	mov rax, [rsi]
		pop rsi
		mov cr1, rax
		pop rax
		ret
	
global read_cr1
read_cr1: 	push rax
		mov rax, cr1
		push rsi
		lea rsi, [rel cr1_reg]
	 	mov [rsi], rax
		pop rsi
		pop rax
		ret
	
global set_cr2
set_cr2: 	push rax
		push rsi
		lea rsi, [rel cr2_reg]
	 	mov rax, [rsi]
		pop rsi
		mov cr2, rax
		pop rax
		ret
	
global read_cr2
read_cr2: 	push rax
		mov rax, cr2
		push rsi
		lea rsi, [rel cr2_reg]
	 	mov [rsi], rax
		pop rsi
		pop rax
		ret
	
global set_cr3
set_cr3: 	push rax
		push rsi
		lea rsi, [rel cr3_reg]
	 	mov rax, [rsi]
		pop rsi
		mov cr3, rax
		pop rax
		ret
	
global read_cr3
read_cr3: 	push rax
		mov rax, cr3
		push rsi
		lea rsi, [rel cr3_reg]
	 	mov [rsi], rax
		pop rax
		ret
	
global set_cr4
set_cr4: 	push rax
		push rsi
		lea rsi, [rel cr4_reg]
	 	mov rax, [rsi]
		pop rsi
		mov cr4, rax
		pop rax
		ret
	
global read_cr4
read_cr4: 	push rax
		mov rax, cr4
		push rsi
		lea rsi, [rel cr4_reg]
	 	mov [rsi], rax
		pop rsi
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
