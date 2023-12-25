bits 32

global _install_gdt
extern gdtr
_install_gdt:       lgdt [gdtr]                     ; Load GDTR
                    jmp 0x08:_gdt_set_cs        ; Set CS
_gdt_set_cs:        mov ax, 0x10                ; Set AX to 32-bit data offset
                    mov ds, ax                  ; Set DS to AX
                    mov fs, ax                  ; Set FS to AX
                    mov gs, ax                  ; Set GS to AX
                    mov ss, ax                  ; Set SS to AX
                    mov es, ax                  ; Set ES to AX
                    ret                             ; Return
