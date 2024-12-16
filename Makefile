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
PRODUCT := $(ARC_ROOT)/volatile/kernel.elf

CPP_DEBUG_FLAG := -DARC_DEBUG_ENABLE
CPP_E9HACK_FLAG := -DARC_E9HACK_ENABLE

ifeq (,$(wildcard ./e9hack.enable))
# Disable E9HACK
	CPP_SERIAL_FLAG :=
endif

ifeq (,$(wildcard ./debug.enable))
# Disable debugging
	CPP_DEBUG_FLAG :=
else
# Must set serial flag if debugging
	CPP_E9HACK_FLAG := -DARC_E9HACK_ENABLE
endif

ifneq (,$(wildcard ./hardware.enable))
# hardware.enable is present, disable E9
	CPP_E9HACK_FLAG :=
endif

CPPFLAGS := $(CPPFLAG_DEBUG) $(CPPFLAG_E9HACK) $(CPP_DEBUG_FLAG) $(CPP_E9HACK_FLAG) $(ARC_TARGET_ARCH) \
	    $(shell find ~+ -type d -wholename "*src/c/include" -exec echo "-I$1" {} \;)

export CPPFLAGS

CFLAGS := -c -m64 -c -masm=intel -fno-stack-protector -fno-stack-check \
		  -fno-lto -mno-mmx -mno-80387 -mno-red-zone -Wall \
		  -Wextra -mno-sse -mno-sse2 -ffreestanding -nostdlib \
		  -fPIE -ffunction-sections -march=x86-64-v2

export CFLAGS

LDFLAGS := -Tlinker.ld -melf_x86_64 --no-dynamic-linker -static -pie -o $(PRODUCT)

NASMFLAGS := -f elf64

export NASMFLAGS

CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name "*.asm")
OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

.PHONY: all
all: build
	$(LD) $(LDFLAGS) $(shell find . -type f -name "*.o")

.PHONY: definitions
definitions:
	python K/drivers/tools/gen_dri_defs.py ARC_REGISTER_DRIVER K/drivers/src/ K/drivers/src/c/include/drivers/dri_defs.h K/drivers/src/c/dri_defs.c

.PHONY: build
build: definitions
	$(MAKE) $(OFILES) drivers mm mp arch lib fs interface boot

.PHONY: mm
mm:
	$(MAKE) -C K/mm

.PHONY: mp
mp:
	$(MAKE) -C K/mp

.PHONY: arch
arch:
	$(MAKE) -C K/arch

.PHONY: arch-x86-64
arch-x86-64:
	$(MAKE) -C K/arch-x86-64

.PHONY: lib
lib:
	$(MAKE) -C K/lib

.PHONY: fs
fs:
	$(MAKE) -C K/fs

.PHONY: drivers
drivers:
	$(MAKE) -C K/drivers

.PHONY: interface
interface:
	$(MAKE) -C K/interface

.PHONY: boot
boot:
	$(MAKE) -C K/boot

src/c/%.o: src/c/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

src/asm/%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(PRODUCT)
	find . -type f -name "*.o" -delete

-include clean
