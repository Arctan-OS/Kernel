#include <mm/vmm.h>

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

struct pml4 *head = NULL;

void map_range(uint64_t physical, uint64_t virtual, size_t size) {

}

void unmap_range(uint64_t virtual, size_t size) {
	
}

void init_pml4() {
	
}

// Switch CR3 to specified table
void switch_tables(struct pml4 *table) {

}

// Shootdown TLB Cache
void tlb_shoot_down() {

}

void initialize_vmm() {

}
