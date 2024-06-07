/**
 * @file resource.c
 *
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
#include <abi-bits/errno.h>
#include <mm/slab.h>
#include <lib/resource.h>
#include <global.h>
#include <util.h>
#include <lib/atomics.h>

extern struct ARC_DriverDef __DRIVERS0_START[];
extern struct ARC_DriverDef __DRIVERS1_START[];
extern struct ARC_DriverDef __DRIVERS2_START[];
extern struct ARC_DriverDef __DRIVERS3_START[];
extern struct ARC_DriverDef __DRIVERS0_END[];
extern struct ARC_DriverDef __DRIVERS1_END[];
extern struct ARC_DriverDef __DRIVERS2_END[];
extern struct ARC_DriverDef __DRIVERS3_END[];

struct ARC_Resource *Arc_InitializeResource(char *name, int dri_group, uint64_t dri_index, void *args) {
	struct ARC_Resource *resource = (struct ARC_Resource *)Arc_SlabAlloc(sizeof(struct ARC_Resource));

	if (resource == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate memory for resource\n");
		return NULL;
	}

	memset(resource, 0, sizeof(struct ARC_Resource));

	ARC_DEBUG(INFO, "Initializing resource \"%s\" (%d, %lu)\n", name, dri_group, dri_index);

	resource->name = strdup(name);
	resource->dri_group = dri_group;
	resource->dri_index = dri_index;

	if (dri_group == 0xAB && dri_index == 0xAB) {
		ARC_DEBUG(INFO, "Initialized place-holder resource\n");
		return resource;
	}

	// Set open, close, read, write, and seek pointers
	// Call driver initialization function from driver table

	struct ARC_DriverDef *def = Arc_GetDriverDef(dri_group, dri_index);

	resource->driver = def;

	if (def != NULL) {
		def->init(resource, args);
	} else {
		ARC_DEBUG(ERR, "Driver has no initialization function\n")
	}

	return resource;
}

int Arc_UninitializeResource(struct ARC_Resource *resource) {
	if (resource == NULL) {
		ARC_DEBUG(ERR, "Resource is NULL, cannot uninitialize\n");
		return 1;
	}

	if (resource->ref_count > 0) {
		ARC_DEBUG(ERR, "Resource %s is in use!\n", resource->name);
		return 2;
	}

	ARC_DEBUG(INFO, "Uninitializing resource: %s\n", resource->name);

	// Call driver uninitialization function from driver table

	struct ARC_Reference *current_ref = resource->references;
	while (current_ref != NULL) {
		void *tmp = current_ref->next;

		// TODO: What if we fail to close?
		if (current_ref->signal != NULL && current_ref->signal(0, NULL) == 0) {
			resource->ref_count -= 1;
			Arc_SlabFree(current_ref);
		}

		current_ref = tmp;
	}

	if (resource->dri_group == 0xAB && resource->dri_index == 0xAB) {
		ARC_DEBUG(INFO, "Uninitialized, place-holder resource\n");
		return 0;
	}

	resource->driver->uninit(resource);

	Arc_SlabFree(resource->name);
	Arc_SlabFree(resource);

	return 0;
}

struct ARC_Reference *Arc_ReferenceResource(struct ARC_Resource *resource) {
	if (resource == NULL) {
		ARC_DEBUG(ERR, "Resource is NULL, cannot reference\n");
		return NULL;
	}

	struct ARC_Reference *ref = (struct ARC_Reference *)Arc_SlabAlloc(sizeof(struct ARC_Reference));


	if (ref == NULL) {
		goto reference_fall;
	}

	memset(ref, 0, sizeof(struct ARC_Reference));

	ref->resource = resource;

	resource->ref_count++; // TODO: Atomize
	Arc_MutexLock(&resource->references->branch_mutex);
	ref->next = resource->references;
	resource->references->prev = ref;
	resource->references = ref;
	Arc_MutexUnlock(&ref->next->branch_mutex);

reference_fall:
	return ref;
}

int Arc_UnreferenceResource(struct ARC_Reference *reference) {
	if (reference == NULL) {
		ARC_DEBUG(ERR, "Resource is NULL, cannot unreference\n");
		return EINVAL;
	}

	struct ARC_Resource *res = reference->resource;

        Arc_MutexLock(&reference->prev->branch_mutex);
        Arc_MutexLock(&reference->branch_mutex);
        Arc_MutexLock(&reference->next->branch_mutex);

	res->ref_count--; // TODO: Atomize

	struct ARC_Reference *next = reference->next;
	struct ARC_Reference *prev = reference->prev;

	if (prev == NULL) {
		res->references = next;
	} else {
		prev->next = next;
	}

	if (next != NULL) {
		next->prev = prev;
	}

        Arc_MutexUnlock(&reference->prev->branch_mutex);
        Arc_MutexUnlock(&reference->branch_mutex);
        Arc_MutexUnlock(&reference->next->branch_mutex);

        Arc_SlabFree(reference);

	return 0;
}

// TODO: This can most definitely be optimzied
struct ARC_DriverDef *Arc_GetDriverDef(int group, uint64_t index) {
	struct ARC_DriverDef *start = NULL;
	struct ARC_DriverDef *end = NULL;

	switch (group) {
	case 0: {
		start = __DRIVERS0_START;
		end = __DRIVERS0_END;
		break;
	}

	case 1: {
		start = __DRIVERS1_START;
		end = __DRIVERS1_END;
		break;
	}

	case 2: {
		start = __DRIVERS2_START;
		end = __DRIVERS2_END;
		break;
	}

	case 3: {
		start = __DRIVERS3_START;
		end = __DRIVERS3_END;
		break;
	}
	}

	if (start == NULL || end == NULL) {
		return NULL;
	}

	for (struct ARC_DriverDef *def = start; def < end; def++) {
		if (def->index == index) {
			return def;
		}
	}

	return NULL;
}
