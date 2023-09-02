all:
	make -C bootstrap
	make -C kernel

clean:
	rm -rf *.o
	rm -rf *.elf