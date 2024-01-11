bits 64

global _install_idt
extern idtr
_install_idt:       cli
                    push rax
                    lea rax, idtr
                    lidt [rax]
                    pop rax
                    sti
                    ret

%macro PUSH_ALL 0
                    push rsp
                    push rbp
                    push rdi
                    push rsi
                    push rdx
                    push rcx
                    push rbx
                    push rax
%endmacro

%macro POP_ALL 0
                    pop rax
                    pop rbx
                    pop rcx
                    pop rdx
                    pop rsi
                    pop rdi
                    pop rbp
                    pop rsp
%endmacro

extern interrupt_junction
%macro common_idt_stub 1
global _idt_stub_%1_
_idt_stub_%1_:      PUSH_ALL
                    mov rdi, rsp
                    mov rsi, %1
                    call interrupt_junction
                    POP_ALL
                    iretq
%endmacro

common_idt_stub 0
common_idt_stub 1
common_idt_stub 2
common_idt_stub 3
common_idt_stub 4
common_idt_stub 5
common_idt_stub 6
common_idt_stub 7
common_idt_stub 8
common_idt_stub 9
common_idt_stub 10
common_idt_stub 11
common_idt_stub 12
common_idt_stub 13
common_idt_stub 14
common_idt_stub 15
common_idt_stub 16
common_idt_stub 17
common_idt_stub 18
common_idt_stub 19
common_idt_stub 20
common_idt_stub 21
common_idt_stub 22
common_idt_stub 23
common_idt_stub 24
common_idt_stub 25
common_idt_stub 26
common_idt_stub 27
common_idt_stub 28
common_idt_stub 29
common_idt_stub 30
common_idt_stub 31
