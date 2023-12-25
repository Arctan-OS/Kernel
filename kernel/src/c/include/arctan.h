/*
    Arctan - Operating System Kernel
    Copyright (C) 2023  awewsomegamer

    This file is apart of Arctan.

    Arctan is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ARC_ARCTAN_H
#define ARC_ARCTAN_H

#define ARC_KERNEL_VADDR 0xFFFFFFFF80000000
#define ARC_HHDM_VADDR   0xFFFFC00000000000 // 192 TiB

#include <stdint.h>
#include <stddef.h>

struct ARC_KernMeta {
	void *kernel_heap;
	int kernel_heap_type; // 0: ARC_Freelist
}__attribute__((packed));

struct ARC_BootMeta {
	uint32_t mb2i; // Physical address of MBI2 structure
	uint32_t first_free; // Physical address of the last page used for the HHDM table
	struct Arc_KernMeta *state; // State of the last kernel
}__attribute__((packed));

#endif
