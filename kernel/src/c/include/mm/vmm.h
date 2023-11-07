#ifndef VMM_H
#define VMM_H

// Keep track of which virtual memory addresses are allocated
// and which are not. Provide functions for allocating and freeing
// addresses.

#define VMM_RANGE_TABLE_MAX 4096

#include <global.h>

struct pml1_entry {
	uint64_t data;
}__attribute((packed));

struct pml1 {
	struct pml1_entry entries[512];
};

struct pml2_entry {
	uint64_t data;
}__attribute__((packed));

struct pml2 {
	struct pml2_entry entries[512];
};

struct pml3_entry {
	uint64_t data;
}__attribute__((packed));

struct pml3 {
	struct pml3_entry entries[512];
};

struct pml4_entry {
	uint64_t data;
}__attribute__((packed));

struct pml4 {
	struct pml4_entry entries[512];
};

void map_range(uint64_t physical, uint64_t virtual, size_t size);
void unmap_range(uint64_t virtual, size_t size);
void init_pml4();
void switch_tables(struct pml4 *table);
void tlb_shoot_down();
void initialize_vmm();

#endif