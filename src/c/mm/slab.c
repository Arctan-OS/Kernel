/**
 * @file slab.c
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is part of Arctan.
 *
 * Arctan is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
*/
#include <mm/freelist.h>
#include <mm/pmm.h>
#include <mm/slab.h>
#include <global.h>
#include <lib/util.h>

void *slab_alloc(struct ARC_SlabMeta *meta, size_t size) {
	if (size > meta->list_sizes[7]) {
		// Just allocate a contiguous set of pages
		ARC_DEBUG(ERR, "Failed to allocate size %lu\n", size);
		return NULL;
	}

	for (int i = 0; i < 8; i++) {
		if (size <= meta->list_sizes[i]) {
			return freelist_alloc(meta->lists[i]);
		}
	}

	return NULL;
}

void *slab_free(struct ARC_SlabMeta *meta, void *address) {
	for (int i = 0; i < 8; i++) {
		void *base = meta->lists[i]->base;
		void *ceil = meta->lists[i]->ceil;

		if (base <= address && address <= ceil) {
			memset(address, 0, meta->list_sizes[i]);
			return freelist_free(meta->lists[i], address);
		}
	}

	ARC_DEBUG(ERR, "Failed to free %p\n", address);

	return NULL;
}

int slab_expand(struct ARC_SlabMeta *slab, int list, size_t pages) {
	if (slab == NULL || list < 0 || list > 7 || pages == 0) {
		return -1;
	}

	uint64_t base = (uint64_t)pmm_contig_alloc(pages);
	struct ARC_FreelistMeta *meta = init_freelist(base, base + (pages * PAGE_SIZE), slab->list_sizes[list]);

	ARC_DEBUG(INFO, "Expanding SLAB %p (%d) by %lu pages\n", slab, list, pages);

	return link_freelists(slab->lists[list], meta);
}

int init_slab(struct ARC_SlabMeta *meta, size_t init_page_count) {
	ARC_DEBUG(INFO, "Initializing SLAB allocator (%lu)\n", init_page_count);

	size_t object_size = 16;
	for (int i = 0; i < 8; i++) {
		ARC_DEBUG(INFO, "Initializing SLAB (%p) list %d { .size = %lu pages, .obj_size = %lu bytes }\n", meta, i, init_page_count, object_size);

		uint64_t base = (uint64_t)pmm_contig_alloc(init_page_count);
		meta->lists[i] = init_freelist(base, base + (init_page_count * PAGE_SIZE), object_size);
		meta->list_sizes[i] = object_size;

		object_size <<= 1;
	}

	ARC_DEBUG(INFO, "Initialized SLAB allocator\n");

	return 0;
}
