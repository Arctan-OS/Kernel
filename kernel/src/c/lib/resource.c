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
#include <mm/allocator.h>
#include <lib/resource.h>
#include <global.h>
#include <util.h>

extern struct ARC_DriverDef __DRIVERS0_START[];

int ItWorked() {
	ARC_DEBUG(INFO, "It worked 1\n");

	return 0;
}

int ItWorked_TheSequel() {
	ARC_DEBUG(INFO, "It worked 2\n");

	return 0;
}

ARC_REGISTER_DRIVER(0, name) = (struct ARC_DriverDef) {
	.index = 1,
	.open = ItWorked
};

ARC_REGISTER_DRIVER(1, name2) = (struct ARC_DriverDef) {
	.index = 0,
	.open = ItWorked_TheSequel
};

int Arc_InitializeResource(char *name, struct ARC_Resource *resource) {
	ARC_DEBUG(INFO, "Initializing resource \"%s\"\n", name);

	resource->name = strdup(name);

	// Set open, close, read, write, and seek pointers
	// Call driver initialization function from driver table

	__DRIVERS0_START[0].open(0, 0);

	resource->args = NULL;

	return 0;
}

int Arc_UninitializeResource(struct ARC_Resource *resource) {
	ARC_DEBUG(INFO, "Uninitializing resource: %s\n", resource->name);

	// Call driver uninitialization function from driver table

	if (resource->args == NULL) {
		// args are not NULL, possible memory leak
		// if it is not freed
		return 1;
	}

	Arc_SlabFree(resource->name);
	Arc_SlabFree(resource);

	return 0;
}
