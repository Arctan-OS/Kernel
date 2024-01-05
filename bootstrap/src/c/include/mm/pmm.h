#ifndef ARC_MM_PMM_H
#define ARC_MM_PMM_H

#include "../multiboot/multiboot2.h"
#include "../mm/freelist.h"

int init_pmm(struct multiboot_tag_mmap *mmap, uintptr_t bootstrap_end, struct ARC_FreelistMeta *physical_mem);

#endif
