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
#include <global.h>

static const char *root = "/";
static struct ARC_VFSNode vfs_root = { 0 };

int Arc_InitializeVFS() {
//	vfs_root.file = NULL;
//	vfs_root.mount = NULL;
//	vfs_root.next = NULL;
//	vfs_root.children = NULL;
//	vfs_root.type = ARC_VFS_N_MOUNT;

//	vfs_root.name = (char *)root;

	return 0;
}

struct ARC_VFSNode *Arc_MountVFS(struct ARC_VFSNode *mountpoint, char *name, void *disk, void *address, int type) {
//	struct ARC_VFSNode *node = (struct ARC_VFSNode *)Arc_SlabAlloc(sizeof(struct ARC_VFSNode));
//	struct ARC_VFSMount *mount = (struct ARC_VFSMount *)Arc_SlabAlloc(sizeof(struct ARC_VFSMount));

//	if (node == NULL || mount == NULL) {
//		return NULL;
//	}

//	mount->disk = disk;
//	mount->super_address = address;
//	mount->fs_type = type;
//	mount->fs_functions = NULL; // Set this

//	node->next = mountpoint->children;
//	mountpoint->children = node;
//	node->name = name;
//	node->type = ARC_VFS_N_MOUNT;
//	node->children = NULL;
//	node->mount = mount;
//	node->file = NULL;

//	if (mount->fs_functions != NULL && mount->fs_functions->mount != NULL) {
//		(mount->fs_functions->mount)(node, name);
//	} else {
//		ARC_DEBUG(ERR, "No fs_functions or fs_functions->mount for %s (%p : %p : %d)!\n", name, disk, address, type);
//	}

//	return node;

	return NULL;
}

int Arc_UnmountVFS(struct ARC_VFSNode *mount) {


//	if (mount->mount->fs_functions != NULL && mount->mount->fs_functions->unmount != NULL) {
//		(mount->mount->fs_functions->unmount)(mount);
//	} else {
//
//	}

	return 0;
}

struct ARC_VFSNode *Arc_OpenFileVFS(char *filepath, char *perms) {
	// Do open
	return NULL;
}

int Arc_ReadFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
//	(file->file->file_functions->read(buffer, size, count, file));

	return 0;
}

int Arc_WriteFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
//	(file->file->file_functions->write(buffer, size, count, file));

	return 0;
}

int Arc_CloseFileVFS(struct ARC_VFSNode *file) {
	// Do close
	return 0;
}
