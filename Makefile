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
PRODUCT := kernel.elf

CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name "*.asm")

OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

CPPFLAGS := $(CPPFLAG_DEBUG) $(CPPFLAG_E9HACK) -I src/c/include -I $(ARC_ROOT)/initramfs/include
CFLAGS := -m64 -c -masm=intel -fno-stack-protector -nostdlib -fno-stack-check \
		  -fno-lto -march=x86-64 -mno-mmx -mno-80387 -mno-red-zone -Wall \
		  -Wextra -ffreestanding -fPIE

LDFLAGS := -Tlinker.ld -melf_x86_64 --no-dynamic-linker -static -nostdlib -pie -o $(PRODUCT)

NASMFLAGS := -f elf64

.PHONY: all
all: $(OFILES)
	$(LD) $(LDFLAGS) $(OFILES)

src/c/%.o: src/c/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

src/asm/%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@
