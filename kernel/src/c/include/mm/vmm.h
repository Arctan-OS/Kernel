#ifndef VMM_H
#define VMM_H

// Keep track of which virtual memory addresses are allocated
// and which are not. Provide functions for allocating and freeing
// addresses.

#define VMM_RANGE_TABLE_MAX 4096

#include <global.h>

void *vmm_allocate(size_t size);
void *vmm_free(void *address, size_t size);

void initialize_vmm();

#endif