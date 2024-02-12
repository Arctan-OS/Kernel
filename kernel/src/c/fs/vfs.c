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

int Arc_InitializeVFS(struct ARC_VFSNode *vfs) {
	vfs->disk = NULL;
	vfs->address = NULL;
	vfs->next = NULL;
	vfs->children = NULL;
	vfs->fs_type = ARC_VFS_NULL;
	vfs->type = ARC_VFS_N_MOUNT;

	vfs->name = (char *)root;

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

// TODO: Implement abstracted functions like read, write,
//       open, and close files.
