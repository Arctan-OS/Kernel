#include <mm/vmm.h>
#include <io/ctrl_reg.h>

struct pml4 *head = NULL;

void map_range(uint64_t physical, uint64_t virtual, size_t size) {

}

void unmap_range(uint64_t virtual, size_t size) {

}

void init_pml4() {
	
}

// Switch CR3 to specified table
void switch_tables(struct pml4 *table) {
	head = table;
	cr3_reg.base = (uintptr_t)(table) >> 12;
	set_cr3();
}

// Shootdown TLB Cache
void tlb_shoot_down() {

}

void initialize_vmm() {

}
