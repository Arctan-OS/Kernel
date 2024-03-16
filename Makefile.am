#/**
# * @file Makefile
# *
# * @author awewsomegamer <awewsomegamer@gmail.com>
# *
# * @LICENSE
# * Arctan - Operating System Kernel
# * Copyright (C) 2023-2024 awewsomegamer
# *
# * This file is part of Arctan.
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

DISCARDABLE := \( ! -path "./initramfs" -and \( -name "*.o" -or -name "*.elf" -or -name "*.iso" \) \)

LD := x86_64-pc-arctan-mlibc-ld
export LD
CC := @CC@
export CC

.PHONY: all
all: kernel bootstrap

.PHONY: bootstrap
bootstrap:
	$(MAKE) -C bootstrap

.PHONY: kernel
kernel:
	$(MAKE) -C kernel

.PHONY: clean
clean:
#	find . -type f $(DISCARDABLE) -delete
#	rm -rf iso

.PHONY: documentation
documentation:
	doxygen Doxyfile
