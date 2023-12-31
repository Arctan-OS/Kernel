bits 32

global _init_sse
init_sse:           push eax
                    push ebx
                    push ecx
                    push edx

                    mov eax, 0x01
                    cpuid

                    and edx, 0b00000110000000000000000000000000
                    and ecx, 0b00000000000110000000001000000001

                    or edx, ecx
                    cmp edx, 0x00
                    je .no_sse

                    mov eax, cr0
                    and al, 0b000000110
                    mov cr0, eax

                    mov eax, cr4
                    or eax, (1 << 9)
                    or eax, (1 << 10)
                    mov cr4, eax

.no_sse:            jmp $
