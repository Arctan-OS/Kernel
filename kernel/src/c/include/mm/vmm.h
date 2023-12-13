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

#ifndef VMM_H
#define VMM_H

// Keep track of which virtual memory addresses are allocated
// and which are not. Provide functions for allocating and freeing
// addresses.

#define VMM_RANGE_TABLE_MAX 4096

#include <global.h>

struct pml1_entry { // PTE
	uint8_t P : 1;
	uint8_t RW : 1;
	uint8_t US : 1;
	uint8_t PWT : 1;
	uint8_t PCD : 1;
	uint8_t A : 1;
	uint8_t D : 1;
	uint8_t PAT : 1;
	uint8_t G : 1;
	uint8_t resv1 : 2;
	uint8_t R : 1;
	uint64_t address : 48;
	uint8_t prot_key: 3;
	uint8_t XD : 1;
}__attribute((packed));

struct pml1 {
	struct pml1_entry entries[512];
};

struct pml2_entry { // PDE
	uint8_t P : 1;
	uint8_t RW : 1;
	uint8_t US : 1;
	uint8_t PWT : 1;
	uint8_t PCD : 1;
	uint8_t A : 1;
	uint8_t resv1 : 1;
	uint8_t PAT : 1; // Keep this 0
	uint8_t resv2 : 3;
	uint8_t R : 1;
	uint64_t address : 48;
	uint8_t resv3 : 3;
	uint8_t XD : 1;
}__attribute__((packed));

struct pml2 {
	struct pml2_entry entries[512];
};

struct pml3_entry { // PDP
	uint8_t P : 1;
	uint8_t RW : 1;
	uint8_t US : 1;
	uint8_t PWT : 1;
	uint8_t PCD : 1;
	uint8_t A : 1;
	uint8_t resv1 : 1;
	uint8_t PAT : 1; // Keep this 0
	uint8_t resv2 : 3;
	uint8_t R : 1;
	uint64_t address : 48;
	uint8_t resv3 : 3;
	uint8_t XD : 1;
}__attribute__((packed));

struct pml3 {
	struct pml3_entry entries[512];
};

struct pml4_entry { // PDP
	uint8_t P : 1;
	uint8_t RW : 1;
	uint8_t PWT : 1;
	uint8_t PCD : 1;
	uint8_t A : 1;
	uint8_t resv1 : 6;
	uint8_t R : 1;
	uint64_t address : 48;
	uint8_t resv3 : 3;
	uint8_t XD : 1;
}__attribute__((packed));

struct pml4 {
	struct pml4_entry entries[512];
	struct pml4 *next;
};

void map_range(uint64_t physical, uint64_t virtual, size_t size);
void unmap_range(uint64_t virtual, size_t size);
struct pml4 *init_pml4();
void switch_tables(struct pml4 *table);
void tlb_shoot_down();
void initialize_vmm();

#endif
