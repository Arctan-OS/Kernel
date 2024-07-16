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
#include <mm/allocator.h>
#include <lib/resource.h>
#include <global.h>
#include <lib/util.h>
#include <lib/atomics.h>

extern struct ARC_DriverDef __DRIVERS0_START[];
extern struct ARC_DriverDef __DRIVERS1_START[];
extern struct ARC_DriverDef __DRIVERS2_START[];
extern struct ARC_DriverDef __DRIVERS3_START[];
extern struct ARC_DriverDef __DRIVERS0_END[];
extern struct ARC_DriverDef __DRIVERS1_END[];
extern struct ARC_DriverDef __DRIVERS2_END[];
extern struct ARC_DriverDef __DRIVERS3_END[];

uint64_t current_id = 0;

struct ARC_Resource *Arc_InitializeResource(int dri_group, uint64_t dri_index, void *args) {
	struct ARC_Resource *resource = (struct ARC_Resource *)Arc_Alloc(sizeof(struct ARC_Resource));

	if (resource == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate memory for resource\n");
		return NULL;
	}

	memset(resource, 0, sizeof(struct ARC_Resource));

	ARC_DEBUG(INFO, "Initializing resource %llu (%d, %lu)\n", current_id, dri_group, dri_index);

	// Initialize resource properties
	resource->id = current_id++; // TODO: Atomize
	resource->dri_group = dri_group;
	resource->dri_index = dri_index;
	Arc_MutexStaticInit(&resource->dri_state_mutex);

	// Fetch and set the appropriate definition
	struct ARC_DriverDef *def = Arc_GetDriverDef(dri_group, dri_index);
	resource->driver = def;

	if (def == NULL) {
		Arc_Free(resource);
		ARC_DEBUG(ERR, "No driver definition found\n");
		return NULL;
	}

	int ret = def->init(resource, args);
	if (ret != 0) {
		ARC_DEBUG(ERR, "Driver init function returned %d\n", ret);
		// TODO: Figure out what to do here
	}

	return resource;
}

int Arc_UninitializeResource(struct ARC_Resource *resource) {
	if (resource == NULL) {
		ARC_DEBUG(ERR, "Resource is NULL, cannot uninitialize\n");
		return 1;
	}

	if (resource->ref_count > 0) {
		ARC_DEBUG(ERR, "Resource %llu is in use!\n", resource->id);
		return 2;
	}

	ARC_DEBUG(INFO, "Uninitializing resource: %llu\n", resource->id);

	// Close all references
	struct ARC_Reference *current_ref = resource->references;
	while (current_ref != NULL) {
		void *next = current_ref->next;

		// TODO: What if we fail to close?
		if (current_ref->signal(ARC_SIGREF_CLOSE, NULL) == 0) {
			resource->ref_count -= 1; // TODO: Atomize
			Arc_Free(current_ref);
		}

		current_ref = next;
	}

	resource->driver->uninit(resource);

	Arc_Free(resource);

	return 0;
}

struct ARC_Reference *Arc_ReferenceResource(struct ARC_Resource *resource) {
	if (resource == NULL) {
		ARC_DEBUG(ERR, "Resource is NULL, cannot reference\n");
		return NULL;
	}

	struct ARC_Reference *ref = (struct ARC_Reference *)Arc_Alloc(sizeof(struct ARC_Reference));

	if (ref == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate reference\n");
		return NULL;
	}

	memset(ref, 0, sizeof(struct ARC_Reference));

	// Set properties of resource
	ref->resource = resource;
	resource->ref_count++; // TODO: Atomize

	// Insert reference
	Arc_MutexLock(&resource->references->branch_mutex);
	ref->next = resource->references;
	if (resource->references != NULL) {
		resource->references->prev = ref;
	}
	resource->references = ref;
	Arc_MutexUnlock(&ref->next->branch_mutex);

	return ref;
}

int Arc_UnreferenceResource(struct ARC_Reference *reference) {
	if (reference == NULL) {
		ARC_DEBUG(ERR, "Resource is NULL, cannot unreference\n");
		return EINVAL;
	}

	struct ARC_Resource *res = reference->resource;
	struct ARC_Reference *next = reference->next;
	struct ARC_Reference *prev = reference->prev;

	// Lock effected nodes
        Arc_MutexLock(&reference->branch_mutex);
	if (prev != NULL) {
		Arc_MutexLock(&prev->branch_mutex);
	}
	if (next != NULL) {
		Arc_MutexLock(&next->branch_mutex);
	}

	res->ref_count--; // TODO: Atomize

	// Update links
	if (prev == NULL) {
		res->references = next;
	} else {
		prev->next = next;
	}

	if (next != NULL) {
		next->prev = prev;
	}

	// Unlock
	Arc_MutexUnlock(&reference->branch_mutex);
	if (reference->prev != NULL) {
		Arc_MutexUnlock(&reference->prev->branch_mutex);
	}
	if (reference->next != NULL) {
		Arc_MutexUnlock(&reference->next->branch_mutex);
	}

        Arc_Free(reference);

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
