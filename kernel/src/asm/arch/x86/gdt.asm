bits 64

extern gdtr
global _install_gdt
_install_gdt:        lgdt [gdtr]                 ; Load GDTR
                     push 0x08
                     push _gdt_set_cs
                     retf
_gdt_set_cs:         mov ax, 0x10                ; Set AX to 32-bit data offset
                     mov ds, ax                  ; Set DS to AX
                     mov fs, ax                  ; Set FS to AX
                     mov gs, ax                  ; Set GS to AX
                     mov ss, ax                  ; Set SS to AX
                     mov es, ax                  ; Set ES to AX
                     ret                         ; Return
