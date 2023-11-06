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

void init_pml4() {
	
}


void initialize_vmm() {

}
