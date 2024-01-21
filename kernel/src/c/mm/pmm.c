#include <arctan.h>
#include <global.h>
#include <mm/freelist.h>
#include <multiboot/multiboot2.h>
#include <mm/pmm.h>
#include <stdint.h>

static struct ARC_FreelistMeta *arc_physical_mem = NULL;

void *Arc_AllocPMM() {
	if (arc_physical_mem == NULL) {
		return NULL;
	}

	return Arc_ListAlloc(arc_physical_mem);
}

void *Arc_FreePMM(void *address) {
	if (arc_physical_mem == NULL) {
		return NULL;
	}

	return Arc_ListFree(arc_physical_mem, address);
}

void Arc_InitPMM(struct multiboot_tag_mmap *mmap, uint32_t boot_state) {
	arc_physical_mem = (struct ARC_FreelistMeta *)(boot_state + ARC_HHDM_VADDR);

	ARC_DEBUG(INFO, "Bootstrap allocator: { B:%p C:%p H:%p SZ:%d }\n", arc_physical_mem->base, arc_physical_mem->ciel, arc_physical_mem->head, arc_physical_mem->object_size);

	// Revise old PMM meta to use HHDM addresses
	arc_physical_mem->base = (struct ARC_FreelistNode *)((uintptr_t)arc_physical_mem->base + ARC_HHDM_VADDR);
	arc_physical_mem->ciel = (struct ARC_FreelistNode *)((uintptr_t)arc_physical_mem->ciel + ARC_HHDM_VADDR);
	arc_physical_mem->head = (struct ARC_FreelistNode *)((uintptr_t)arc_physical_mem->head + ARC_HHDM_VADDR);

	// Update old PMM freelist to point to HHDM
	// addresses
	struct ARC_FreelistNode *current = arc_physical_mem->head;

	while (current != NULL) {
		if ((((uintptr_t)current->next) >> 32) != (uint32_t)(ARC_HHDM_VADDR >> 32)) {
			// Next pointer is not a HHDM address]
			// NOTE: Bootstrapper's PMM should only use the lower 32-bits
			//       of the address
			current->next = (struct ARC_FreelistNode *)(((uintptr_t)current->next & UINT32_MAX) + ARC_HHDM_VADDR);
			current = current->next;
		}
	}

	int entry_count = (mmap->size - sizeof(struct multiboot_tag)) / mmap->entry_size;
}
