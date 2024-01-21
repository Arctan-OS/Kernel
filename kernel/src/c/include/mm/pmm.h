#ifndef ARC_MM_PMM_H
#define ARC_MM_PMM_H

#include <multiboot/multiboot2.h>
#include <global.h>

void *Arc_AllocPMM();
void *Arc_FreePMM(void *address);
void Arc_InitPMM(struct multiboot_tag_mmap *mmap, uint32_t boot_state);

#endif
