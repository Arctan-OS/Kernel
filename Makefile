#/**
# * @file Makefile
# *
# * @author awewsomegamer <awewsomegamer@gmail.com>
# *
# * @LICENSE
# * Arctan-OS/Kernel - Operating System Kernel
# * Copyright (C) 2023-2025 awewsomegamer
# *
# * This file is part of Arctan-OS/Kernel.
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

PRODUCT := $(ARC_ROOT)/volatile/kernel.elf

ifeq (,$(ARC_ROOT))
	ARC_ROOT := .
	PRODUCT := ./kernel.elf
endif

CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name "*.asm")
OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

CPPFLAGS := $(ARC_INCLUDE_DIRS) $(ARC_DEF_ARCH) $(ARC_DEF_SCHED) $(ARC_DEF_COM) $(ARC_DEF_DEBUG) \
	    $(shell find ~+ -type d -wholename "*src/c/include" -exec echo "-I$1" {} \;)

export CPPFLAGS

CFLAGS := -c -m64 -c -masm=intel -fno-stack-protector -fno-stack-check \
		  -fno-lto -mno-mmx -mno-80387 -mno-red-zone -Wall \
		  -Wextra -mno-sse -mno-sse2 -ffreestanding -nostdlib \
		  -fPIE -ffunction-sections -march=x86-64-v2 -O1

export CFLAGS

LDFLAGS := -Tlinker.ld -melf_x86_64 --no-dynamic-linker -static -pie -o $(PRODUCT)

NASMFLAGS := -f elf64

export NASMFLAGS

.PHONY: all
all: build
	$(LD) $(LDFLAGS) $(shell find . -type f -name "*.o")
#	$(STRIP) $(PRODUCT)

.PHONY: definitions
definitions:
	python K/drivers/tools/gen_dri_defs.py ARC_REGISTER_DRIVER K/drivers/src/ K/drivers/src/c/include/drivers/dri_defs.h K/drivers/src/c/dri_defs.c

.PHONY: build
build: pre-build definitions
ifeq ($(ARC_OPT_ARCH),x86_64)
# Clone architecture repository if needed
	$(MAKE) -C K/arch-x86-64
endif
	$(MAKE) $(OFILES) drivers mm mp arch lib fs userspace

.PHONY: pre-build
pre-build:
	find . -type f -name "*.o" -delete

.PHONY: mm
mm:
	$(MAKE) -C K/mm

.PHONY: mp
mp:
	$(MAKE) -C K/mp

.PHONY: arch
arch:

	$(MAKE) -C K/arch

.PHONY: lib
lib:
	$(MAKE) -C K/lib

.PHONY: fs
fs:
	$(MAKE) -C K/fs

.PHONY: drivers
drivers:
	$(MAKE) -C K/drivers

.PHONY: userspace
userspace:
	$(MAKE) -C K/userspace

src/c/%.o: src/c/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

src/asm/%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@

.PHONY: clean
clean:
	find . -type f -name "*.o" -delete
