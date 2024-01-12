bits 64

global _kernel_entry
extern kernel_main
_kernel_entry:  lea rbp, [rel stack_end]
                mov rsp, rbp
                call kernel_main
                jmp $


section .bss
stack:  resb 0x1000
stack_end:
