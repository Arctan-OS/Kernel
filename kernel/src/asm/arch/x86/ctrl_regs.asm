bits 64
section .text

%macro GET_DATA 2
    push rax
    push rsi
    lea rsi, [rel %1]
    mov rax, %2
    mov [rsi], rax
    pop rsi
    pop rax
    ret
%endmacro

%macro SET_DATA 2
    push rax
    push rsi
    lea rsi, [rel %1]
    mov rax, [rsi]
    mov %2, rax
    pop rsi
    pop rax
    ret
%endmacro

global _x86_getCR0
_x86_getCR0:    GET_DATA _x86_CR0, cr0

global _x86_setCR0
_x86_setCR0:    SET_DATA _x86_CR0, cr0

global _x86_getCR1
_x86_getCR1:    GET_DATA _x86_CR1, cr1

global _x86_setCR1
_x86_setCR1:    SET_DATA _x86_CR1, cr1

global _x86_getCR2
_x86_getCR2:    GET_DATA _x86_CR2, cr2

global _x86_setCR2
_x86_setCR2:    SET_DATA _x86_CR2, cr2

global _x86_getCR3
_x86_getCR3:    GET_DATA _x86_CR3, cr3

global _x86_setCR3
_x86_setCR3:    SET_DATA _x86_CR3, cr3

global _x86_getCR4
_x86_getCR4:    GET_DATA _x86_CR4, cr4

global _x86_setCR4
_x86_setCR4:    SET_DATA _x86_CR4, cr4

section .bss
global _x86_CR0
global _x86_CR1
global _x86_CR2
global _x86_CR3
global _x86_CR4
_x86_CR0:   resb 8
_x86_CR1:   resb 8
_x86_CR2:   resb 8
_x86_CR3:   resb 8
_x86_CR4:   resb 8
