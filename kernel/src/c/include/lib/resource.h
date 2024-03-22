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

#define ARC_DRIVER_IDEN_SUPER 0x5245505553 // "SUPER" little endian

struct ARC_DriverDef {
	uint64_t index;
	// Specific
	uint64_t identifer;
	void *driver;
	// Generic
	int (*init)(void *args);
	int (*uninit)(void *args);
	int (*open)(struct ARC_VFSNode *file, int flags, uint32_t mode); // FS-specific (filepath = resource->name)
	int (*write)(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file);
	int (*read)(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file);
	int (*close)(struct ARC_VFSNode *file); // FS-specific
	int (*seek)(struct ARC_VFSNode *file, long offset, int whence);
	int (*stat)(struct ARC_VFSNode *mount, char *filename, struct stat *stat);
}__attribute__((packed));

struct ARC_SuperDriverDef {
	int (*create)(char *path, uint32_t mode);
	int (*remove)(char *path);
	int (*rename)(char *a, char *b);
	int (*link)(struct ARC_VFSNode *a, struct ARC_VFSNode *b);
}__attribute__((packed));

#define ARC_REGISTER_DRIVER(group, name) \
	static struct ARC_DriverDef __driver__##name __attribute__((used, section(".drivers."#group), aligned(1)))

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
	/// Additional arguments
	void *args;
	/// Driver functions.
	struct ARC_DriverDef *driver;
};

struct ARC_Resource *Arc_InitializeResource(char *name, int dri_group, uint64_t dri_index, void *args);
int Arc_UninitializeResource(struct ARC_Resource *resource);
struct ARC_Reference *Arc_ReferenceResource(struct ARC_Resource *resource);
int Arc_UnreferenceResource(struct ARC_Reference *reference);
struct ARC_DriverDef *Arc_GetDriverDef(int group, uint64_t index);

#endif
