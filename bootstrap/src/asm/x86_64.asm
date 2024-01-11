bits 64

extern kernel_entry
extern _boot_meta
extern _stack_end
global _kernel_station
_kernel_station:    mov rax, [kernel_entry]
                    lea rdi, [rel _boot_meta]

                    mov rbp, _stack_end                 ; Add HHDM address
                    mov rsp, rbp

                    jmp rax
                    jmp $
