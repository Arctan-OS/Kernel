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
#include <lib/resource.h>
#include <mm/allocator.h>
#include <fs/vfs.h>
#include <global.h>
#include <util.h>

static const char *root = "\0";
static const struct ARC_Resource root_res = { .name = "/" };
static struct ARC_VFSNode vfs_root = { 0 };

int Arc_InitializeVFS() {
	vfs_root.name = (char *)root;
	vfs_root.children = NULL;
	vfs_root.next = NULL;
	vfs_root.parent = NULL;
	vfs_root.resource = (struct ARC_Resource *)&root_res;
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
		ARC_DEBUG(ERR, "Name or resource not specified, cannot mount %s/%s\n", mountpoint->resource->name, name);
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

	struct ARC_VFSMount *mount = Arc_SlabAlloc(sizeof(struct ARC_VFSMount));
	mount->fs_type = type;
	mount->node = node;

	node->spec = (void *)mount;

	// Preform additional physical mount operations

	ARC_DEBUG(INFO, "Mounted %s on %s (0x%"PRIX64") type %d\n", name, mountpoint->resource->name, resource, type);

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

	// TODO: Close references

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

struct ARC_VFSNode *Arc_OpenFileVFS(char *filepath, int flags, uint32_t mode) {
	(void)flags;
	(void)mode;

	ARC_DEBUG(INFO, "Opening %s (%d, %u)\n", filepath, flags, mode);

	// Implement some additional logic for detecting permissions and such

	size_t max = strlen(filepath);
	struct ARC_VFSNode *current = &vfs_root;

	for (size_t i = 0; i < max; i++) {
		if (*(filepath + i) == '/') {
			int j = 0;
			for (; (*(filepath + i + j + 1) != '/') && (i + j < max); j++);

			char *folder = (filepath + i + 1);

			// Interpret dot, dotdot dirs
			if (strncmp(folder, "..", j) == 0) {
				current = current->parent;
				continue;
			} else if (strncmp(folder, ".", j) == 0) {
				continue;
			}

			struct ARC_VFSNode *child = current->children;

			while (child != NULL) {
				if (strncmp(folder, child->name, j) == 0) {
					// Found file, break and move on
					i += j;

					break;
				}

				child = child->next;
			}

			if (child == NULL) {
				// We could not find the file / resource in the existing
				// node tree, ask the relevant FS driver to find it for us
				ARC_DEBUG(INFO, "Stuck on %s\n", folder);

				// Once the FS driver has found our file, we can return it
				// here. Name should be the disk path the FS driver looking
				// for

				for (;;);
			}

			current = child;
		}
	}

	// We can assume we found the file inside of the node tree,
	// and that current is the node that we are looking for
	return current;
}

int Arc_ReadFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	if (buffer == NULL || file == NULL) {
		return 1;
	}

	if (size == 0 || count == 0) {
		return 0;
	}

	return file->resource->driver->read(buffer, size, count);
}

int Arc_WriteFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	if (buffer == NULL || file == NULL) {
		return 1;
	}

	if (size == 0 || count == 0) {
		return 0;
	}

	return file->resource->driver->write(buffer, size, count);
}

int Arc_SeekFileVFS(struct ARC_VFSNode *file, long offset, int whence) {
	if (file == NULL) {
		return 1;
	}

	return file->resource->driver->seek(offset, whence);
}

int Arc_CloseFileVFS(struct ARC_VFSNode *file) {
	(void)file;

	struct ARC_Resource *res = file->resource;

	if (res->reference->ref_count > 0) {
		// This resource is still in use, for now do not
		// close the file
		ARC_DEBUG(INFO, "VFS Node %p is still in use, cannot close\n", file)
		return 1;
	}

	res->driver->close();

	Arc_SlabFree(file->spec);
	Arc_SlabFree(file->name);

	struct ARC_VFSNode *prev = file->prev;
	struct ARC_VFSNode *next = file->next;

	if (prev == NULL) {
		file->parent->children = next;
	} else {
		prev->next = next;

		if (next != NULL) {
			next->prev = prev;
		}
	}

	Arc_SlabFree(file);

	return 0;
}
