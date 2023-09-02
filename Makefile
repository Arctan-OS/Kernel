CFILES := $(shell find ./src/ -f -name "*.c")
OFILES := $(CFILES:.c=.o)

ASFILES := $(shell find ./assembly/ -f -name '*.asm')
AOFILES := $(ASFILES:.asm=.o)

CFLAGS := -fno-builtin -fno-stack-protector -nostartfiles -nodefualtlibs \
		       -nostdlibs -ffreestanding -c -I src/include/ -masm=intel \
		       -Wall -Wextra

LDFLAGS := -Tlinker.ld -melf_x86_64 -o kernel.elf


all: $(OFILES)
	gcc assembly/help.c $()
	ld $(LDFLAGS) $(AOFILES) $(OFILES)

	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot
	cp grub.cfg iso/boot/grub

	genisoimage -R -b boot/grub/stage2_eltorito.bin -no-emul-boot -boot-load-size 4 -A ArcTan -input-charset utf8 -quiet -boot-info-table -o arctan.iso iso

run: all
	qemu-system-x86_64 -cdrom arctan.iso

%.o: src/%.c
	gcc $(CFLAGS) $< -o $@

%.o: assembly/%.asm
	nasm $(NASMFLAGS) $< -o $@

clean:
	rm -rf src/*.o
	rm -rf assembly/*.o
	rm -rf iso
	rm arctan.iso
