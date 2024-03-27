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

#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

#define ARC_DRIVER_IDEN_SUPER 0x5245505553 // "SUPER" little endian

struct ARC_Resource {
	int lock; // TODO: Implement lock system

	struct ARC_Reference *references;
	int ref_count;
	int ref_lock; // Reference specific lock

	/// State managed by driver, owned by resource.
	void *driver_state;
	/// External swappable state.
	void *vfs_state;

	/// Dynamically allocated name.
	char *name;
	/// Driver function group (supplied on init by caller).
	int dri_group;
	/// Specific driver function set (supplied on init by caller).
	int dri_index;
	/// Driver functions.
	struct ARC_DriverDef *driver;
};

struct ARC_Reference {
	// Functions for managing this reference.
	struct ARC_Resource *resource;
	int (*close)();
	struct ARC_Reference *prev;
	struct ARC_Reference *next;
};

struct ARC_DriverDef {
	uint64_t index;
	// Specific
	uint64_t identifer;
	void *driver;
	// Generic
	int (*init)(struct ARC_Resource *res, void *args);
	int (*uninit)(struct ARC_Resource *res);
	int (*open)(struct ARC_Resource *res, char *path, int flags, uint32_t mode);
	int (*write)(void *buffer, size_t size, size_t count, struct ARC_Resource *res);
	int (*read)(void *buffer, size_t size, size_t count, struct ARC_Resource *res);
	int (*close)(struct ARC_Resource *res);
	int (*seek)(struct ARC_Resource *res, long offset, int whence);
	/// Rename the resource.
	int (*rename)(char *newname, struct ARC_Resource *res);
}__attribute__((packed));

struct ARC_SuperDriverDef {
	int (*create)(char *path, uint32_t mode);
	int (*remove)(char *path);
	int (*link)(char *a, char *b);
	/// Rename the file.
	int (*rename)(char *a, char *b);
	int (*stat)(struct ARC_Resource *res, char *filename, struct stat *stat);
}__attribute__((packed));

#define ARC_REGISTER_DRIVER(group, name) \
	static struct ARC_DriverDef __driver__##name __attribute__((used, section(".drivers."#group), aligned(1)))

struct ARC_Resource *Arc_InitializeResource(char *name, int dri_group, uint64_t dri_index, void *args);
int Arc_UninitializeResource(struct ARC_Resource *resource);
struct ARC_Reference *Arc_ReferenceResource(struct ARC_Resource *resource);
int Arc_UnreferenceResource(struct ARC_Reference *reference);
struct ARC_DriverDef *Arc_GetDriverDef(int group, uint64_t index);

#endif
