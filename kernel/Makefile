#    Arctan - Operating System Kernel
#    Copyright (C) 2023  awewsomegamer
#
#    This file is apart of Arctan.
#
#    Arctan is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License
#    as published by the Free Software Foundation; version 2
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

PRODUCT := kernel.elf

CFILES := $(shell find ./src/c/ -type f -name "*.c")
ASFILES := $(shell find ./src/asm/ -type f -name "*.asm")

OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

CPPFLAGS ?=
CFLAGS := -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
		       -nostdlib -ffreestanding -c -I src/c/include/ -masm=intel \
		       -Wall -Wextra -mabi=sysv $(CPPFLAGS) -m64 -g


LDFLAGS := -Tlinker.ld -melf_x86_64 -z max-page-size=0x1000 -o $(PRODUCT)

NASMFLAGS := -f elf64

all: $(OFILES)
	ld $(LDFLAGS) $(OFILES)

src/c/%.o: src/c/%.c
	gcc $(CFLAGS) $< -o $@

src/asm/%.o: src/asm/%.asm
	nasm $(NASMFLAGS) $< -o $@
