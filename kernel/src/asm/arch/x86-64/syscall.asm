bits 64
section .text

global _syscall
extern syscall_handler
_syscall:
            push rcx
            push r11

            call syscall_handler

            pop r11
            pop rcx

            o64 sysret
