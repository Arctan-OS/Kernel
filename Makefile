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

CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name "*.asm")

OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

CPPFLAGS := $(CPPFLAG_DEBUG) $(CPPFLAG_E9HACK) -I src/c/include $(CPP_DEBUG_FLAG) $(CPP_E9HACK_FLAG) $(ARC_TARGET_ARCH)
CFLAGS := -c -m64 -c -masm=intel -fno-stack-protector -fno-stack-check \
		  -fno-lto -mno-mmx -mno-80387 -mno-red-zone -Wall \
		  -Wextra -mno-sse -mno-sse2 \
		  -fPIE -ffunction-sections -march=x86-64-v2

LDFLAGS := -Tlinker.ld -melf_x86_64 --no-dynamic-linker -static -pie -o $(PRODUCT)

NASMFLAGS := -f elf64

.PHONY: all
all: $(OFILES)
	$(LD) $(LDFLAGS) $(OFILES)

.PHONY: clean
clean:
	find . -name "*.o" -delete

src/c/%.o: src/c/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

src/asm/%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@
