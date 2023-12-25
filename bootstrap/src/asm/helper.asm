global outb
outb:               mov al, [esp + 8]                   ; Get 8-bit data
                    mov dx, [esp + 4]                   ; Get 16-bit port
                    out dx, al                          ; Write data to port
                    ret                                     ; Return
