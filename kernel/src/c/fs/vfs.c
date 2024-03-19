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
#include <abi-bits/errno.h>
#include <lib/resource.h>
#include <mm/allocator.h>
#include <fs/vfs.h>
#include <global.h>
#include <util.h>

static const char *root = "\0";
static const struct ARC_Resource root_res = { .name = "/" };
static struct ARC_VFSNode vfs_root = { 0 };

int vfs_destroy_node_graph(struct ARC_VFSNode *start, struct ARC_VFSNode *stop) {
	ARC_DEBUG(INFO, "Destroy node graph from %p to %p\n", start, stop);
	while (start != stop) {
		void *tmp = start->parent;

		if (Arc_SlabFree(start) != start) {
			return 1;
		}

		start = (struct ARC_VFSNode *)tmp;
	}

	return 0;
}

size_t vfs_traverse_node_graph(char *filepath, struct ARC_VFSNode **_mount, struct ARC_VFSNode **_node, char **_mount_path) {
	if (filepath == NULL) {
		ARC_DEBUG(ERR, "Given filepath is NULL\n");
		return -1;
	}

	struct ARC_VFSNode *node = &vfs_root;
	*_node = node;

	size_t max = strlen(filepath);

	for (size_t i = 0; i < max; i++) {
		if (*(filepath + i) == '/') {
			int j = 0;
			for (; (*(filepath + i + j + 1) != '/') && (i + j < max); j++);

			char *folder = (filepath + i + 1);

			if (node->type == ARC_VFS_N_MOUNT) {
				// Keep track of mountpoint
				*_mount = node;
				*_mount_path = folder;
			}

			// Interpret dot, dotdot dirs
			if (strncmp(folder, "..", j) == 0) {
				node = node->parent;
				continue;
			} else if (strncmp(folder, ".", j) == 0) {
				continue;
			}

			struct ARC_VFSNode *child = node->children;

			while (child != NULL) {
				if (strncmp(folder, child->name, j) == 0) {
					// Found file, break and move on
					i += j;

					break;
				}

				child = child->next;
			}

			if (child == NULL) {
				// Could not find the file, return the number
				// of characters we managed to parse
				// + 1 to skip the '/'
				return i + 1;
			}

			node = child;
			*_node = node;
		}
	}

	return max;
}

int vfs_create_node_graph(char *filepath, struct ARC_VFSNode **node) {
	if (*node == NULL) {
		ARC_DEBUG(ERR, "Cannot create node graph, given node is NULL\n");
		return 1;
	}

	// Create a node path down to the file
	char *folder = filepath;

	ARC_DEBUG(INFO, "Creating node graph for \"%s\"\n", filepath);

	size_t length = strlen(folder);
	size_t j_ = 0;

	struct ARC_VFSNode *nnode = *node;

	for (size_t j = j_; j < length; j++) {
		if (*(folder + j) == '/' && j - j_ > 0) {
		        create_node_graph:;

			struct ARC_VFSNode *nnode_ = (struct ARC_VFSNode *)Arc_SlabAlloc(sizeof(struct ARC_VFSNode));

			if (nnode_ == NULL) {
				goto epic_node_graph_fail;
			}

			memset(nnode_, 0, sizeof(struct ARC_VFSNode));

			char *name = (char *)Arc_SlabAlloc(j - j_);

			if (name == NULL) {
				goto epic_node_graph_fail;
			}

			memcpy(name, folder + j_, j - j_);

			nnode_->name = name;
			nnode_->parent = nnode;
			nnode_->next = nnode->children;
			nnode->children = nnode_;
			nnode_->type = ARC_VFS_N_DIR;

			nnode = nnode_;

			j_ = j + 1;
		}
	}

	if (j_ < length) {
		// Create the node which represents the file
		goto create_node_graph;
	}

	if (nnode == NULL) {
		goto epic_node_graph_fail;
	}

	*node = nnode;

	return 0;

	epic_node_graph_fail:;
	ARC_DEBUG(ERR, "Failed to create node graph to %s\n", filepath);

	vfs_destroy_node_graph(nnode, *node);

	return 1;
}

int vfs_recursive_free_nodes(struct ARC_VFSNode *head) {
	int count = 0;

	if (head == NULL) {
		return count;
	}

	struct ARC_VFSNode *current_node = head->children;

	while (current_node != NULL) {
		count += vfs_recursive_free_nodes(current_node);
		current_node = current_node->next;
	}

	// Close all references
	struct ARC_Reference *current_ref = head->resource->references;
	while (current_ref != NULL) {
		void *tmp = current_ref->next;

		if (current_ref->close() == 0) {
			head->resource->ref_count -= 1;
		}

		Arc_SlabFree(current_ref);

		current_ref = tmp;
	}

	struct ARC_DriverDef *driver = head->resource->driver;

	if (head->spec != NULL) {
		Arc_SlabFree(head->spec);
	}

	if (head->type == ARC_VFS_N_FILE) {
		Arc_CloseFileVFS(head);
	} else {
		Arc_SlabFree(head->name);
		Arc_SlabFree(head);
	}

	// Uninitialize the driver
	driver->uninit(NULL);

	return count;
}

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
	if (resource->driver == NULL || resource->driver->mount == NULL || resource->driver->mount() != 0) {
		ARC_DEBUG(ERR, "Failed to mount \"%s\"\n", name);
		// TODO: Cleanup
		return NULL;
	}

	ARC_DEBUG(INFO, "Mounted %s on %s (0x%"PRIX64") type %d\n", name, mountpoint->resource->name, resource, type);

	return node;
}


