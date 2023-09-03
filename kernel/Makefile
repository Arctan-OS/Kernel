PRODUCT := kernel.elf

CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name "*.asm")

OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

CFLAGS := -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
		       -nostdlib -ffreestanding -c -I src/c/icnlude/ -masm=intel \
		       -Wall -Wextra

LDFLAGS := -Tlinker.ld -melf_x86_64 -z page-size=0x1000 -o $(PRODUCT)

NASMFLAGS := -f elf64

all: $(OFILES)
	ld $(LDFLAGS) $(OFILES)

src/c/%.o: src/c/%.c
	gcc $(CFLAGS) $< -o $@

src/asm/%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@