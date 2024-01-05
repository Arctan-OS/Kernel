#include "../include/mm/pmm.h"
#include "../include/global.h"

// Return 0: success
int init_pmm(struct multiboot_tag_mmap *mmap, uintptr_t bootstrap_end, struct ARC_FreelistMeta *physical_mem) {
	ARC_DEBUG(INFO, "Initializing PMM\n")

	int entries = (mmap->size - sizeof(struct multiboot_tag_mmap)) / mmap->entry_size;

	struct ARC_FreelistMeta a = { 0 };

	for (int i = 0; i < entries; i++) {
		struct multiboot_mmap_entry entry = mmap->entries[i];

		if ((entry.addr < bootstrap_end && entry.addr + entry.len < bootstrap_end) || entry.type != MULTIBOOT_MEMORY_AVAILABLE) {
			// Entry not suitable for a freelist table
			continue;
		}

		// This entry either contains the bootstrap_end or is located after
		ARC_DEBUG(INFO, "Entry %d suitable for freelist\n", i)

		if ((uint32_t)(entry.addr >> 32) > 0) {
			ARC_DEBUG(INFO, "Entry %d is above 32-bit address range, ignoring\n")
			continue;
		}

		void *base = (void *)(entry.addr);
		void *ciel = (void *)(entry.addr + entry.len);

		if (entry.addr < bootstrap_end && entry.addr + entry.len > bootstrap_end) {
			// bootstrap_end contained is in this entry
			base = (void *)ALIGN(bootstrap_end, 0x1000);
		}

		ARC_DEBUG(INFO, "Initializing freelist %p -> %p\n", base, ciel)

		if (a.base == 0) {
			Arc_InitializeFreelist(base, ciel, 0x1000, &a);
		} else {
			struct ARC_FreelistMeta b = { 0 };
			struct ARC_FreelistMeta c = { 0 };
			Arc_InitializeFreelist(base, ciel, 0x1000, &a);
			int err = Arc_ListLink(&a, &b, &c);

			if (err != 0) {
				ARC_DEBUG(ERR, "Failed to link lists A and B\n")
				// TODO: Do something, currently we hope this does not happen.
				//       Idealy pick the larger list and copy it into A.
			}
		}
	}

	ARC_DEBUG(INFO, "Initialized PMM\n")

	void *allocation = (void *)Arc_ListAlloc(&a);
	ARC_DEBUG(INFO, "%p\n", allocation);
	Arc_ListAlloc(&a);
	void *allocation_b = (void *)Arc_ListAlloc(&a);
	ARC_DEBUG(INFO, "%p\n", allocation_b);
	Arc_ListFree(&a, allocation);
	void *allocation_c = (void *)Arc_ListAlloc(&a);
	ARC_DEBUG(INFO, "%p\n", allocation_c);

	return 0;
}
