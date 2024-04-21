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
#include <abi-bits/stat.h>
#include <lib/atomics.h>
#include <abi-bits/errno.h>
#include <lib/resource.h>
#include <mm/allocator.h>
#include <fs/vfs.h>
#include <global.h>
#include <util.h>

static const char *root = "\0";
static const struct ARC_Resource root_res = { .name = "/" };
static struct ARC_VFSNode vfs_root = { 0 };

struct vfs_traverse_info {
	struct ARC_VFSNode *start;
	/// The sought after nodes.
	struct ARC_VFSNode *node;
	/// Node that node is mounted on.
	struct ARC_VFSNode *mount;
	/// Path to node from the mountpoint.
	char *mountpath;
	/// Mode to create new nodes with.
	uint32_t mode;
	/// Type of node to create.
	int type;
	/// Creation level (0: no creation, 1: create node graph, 2: create node graph and physical files)
#define VFS_NO_CREAT 0
#define VFS_GR_CREAT 1
#define VFS_FS_CREAT 2
	int create_level;
};

int vfs_stat2type(struct stat stat) {
	switch (stat.st_mode & S_IFMT) {
		case S_IFDIR: {
			return ARC_VFS_N_DIR;
		}

		case S_IFLNK: {
			return ARC_VFS_N_LINK;
		}

		case S_IFREG: {
			return ARC_VFS_N_FILE;
		}

		default: {
			return ARC_VFS_NULL;
		}
	}
}

int vfs_create_node() {
	return 0;
}

int create_link(struct ARC_VFSNode *node, int link_depth) {
	return 0;
}

int vfs_traverse(char *filepath, struct vfs_traverse_info *info, int link_depth) {
	if (filepath == NULL || info == NULL || info->start == NULL) {
		return EINVAL;
	}

	size_t max = strlen(filepath);

	struct ARC_VFSNode *node = info->start;
	node->ref_count++; // TODO: Atomize

	int64_t tid = Arc_QLock(&node->branch_lock);
	if (tid < 0 && tid != -1) {
		 ARC_DEBUG(ERR, "Lock error!\n");
	}

	size_t last_div = 0;
	for (size_t i = 0; i < max; i++) {
		if (filepath[i] != '/' && i != max -1) {
			continue;
		}

		size_t component_length = i - last_div;

		if (component_length <= 0) {
			continue;
		}

		char *component = (char *)(filepath + last_div);

		if (*component == '/') {
			component++;
		}

		last_div = i;
		if (node->type == ARC_VFS_N_MOUNT) {
			info->mount = node;
			info->mountpath = component;
		}

		Arc_QYield(&node->branch_lock, tid);

		if (strncmp(component, "..", component_length) == 0) {
			// .. dir, go up one
			node = node->parent == NULL ? node : node->parent;
			continue;
		} else if (strncmp(component, ".", component_length) == 0) {
			// . dir, skip
			continue;
		}

		struct ARC_VFSNode *child = node->children;
		while (child != NULL) {
			if (strncmp(component, child->name, 0) == 0) {
				break;
			}

			child = child->next;
		}

		if (child == NULL) {
			if (info->create_level == VFS_NO_CREAT) {
				// No creation desired by caller
				ARC_DEBUG(ERR, "VFS_NO_CREAT specified\n");
				return i;
			}

			// Next node down does not exist in node graph
			// Create it
			struct ARC_VFSNode *new = (struct ARC_VFSNode *)Arc_SlabAlloc(sizeof(struct ARC_VFSNode));

			if (new == NULL) {
				ARC_DEBUG(ERR, "Cannot allocate next node\n");
			}

			memset(new, 0, sizeof(struct ARC_VFSNode));

			new->name = strndup(component, component_length);

			new->mount = info->mount->mount;
			new->next = node->children;

			new->type = ARC_VFS_N_DIR;
			if (i == max - 1) {
				// This is the last component, we need to create
				// it with the specified type
				new->type = info->type;
			}

			if (node->children != NULL) {
				node->children->prev = new;
			}
			node->children = new;
			child = new;

			if (info->mount != NULL) {
				struct ARC_Resource *res = info->mount->resource;
				struct ARC_SuperDriverDef *def = (struct ARC_SuperDriverDef *)res->driver->driver;

				if (def->stat(res, info->mountpath, &new->stat) != 0) {
					ARC_DEBUG(ERR, "Failed to stat %s\n", info->mountpath);
					if (info->create_level != VFS_FS_CREAT) {
						ARC_DEBUG(ERR, "VFS_FS_CREAT not allowed\n");

						Arc_QUnlock(&node->branch_lock);

						return i;
					}

					// Since all mountpaths exist in VFS, we do not need to be
					// concerened with the situation where a path such as:
					// /path/to/thing/that/exists.txt where path and
					// thing are mountpoints causes this case to break. As
					// we can be sure that we will not try to create
					// /to/thing/that/exists.txt (as /path/to/thing is present)
					if (def->create(info->mountpath, info->mode, info->type) != 0) {
						ARC_DEBUG(ERR, "VFS_FS_CREAT failed\n");
					}
				} else {
					new->type = vfs_stat2type(new->stat);
				}
			}
		}

		tid = Arc_QLock(&child->branch_lock);
		if (tid < 0 && tid != -1) {
			ARC_DEBUG(ERR, "Lock error!\n");
		}
		Arc_QUnlock(&node->branch_lock);

		node->ref_count--; // TODO: Atomize
		node = child;
		node->ref_count++; // TODO: Atomize
	}

	return 0;
}

