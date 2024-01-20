#include "global.h"
#include "mm/freelist.h"
#include <mm/pmm.h>

struct ARC_FreelistMeta arc_physical_mem = { 0 };

void Arc_InitPMM(uint32_t boot_state) {
	uint32_t *state32 = (uint32_t *)(boot_state + ARC_HHDM_VADDR + 4);

	arc_physical_mem.head = (void *)(*state32 + ARC_HHDM_VADDR);
	arc_physical_mem.base = (void *)(*(state32 + 1) + ARC_HHDM_VADDR);

	arc_physical_mem.ciel = (struct ARC_FreelistNode *)(*(state32 + 2) + ARC_HHDM_VADDR);

	arc_physical_mem.object_size = *(state32 + 3);

	ARC_DEBUG(INFO, "%p %p %p %d\n", arc_physical_mem.base, arc_physical_mem.ciel, arc_physical_mem.head, arc_physical_mem.object_size);
}
