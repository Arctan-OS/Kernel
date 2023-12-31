bits 64
section .text

global outb
outb:           push rdx
                push rax
                mov dx, di
                mov ax, si
                out dx, al
                pop rax
                pop rdx
                ret
