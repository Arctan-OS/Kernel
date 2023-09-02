CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name '*.asm')

OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

CFLAGS := -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
		       -nostdlib -ffreestanding -c -I src/include/ -masm=intel \
		       -Wall -Wextra -m32

LDFLAGS := -Tlinker.ld -melf_i386 -o kernel.elf

all: $(OFILES)
	ld $(LDFLAGS) $(AOFILES) $(OFILES)

%.o: src/c/%.c
	gcc $(CFLAGS) $< -o $@

%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@
	
clean:
	rm -rf c/*.o
	rm kernel.elf
	rm -rf asm/*.o