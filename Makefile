PRODUCT := ArcTan

all:
	make -C bootstrap
	make -C kernel

	mkdir -p iso/boot/grub

	cp kernel/kernel.elf iso/boot
	cp bootstrap/bootstrap.elf iso/boot
	cp grub.cfg iso/boot/grub

	grub-mkrescue -o $(PRODUCT).iso iso


run: clean all
	qemu-system-x86_64 -cdrom $(PRODUCT).iso

clean:
	find . -type f -name "*.o" -delete
	find . -type f -name "*.elf" -delete
	rm -rf iso
	