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
#include <util.h>

extern struct ARC_DriverDef __DRIVERS0_START[];
extern struct ARC_DriverDef __DRIVERS1_START[];
extern struct ARC_DriverDef __DRIVERS2_START[];
extern struct ARC_DriverDef __DRIVERS3_START[];
extern struct ARC_DriverDef __DRIVERS0_END[];
extern struct ARC_DriverDef __DRIVERS1_END[];
extern struct ARC_DriverDef __DRIVERS2_END[];
extern struct ARC_DriverDef __DRIVERS3_END[];

struct ARC_Resource *Arc_InitializeResource(char *name, int dri_group, uint64_t dri_index, void *args) {
	ARC_DEBUG(INFO, "Initializing resource \"%s\" (%d, %d)\n", name, dri_group, dri_index);

	struct ARC_Resource *resource = (struct ARC_Resource *)Arc_SlabAlloc(sizeof(struct ARC_Resource));

	if (resource == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate memory for resource\n");
		return NULL;
	}

	memset(resource, 0, sizeof(struct ARC_Resource));

	resource->name = strdup(name);

	// Set open, close, read, write, and seek pointers
	// Call driver initialization function from driver table

	struct ARC_DriverDef *def = Arc_GetDriverDef(dri_group, dri_index);

	resource->driver = def;
	resource->dri_group = dri_group;
	resource->dri_index = dri_index;

	if (def != NULL) {
		def->init(resource, args);
	} else {
		ARC_DEBUG(INFO, "Driver has no initialization function\n")
	}

	return resource;
}

int Arc_UninitializeResource(struct ARC_Resource *resource) {
	ARC_DEBUG(INFO, "Uninitializing resource: %s\n", resource->name);

	// Call driver uninitialization function from driver table

	struct ARC_Reference *current_ref = resource->references;
	while (current_ref != NULL) {
		void *tmp = current_ref->next;

		// TODO: What if we fail to close?
		if (current_ref->close != NULL && current_ref->close() == 0) {
			resource->ref_count -= 1;
			Arc_SlabFree(current_ref);
		}

		current_ref = tmp;
	}

	resource->driver->uninit(resource);

	Arc_SlabFree(resource->name);
	Arc_SlabFree(resource);

	return 0;
}

struct ARC_Reference *Arc_ReferenceResource(struct ARC_Resource *resource) {
	// Lock reference lock

	struct ARC_Reference *ref = (struct ARC_Reference *)Arc_SlabAlloc(sizeof(struct ARC_Reference));

	if (ref == NULL) {
		goto reference_fall;
	}

	memset(ref, 0, sizeof(struct ARC_Reference));

	ref->resource = resource;

	resource->ref_count += 1;
	ref->next = resource->references;
	resource->references->prev = ref;
	resource->references = ref;

reference_fall:
	// Unlock lock
	return ref;
}

int Arc_UnreferenceResource(struct ARC_Reference *reference) {
	if (reference == NULL) {
		return EINVAL;
	}

	// Lock reference lock

	struct ARC_Resource *res = reference->resource;

	res->ref_count -= 1;

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

	// Unlock lock

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