int Arc_InitializeVFS() {
	vfs_root.name = (char *)root;
	vfs_root.resource = (struct ARC_Resource *)&root_res;
	vfs_root.type = ARC_VFS_N_ROOT;

	Arc_QLockStaticInit(&vfs_root.branch_lock);
	Arc_MutexStaticInit(&vfs_root.property_lock);

	ARC_DEBUG(INFO, "Created VFS root\n");

	return 0;
}

int Arc_MountVFS(char *mountpoint, struct ARC_Resource *resource, int fs_type) {
	if (mountpoint == NULL || resource == NULL || fs_type == ARC_VFS_NULL) {
		return EINVAL;
	}

	struct vfs_traverse_info info = { 0 };
	if (*mountpoint == '/') {
		info.start = &vfs_root;
	} else {
		// info.start = current_working_directory();
	}
	info.create_level = VFS_GR_CREAT;

	if (vfs_traverse(mountpoint, &info, 0) != 0) {
		ARC_DEBUG(ERR, "Failed to traverse graph\n");
		return -1;
	}

	struct ARC_VFSNode *mount = info.node;

	if (mount->type != ARC_VFS_N_DIR) {
		ARC_DEBUG(ERR, "%s is not a directory (or already mounted)\n", mountpoint);
		return -1;
	}

	Arc_MutexLock(&mount->property_lock);

	mount->type = ARC_VFS_N_MOUNT;
	mount->resource = resource;

	Arc_MutexUnlock(&mount->property_lock);

	return 0;
}

int Arc_UnmountVFS(struct ARC_VFSNode *mount) {
	if (mount == NULL || mount->type != ARC_VFS_N_MOUNT) {
		return EINVAL;
	}

	int64_t tid = Arc_QLock(&mount->branch_lock);
	if (tid < 0 && tid != -1) {
		ARC_DEBUG(ERR, "Lock error!\n");
		return -1;
	}
	Arc_QYield(&mount->branch_lock, tid);

	if (Arc_MutexLock(&mount->property_lock) != 0) {
		ARC_DEBUG(ERR, "Lock error!\n");
		return -1;
	}



	return 0;
}

// TODO: Don't signal return type through sign of int link_depth
// link_depth >> 31 = 0: void *ret = struct ARC_VFSFile *
// link_depth >> 31 = 1: void *ret = struct ARC_VFSNode *
int Arc_OpenVFS(char *path, int flags, uint32_t mode, int link_depth, void **ret) {
	(void)flags;

	if (path == NULL) {
		return EINVAL;
	}

	struct vfs_traverse_info info = { 0 };
	if (*path == '/') {
		info.start = &vfs_root;
	} else {
		// info.start = current_working_directory();
	}

	info.create_level = 1;

	if (vfs_traverse(path, &info, link_depth) != 0) {
		// Error
	}

	struct ARC_VFSNode *node = info.node;

	if (((link_depth >> 31) & 1) == 1) {
		*ret = (void *)node;
		return 0;
	}

	// TODO: Check perms

	struct ARC_VFSFile *desc = (struct ARC_VFSFile *)Arc_SlabAlloc(sizeof(struct ARC_VFSFile));

	if (desc == NULL) {
		return ENOMEM;
	}

	memset(desc, 0, sizeof(struct ARC_VFSFile));

	desc->reference = Arc_ReferenceResource(node->resource);
	desc->mode = mode;

	return 0;
}

int Arc_CreateVFS(char *path, uint32_t mode, int type) {
	if (path == NULL) {
		return EINVAL;
	}

	struct vfs_traverse_info info = { .mode = mode, .type = type, .create_level = VFS_FS_CREAT };
	if (*path == '/') {
		info.start = &vfs_root;
	} else {
		// info.start = get_current_directory();
	}

	vfs_traverse(path, &info, 0);

	Arc_MutexLock(&info.node->property_lock);

	Arc_MutexLock(&info.node->property_lock);

	return 0;
}

#undef VFS_NO_CREAT
#undef VFS_GR_CREAT
#undef VFS_FS_CREAT
