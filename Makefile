all:
	make -C bootstrap
	make -C kernel

clean:
	find . -type f -name "*.o" -delete
	find . -type f -name "*.elf" -delete
	