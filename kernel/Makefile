CFILES := $(shell find ./src/c/ -type f -name "*.c")
OFILES := $(CFILES:.c=.o)

ASFILES := $(shell find ./src/asm/ -type f -name '*.asm')
AOFILES := $(ASFILES:.asm=.o)

CFLAGS := -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
		       -nostdlib -ffreestanding -c -I src/include/ -masm=intel \
		       -Wall -Wextra

LDFLAGS := -Tlinker.ld -melf_x86_64 -o kernel.elf

all: $(OFILES) $(AOFILES)
	ld $(LDFLAGS) $(AOFILES) $(OFILES)

%.o: src/c/%.c
	gcc $(CFLAGS) $< -o $@

%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@
	
clean:
	rm -rf c/*.o
	rm kernel.elf
	rm -rf asm/