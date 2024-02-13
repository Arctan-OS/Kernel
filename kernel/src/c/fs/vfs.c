/**
 * @file initramfs.h
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
 * Abstract virtual file system driver. Is able to create and delete virtual
 * file systems for caching files on disk.
*/
#include "mm/allocator.h"
#include <fs/vfs.h>

static const char *root = "/";
static struct ARC_VFSNode vfs_root = { 0 };

int Arc_InitializeVFS() {
	vfs_root.disk = NULL;
	vfs_root.address = NULL;
	vfs_root.next = NULL;
	vfs_root.children = NULL;
	vfs_root.fs_type = ARC_VFS_NULL;
	vfs_root.type = ARC_VFS_N_MOUNT;

	vfs_root.name = (char *)root;

	return 0;
}

struct ARC_VFSNode *Arc_MountVFS(struct ARC_VFSNode *mountpoint, char *name, void *disk, void *address, int type) {
	struct ARC_VFSNode *mount = (struct ARC_VFSNode *)Arc_SlabAlloc(sizeof(struct ARC_VFSNode));

	if (mount == NULL) {
		return NULL;
	}

	mount->next = mountpoint->children;
	mountpoint->children = mount;

	mount->name = name;
	mount->disk = disk;
	mount->address = address;
	mount->fs_type = type;
	mount->type = ARC_VFS_N_MOUNT;
	mount->children = NULL;

	return mount;
}

struct ARC_VFSNode *Arc_OpenFileVFS(char *filepath, char *perms) {
	return NULL;
}

int Arc_ReadFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	return 0;
}

int Arc_WriteFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	return 0;
}

int Arc_CloseFileVFS(struct ARC_VFSNode *file) {
	return 0;
}
