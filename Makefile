PRODUCT := Arctan
CPPFLAGS =
QEMUFLAGS := -M q35,smm=off -m 4G -cdrom $(PRODUCT).iso -debugcon stdio

all: clean
	make -C bootstrap CPPFLAGS=$(CPPFLAGS)
	make -C kernel CPPFLAGS=$(CPPFLAGS)

	mkdir -p iso/boot/grub

	cp kernel/kernel.elf iso/boot
	cp bootstrap/bootstrap.elf iso/boot
	cp grub.cfg iso/boot/grub

	grub-mkrescue -o $(PRODUCT).iso iso


run: all
	qemu-system-x86_64 -enable-kvm -cpu host -d cpu_reset $(QEMUFLAGS)

clean:
	find . -type f -name "*.o" -delete
	find . -type f -name "*.elf" -delete
	find . -type f -name "*.iso" -delete
	rm -rf iso

debug: CPPFLAGS += -DDEBUG
debug: all

e9hack: CPPFLAGS += -DE9HACK
e9hack: all
