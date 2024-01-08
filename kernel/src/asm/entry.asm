bits 64

global _kernel_entry
extern kernel_main
_kernel_entry:  call kernel_main
                jmp $
