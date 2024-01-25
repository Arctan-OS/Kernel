#/**
# * @file Makefile
# *
# * @author awewsomegamer <awewsomegamer@gmail.com>
# *
# * @LICENSE
# * Arctan - Operating System Kernel
# * Copyright (C) 2023-2024 awewsomegamer
# *
# * This file is apart of Arctan.
# *
# * Arctan is free software; you can redistribute it and/or
# * modify it under the terms of the GNU General Public License
# * as published by the Free Software Foundation; version 2
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# *
# * @DESCRIPTION
#*/
PRODUCT := Arctan

CPPFLAG_E9HACK :=
export CPPFLAG_E9HACK
CPPFLAG_DEBUG :=
export CPPFLAG_DEBUG

QEMUFLAGS := -M q35,smm=off -m 8G -cdrom $(PRODUCT).iso -debugcon stdio

DISCARDABLE := \( ! -path "./initramfs" -and \( -name "*.o" -or -name "*.elf" -or -name "*.iso" \) \)

all: clean
	make -C bootstrap
	make -C kernel

	mkdir -p iso/boot/grub

	# Put initramfs together
	cp -u kernel/kernel.elf initramfs/kernel/
	find ./initramfs -type f | cpio -o > iso/boot/initramfs.cpio

	# Copy various important things to grub directory
	cp bootstrap/bootstrap.elf iso/boot
	cp grub.cfg iso/boot/grub

	# Create ISO
	grub-mkrescue -o $(PRODUCT).iso iso

run: all
	qemu-system-x86_64 -enable-kvm -cpu qemu64 -d cpu_reset $(QEMUFLAGS)

clean:
	find . -type f $(DISCARDABLE) -delete

	rm -rf iso

documentation:
	doxygen Doxyfile

debug: CPPFLAG_DEBUG = -DARC_DEBUG_ENABLE
debug: e9hack

e9hack: CPPFLAG_E9HACK = -DARC_E9HACK_ENABLE
e9hack: all
