#ifndef PMM_H
#define PMM_H

// Keep track of which physical memory addresses are allocated
// and which are not. Provide functions for allocating and freeing
// addresses.
#include <global.h>

int initialize_pmm(struct multiboot_tag_mmap *mmap);

#endif