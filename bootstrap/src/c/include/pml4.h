#ifndef ARC_PML4_H
#define ARC_PML4_H

#include "global.h"

// Recursively map one page in
int map(uint64_t *table, uint64_t page_frame, uint64_t page, int overwrite, int level);
// Recursively map all pages in given table
int map_table(uint64_t *table, uint64_t page_frame, uint64_t page, int overwrite, int level);


#endif
