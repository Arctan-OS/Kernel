#include "global.h"
#include <mm/vmm.h>
#include <io/ctrl_reg.h>
#include <mm/alloc.h>
#include <string.h>

struct pml4 *head = NULL;

void map_range(uint64_t physical, uint64_t virtual, size_t size) {

}

void unmap_range(uint64_t virtual, size_t size) {

}

struct pml4 *init_pml4() {
	struct pml4 *table = (struct pml4 *)alloc_pages(kernel_heap_pool, 1);
	memset(table, 0, PAGE_SIZE);

	head->next = table;
	head = table;

	return table;
}

struct pml4 *free_pml4(struct pml4 *table) {
	for (int i = 0; i < 512; i++) {
		if (table->entries[i].P == 1) {

		}
	}

	return table;
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
