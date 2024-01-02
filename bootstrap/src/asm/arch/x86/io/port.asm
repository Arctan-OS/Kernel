bits 32

global outb
outb:           mov eax, [esp + 8]
                mov edx, [esp + 4]
                out dx, al
                ret
