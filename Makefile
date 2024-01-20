PRODUCT := Arctan

CPPFLAG_E9HACK :=
export CPPFLAG_E9HACK
CPPFLAG_DEBUG :=
export CPPFLAG_DEBUG

QEMUFLAGS := -M q35,smm=off -m 4G -cdrom $(PRODUCT).iso -debugcon stdio

all: clean
	make -C bootstrap
	make -C kernel

	mkdir -p iso/boot/grub

	cp kernel/kernel.elf iso/boot
	cp FONT.fnt iso/boot
	cp bootstrap/bootstrap.elf iso/boot
	cp grub.cfg iso/boot/grub

	grub-mkrescue -o $(PRODUCT).iso iso


run: all
	qemu-system-x86_64 -enable-kvm -cpu qemu64 -d cpu_reset $(QEMUFLAGS)

clean:
	find . -type f -name "*.o" -delete
	find . -type f -name "*.elf" -delete
	find . -type f -name "*.iso" -delete
	find . -type f -name "*.src.*" -delete
	rm -rf iso

nothing:

debug: CPPFLAG_DEBUG = -DARC_DEBUG_ENABLE
debug: e9hack

e9hack: CPPFLAG_E9HACK = -DARC_E9HACK_ENABLE
e9hack: all
