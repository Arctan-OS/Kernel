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
#include "lib/reference.h"
#include <lib/resource.h>
#include <mm/allocator.h>
#include <fs/vfs.h>
#include <global.h>
#include <util.h>

static const char *root = "\0";
static struct ARC_VFSNode vfs_root = { 0 };

int Arc_InitializeVFS() {
	vfs_root.name = (char *)root;
	vfs_root.children = NULL;
	vfs_root.next = NULL;
	vfs_root.parent = NULL;
	vfs_root.ref_count = 0;
	vfs_root.references = NULL;
	vfs_root.resource = NULL;
	vfs_root.type = ARC_VFS_N_ROOT;
	vfs_root.spec = NULL;

	ARC_DEBUG(INFO, "Created VFS root\n");

	return 0;
}

struct ARC_VFSNode *Arc_MountVFS(struct ARC_VFSNode *mountpoint, char *name, struct ARC_Resource *resource, int type) {
	if (mountpoint == NULL) {
		ARC_DEBUG(WARN, "Mountpoint is NULL, using vfs_root\n");
		mountpoint = &vfs_root;
	}

	if (name == NULL || resource == NULL) {
		ARC_DEBUG(ERR, "Name or resource not specified, cannot mount %s/%s\n", mountpoint->name, name);
		return NULL;
	}

	struct ARC_VFSNode *node = (struct ARC_VFSNode *)Arc_SlabAlloc(sizeof(struct ARC_VFSNode));

	node->resource = resource;
	node->type = ARC_VFS_N_MOUNT;
	node->children = NULL;
	node->name = strdup(name);
	node->next = mountpoint->children;
	node->prev = NULL;
	mountpoint->children = node;
	node->resource = resource;
	node->children = NULL;
	node->parent = mountpoint;
	node->ref_count = 0;
	node->references = NULL;

	struct ARC_VFSMount *mount = Arc_SlabAlloc(sizeof(struct ARC_VFSMount));
	mount->fs_type = type;
	mount->node = node;

	node->spec = (void *)mount;

	// Preform additional physical mount operations

	ARC_DEBUG(INFO, "Mounted %s on %s (0x%"PRIX64") type %d\n", name, mountpoint->name, resource, type);

	return node;
}

int Arc_RecursiveFreeNodes(struct ARC_VFSNode *head) {
	int count = 0;

	if (head == NULL) {
		return count;
	}

	struct ARC_VFSNode *current_node = head->children;

	while (current_node != NULL) {
		Arc_RecursiveFreeNodes(current_node);
		current_node = current_node->next;

		count++;
	}

	Arc_SlabFree(head->name);

	struct ARC_Reference *current_ref = head->references;

	while (current_ref != NULL) {
		// "Free" the reference
		// Shut it down, make sure no FS tasks are being
		// done, or nothing thikgs it is able to use this
		// node anymore.
		current_ref = current_ref->next;

		count++;
	}

	// Destroy head->resource

	if (head->spec != NULL) {
		Arc_SlabFree(head->spec);
	}

	Arc_SlabFree(head);

	return 0;
}

int Arc_UnmountVFS(struct ARC_VFSNode *mount) {
	if (mount->type != ARC_VFS_N_MOUNT) {
		ARC_DEBUG(INFO, "%s is not a mountpoint\n", mount->name);
		return -1;
	}

	Arc_RecursiveFreeNodes(mount);

	return 0;
}

struct ARC_VFSNode *Arc_OpenFileVFS(char *filepath, char *perms) {
	(void)filepath;
	(void)perms;

	return NULL;
}

int Arc_ReadFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	(void)buffer;
	(void)size;
	(void)count;
	(void)file;

	return 0;
}

int Arc_WriteFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	(void)buffer;
	(void)size;
	(void)count;
	(void)file;

	return 0;
}

int Arc_CloseFileVFS(struct ARC_VFSNode *file) {
	(void)file;

	return 0;
}
