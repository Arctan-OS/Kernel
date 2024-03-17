/**
 * @file resource.h
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
#ifndef ARC_RESOURCE_H
#define ARC_RESOURCE_H

#include <fs/vfs.h>
#include <stddef.h>
#include <stdint.h>

struct ARC_DriverDef {
	int index;
	int (*open)(struct ARC_VFSNode *file, int flags, uint32_t mode); // FS-specific (filepath = resource->name)
	int (*write)(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file);
	int (*read)(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file);
	int (*close)(struct ARC_VFSNode *file); // FS-specific
	int (*seek)(struct ARC_VFSNode *file, long offset, int whence);

	int (*init)(void *args);
	int (*uninit)(void *args);
};

#define ARC_REGISTER_DRIVER(group, name) \
	static struct ARC_DriverDef __driver__##name __attribute__((used, section(".drivers."#group)))

struct ARC_Reference {
	// Functions for managing this reference.
	struct ARC_Resource *resource;
	int (*close)();
	struct ARC_Reference *prev;
	struct ARC_Reference *next;
};

struct ARC_Resource {
	int lock; // TODO: Implement lock system

	struct ARC_Reference *references;
	int ref_count;
	int ref_lock; // Reference specific lock

	/// Dynamically allocated name.
	char *name;
	/// Driver function group (supplied on init by caller).
	int dri_group;
	/// Specific driver function set (supplied on init by caller).
	int dri_index;
	/// Driver functions.
	struct ARC_DriverDef *driver;
};

int Arc_InitializeResource(char *name, struct ARC_Resource *resource, void *args);
int Arc_UninitializeResource(struct ARC_Resource *resource);

struct ARC_DriverDef *Arc_GetDriverDef(int group, int index);

#endif
