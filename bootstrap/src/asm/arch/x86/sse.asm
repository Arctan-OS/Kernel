bits 32
%define FXSAVE_SIZE 512

global _osxsave_support
_osxsave_support:   push ecx
                    push edx
                    push eax
                    lea eax, [rel _fxsave_space]
                    fxsave [eax]


                    mov ecx, 0
                    xgetbv
                    or eax, 0b111
                    xsetbv
                    pop eax
                    pop edx
                    pop ecx

                    ret

section .data
global _fxsave_space
align 16
_fxsave_space: times FXSAVE_SIZE db 0x0
