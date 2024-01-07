#ifndef ARC_MM_VMM_H
#define ARC_MM_VMM_H

#include "../global.h"

uint64_t *map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr, int overwrite);

#endif