int Arc_UnmountVFS(struct ARC_VFSNode *mount) {
	if (mount->type != ARC_VFS_N_MOUNT) {
		ARC_DEBUG(INFO, "%s is not a mountpoint\n", mount->name);
		return EINVAL;
	}

	mount->resource->driver->unmount();

	vfs_recursive_free_nodes(mount);

	return 0;
}

struct ARC_VFSNode *Arc_OpenFileVFS(char *filepath, int flags, uint32_t mode) {
	ARC_DEBUG(INFO, "Opening %s (%d, %u)\n", filepath, flags, mode);

	// Implement some additional logic for detecting permissions and such

	size_t max = strlen(filepath);

	struct ARC_VFSNode *node = NULL;
	struct ARC_VFSNode *mount = NULL;
	char *mount_path = NULL;

	size_t unknown_start = 0;
	if ((unknown_start = vfs_traverse_node_graph(filepath, &mount, &node, &mount_path)) < max && unknown_start > 0) {
		// Create path
		struct ARC_VFSNode *current = node;

		if (vfs_create_node_graph(filepath + unknown_start, &node) != 0) {
			ARC_DEBUG(ERR, "Failed to create node graph for \"%s\"\n", filepath);
			return NULL;
		}

		// Created path, fill out relevant information

		// Create the filespec
		struct ARC_VFSFile *file_spec = (struct ARC_VFSFile *)Arc_SlabAlloc(sizeof(struct ARC_VFSFile));
		file_spec->node = node;
		node->spec = (void *)file_spec;
		node->type = ARC_VFS_N_FILE;

		// Create new resource
		struct ARC_Resource *res = mount->resource;
		struct ARC_Resource *nres = Arc_InitializeResource(mount_path, res->dri_group, res->dri_index, res->args);

		node->resource = nres;
		int err = 0;
		if (nres->driver == NULL || nres->driver->open == NULL || (err = nres->driver->open(node, flags, mode)) != 0) {
			ARC_DEBUG(ERR, "Failed to open file (%p %p %d)\n", nres->driver, nres->driver->open, err);

			Arc_UninitializeResource(nres);
			Arc_SlabFree(file_spec);

			vfs_destroy_node_graph(node, current);
		}

		ARC_DEBUG(INFO, "Opened file %s\n", filepath);
	}

	if (unknown_start < 0) {
		ARC_DEBUG(INFO, "Failed to traverse node graph (%d)\n", unknown_start);
		return NULL;
	}

	// We can assume we found the file inside of the node tree,
	// and that node is the node that we are looking for
	return node;
}

int Arc_ReadFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	if (buffer == NULL || file == NULL) {
		return -EINVAL;
	}

	if (size == 0 || count == 0) {
		return 0;
	}

	return file->resource->driver->read(buffer, size, count, file);
}

int Arc_WriteFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	if (buffer == NULL || file == NULL) {
		return -EINVAL;
	}

	if (size == 0 || count == 0) {
		return 0;
	}

	return file->resource->driver->write(buffer, size, count, file);
}

int Arc_SeekFileVFS(struct ARC_VFSNode *file, long offset, int whence) {
	if (file == NULL) {
		return 1;
	}

	return file->resource->driver->seek(file, offset, whence);
}

int Arc_CloseFileVFS(struct ARC_VFSNode *file) {
	if (file->type != ARC_VFS_N_FILE) {
		ARC_DEBUG(ERR, "Given file is not a file\n");
		return 1;
	}

	struct ARC_Resource *res = file->resource;

	if (res->ref_count > 0) {
		// This resource is still in use, for now do not
		// close the file
		ARC_DEBUG(ERR, "VFS Node %p is still in use, cannot close\n", file)
		return EBUSY;
	}

	// TODO: Traverse tree upwards, if there are nodes which
	//       are parents of this file and have no other children
	//       than this node, remove them as well. Exception: if the
	//       node is a mountpoint. A mountpoint has to be unmounted

	res->driver->close(file);

	Arc_SlabFree(res->name);
	Arc_SlabFree(res);

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

int Arc_StatFileVFS(char *filepath, struct stat *stat) {
	struct ARC_VFSNode *node = NULL;
	struct ARC_VFSNode *mount = NULL;
	char *mount_path = NULL;

	size_t err = vfs_traverse_node_graph(filepath, &mount, &node, &mount_path);

	if (err < 0) {
		return err;
	}

	if (err == strlen(filepath)) {
		struct ARC_VFSFile *spec = (struct ARC_VFSFile *) node->spec;
		memcpy(stat, &spec->stat, sizeof(struct stat));
		err = 0;
	} else {
		if (mount == NULL) {
			ARC_DEBUG(ERR, "Mount not set\n");
			return -1;
		}

		err = mount->resource->driver->stat(mount, filepath, stat);
	}

	return err;
}
