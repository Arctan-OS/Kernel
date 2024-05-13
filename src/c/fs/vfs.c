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

// TODO: Implement error cases for all functions.

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
	/// Creation level
#define VFS_NO_CREAT 0x1 // Don't create anything, just traverse.
#define VFS_GR_CREAT 0x2 // Create node graph from physical file system.
#define VFS_FS_CREAT 0x4 // Create node graph and physical file system from path.
#define VFS_NOLCMP   0x8 // Create node graph except for last component in path.
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

int vfs_delete_node_recurse(struct ARC_VFSNode *node) {
	if (node == NULL || node->ref_count > 0 || node->is_open != 0) {
		return 1;
	}

	int err = 0;

	struct ARC_VFSNode *child = node->children;
	while (child != NULL) {
		err += vfs_delete_node_recurse(child);
		child = child->next;
	}

	Arc_SlabFree(node->name);
	Arc_UninitializeResource(node->resource);
	Arc_SlabFree(node);

	return err;
}

int vfs_delete_node(struct ARC_VFSNode *node, bool recurse) {
	if (node == NULL || node->ref_count > 0 || node->is_open != 0) {
		return 1;
	}

	struct ARC_VFSNode *child = node->children;

	int err = 0;

	while (child != NULL && recurse == 1) {
		err = vfs_delete_node_recurse(child);
		child = child->next;
	}

	Arc_SlabFree(node->name);
	Arc_UninitializeResource(node->resource);

	if (node->prev == NULL) {
		node->parent->children = node->next;
	} else {
		node->prev->next = node->next;
	}

	if (node->next != NULL) {
		node->next->prev = node->prev;
	}

	Arc_SlabFree(node);

	return err;
}

// TODO: This prune function is called when closing or removing a file.
//       First, it needs testing like the remove and close functions.
//       Second, this may become costly in terms of speed, as quite a
//       big lock is grabbed (locking the topmost node).
int vfs_bottom_up_prune(struct ARC_VFSNode *bottom, struct ARC_VFSNode *top) {
	struct ARC_VFSNode *current = bottom;
	int freed = 0;

	if (Arc_QLock(&top->branch_lock) != 0) {
		return -1;
	}
	Arc_QYield(&top->branch_lock);

	// We have the topmost node held
	// Delete unused directories
	do {
		void *tmp = current->parent;

		if (current->children == NULL && current->ref_count == 0) {
			vfs_delete_node(current, 0);
			freed++;
		}

		current = tmp;
	} while (current != top && current != NULL && current->type != ARC_VFS_N_MOUNT);

	Arc_QUnlock(&top->branch_lock);

	return freed;
}

int vfs_open_link(struct ARC_VFSNode *node, struct ARC_VFSNode **ret, int link_depth) {
	(void)node;
	(void)ret;
	(void)link_depth;
	// TODO: Open and read contents of node, which is a link,
	//       vfs_traverse to the path this node contains.
	//       Once link_depth == 0, return whatever node was
	//       found, even if it is a link.
	//
	//       Error Cases:
	//          Cyclic Links - If any given node in the list of nodes
	//                         we have found with this function points
	//                         back to another node in this functio, we have
	//                         a cyclic link. Not sure how to resolve this
	//                         besides just erroring out and saying "could
	//                         not resolve link"
	//          vfs_traverse - We failed to traverse the node graph, so we
	//                         "could not resolve link"
	//
	return 0;
}

int vfs_traverse(char *filepath, struct vfs_traverse_info *info, int link_depth) {
	if (filepath == NULL || info == NULL || info->start == NULL) {
		return EINVAL;
	}

	ARC_DEBUG(INFO, "Traversing %s\n", filepath);

	size_t max = strlen(filepath);

	struct ARC_VFSNode *node = info->start;
	node->ref_count++; // TODO: Atomize
	info->node = node;

	if (Arc_QLock(&node->branch_lock) != 0) {
		 ARC_DEBUG(ERR, "Lock error!\n");
		 return -1;
	}

	size_t last_div = 0;
	for (size_t i = 0; i < max; i++) {
		if (filepath[i] != '/' && i != max - 1) {
			// Code executes only if:
			//     Current character is not a '/'
			//     Or we are not at the end of a line
			continue;
		}

		if (i == max - 1 && (info->create_level & VFS_NOLCMP) != 0) {
			// If we are at the last component, and VFS_NOLCMP is set
			// then continue
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

		Arc_QYield(&node->branch_lock);

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
			if (strncmp(component, child->name, component_length) == 0) {
				break;
			}

			child = child->next;
		}

		if (child == NULL) {
			if ((info->create_level & VFS_NO_CREAT) == 1) {
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

			new->type = ARC_VFS_N_DIR;
			if (i == max - 1) {
				// This is the last component, we need to create
				// it with the specified type
				new->type = info->type;
			}

			new->next = node->children;
			if (node->children != NULL) {
				node->children->prev = new;
			}
			node->children = new;
			new->parent = node;
			child = new;

			char *use_path = info->mountpath == NULL ? filepath : info->mountpath;
			int length = i - ((uintptr_t)use_path - (uintptr_t)filepath) + 1;
			char *stat_path = strndup(use_path, length);

			if (info->mount != NULL) {
				ARC_DEBUG(INFO, "Mount is present, statting %s\n", stat_path);

				struct ARC_Resource *res = info->mount->resource;
				struct ARC_SuperDriverDef *def = (struct ARC_SuperDriverDef *)res->driver->driver;

				if (def->stat(res, stat_path, &new->stat) != 0) {
					ARC_DEBUG(ERR, "Failed to stat %s\n", stat_path);
					if ((info->create_level & VFS_FS_CREAT) != 1) {
						ARC_DEBUG(ERR, "VFS_FS_CREAT not allowed\n");

						Arc_SlabFree(stat_path);
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
					if (vfs_stat2type(new->stat) != ARC_VFS_NULL) {
						new->type = vfs_stat2type(new->stat);
					}
				}
			}

                        // Initialzie resource if needed
			if (new->type != ARC_VFS_N_DIR && new->resource == NULL) {
				ARC_DEBUG(INFO, "Node has no resource, creating one\n");
				struct ARC_Resource *res = info->mount->resource;
				Arc_MutexLock(&res->dri_state_mutex);

				int group = res->dri_group;
				uint64_t index = res->dri_index + 1;

				if (new->type == ARC_VFS_N_LINK) {
					group = 0xAB;
					index = 0xAB;
				}

				struct ARC_Resource *nres = Arc_InitializeResource(stat_path, group, index, res->driver_state);
				Arc_MutexUnlock(&res->dri_state_mutex);
				new->resource = nres;

				if (nres == NULL) {
					ARC_DEBUG(ERR, "Failed to create resource\n");
					Arc_SlabFree(stat_path);
					return -1;
				}
			}

			Arc_SlabFree(stat_path);

			ARC_DEBUG(INFO, "Created new node \"%s\" (%p)\n", new->name, new);

			if (new->type == ARC_VFS_N_LINK) {
				// TODO: Resolve link
			}
		}

		if (Arc_QLock(&child->branch_lock) != 0) {
			ARC_DEBUG(ERR, "Lock error!\n");
			goto cleanup;
		}
		Arc_QUnlock(&node->branch_lock);

		child->ref_count++; // TODO: Atomize
		node->ref_count--; // TODO: Atomize
		node = child;
		info->node = node;
	}

	return 0;

cleanup:;
	ARC_DEBUG(WARN, "Definitely cleaning up\n");
	return -1;
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
		ARC_DEBUG(ERR, "Invalid arguments (%p, %p, %d)\n", mountpoint, resource, fs_type);
		return EINVAL;
	}

	ARC_DEBUG(INFO, "Mounting %p on %s (%d)\n", resource, mountpoint, fs_type);

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

	if (mount == NULL) {
		ARC_DEBUG(ERR, "Mount is NULL\n");
		return -1;
	}

	if (mount->type != ARC_VFS_N_DIR) {
		ARC_DEBUG(ERR, "%s is not a directory (or already mounted)\n", mountpoint);
		return -1;
	}

	Arc_MutexLock(&mount->property_lock);

	mount->type = ARC_VFS_N_MOUNT;
	mount->resource = resource;

	Arc_MutexUnlock(&mount->property_lock);

	ARC_DEBUG(INFO, "Successfully mounted resource %p at %s (%d, %p)\n", resource, mountpoint, fs_type, mount);

	return 0;
}

int Arc_UnmountVFS(struct ARC_VFSNode *mount) {
	if (mount == NULL || mount->type != ARC_VFS_N_MOUNT) {
		ARC_DEBUG(ERR, "Given mount is NULL or not a mount\n");
		return EINVAL;
	}

	ARC_DEBUG(INFO, "Unmounting %p\n", mount);

	if (Arc_MutexLock(&mount->property_lock) != 0) {
		ARC_DEBUG(ERR, "Lock error!\n");
		return -1;
	}

	// Mark node for unmounting

	if (Arc_QLock(&mount->branch_lock) != 0) {
		ARC_DEBUG(ERR, "Lock error!\n");
		return -1;
	}
	Arc_QYield(&mount->branch_lock);

	if (Arc_QFreeze(&mount->branch_lock) != 0) {
		ARC_DEBUG(ERR, "Could not freeze lock!\n");
		return -1;
	}

	// TODO: Synchronize
	// TODO: Destroy nodes

	ARC_DEBUG(INFO, "Successfully unmount %p\n", mount);

	return 0;
}

int Arc_OpenVFS(char *path, int flags, uint32_t mode, int link_depth, void **ret) {
	(void)flags;

	if (path == NULL) {
		return EINVAL;
	}

	ARC_DEBUG(INFO, "Opening file %s (%d %d) to a depth of %d, returning to %p\n", path, flags, mode, link_depth, ret);

	// Find file in node graph, create node graph if needed, do not create the file
	struct vfs_traverse_info info = { .create_level = VFS_GR_CREAT };
	if (*path == '/') {
		info.start = &vfs_root;
	} else {
		// info.start = current_working_directory();
	}

	int info_ret = vfs_traverse(path, &info, link_depth);
	if (info_ret != 0) {
		ARC_DEBUG(ERR, "Failed to traverse node graph (%d)\n", info_ret);
		return -1;
	}

	struct ARC_VFSNode *node = info.node;

	if (node == NULL) {
		ARC_DEBUG(ERR, "Discovered node is NULL\n");
	}

	ARC_DEBUG(INFO, "Found node %p\n", node);

	// Create file descriptor
	struct ARC_File *desc = (struct ARC_File *)Arc_SlabAlloc(sizeof(struct ARC_File));
	if (desc == NULL) {
		return ENOMEM;
	}
	memset(desc, 0, sizeof(struct ARC_File));
	*ret = desc;

	desc->mode = mode;
	desc->node = node;

	ARC_DEBUG(INFO, "Created file descriptor %p\n", desc);

	Arc_MutexLock(&node->property_lock);
	if (node->is_open == 0) {
		struct ARC_VFSNode *tmp = node;

		if (node->type == ARC_VFS_N_LINK) {
			node = node->link;
		}

		Arc_MutexLock(&node->property_lock);
		// NOTE: node->resource->name will always correspond to the path
		//       to the node within the filesystem. This name is changed
		//       when renaming, the path is absolute relative to the
		//       mount
		struct ARC_Resource *res = node->resource;
		if (res->driver->open(desc, res, node->resource->name, 0, mode) != 0) {
			ARC_DEBUG(ERR, "Failed to open file\n");
		}

		node->is_open = 1;
		tmp->is_open = 1;
		Arc_MutexUnlock(&node->property_lock);

		node = tmp;
	}
	Arc_MutexUnlock(&node->property_lock);

	desc->reference = Arc_ReferenceResource(node->resource);

	ARC_DEBUG(INFO, "Opened file successfully\n");

	return 0;
}


int Arc_ReadVFS(void *buffer, size_t size, size_t count, struct ARC_File *file) {
	if (buffer == NULL || file == NULL) {
		return -1;
	}

	if (size == 0 || count == 0) {
		return 0;
	}

	if (file->node->type == ARC_VFS_N_LINK && file->node->link == NULL) {
		return -1;
	}

	struct ARC_Resource *res = file->node->type == ARC_VFS_N_LINK ? file->node->link->resource : file->node->resource;

	if (res == NULL) {
		return -1;
	}

	return res->driver->read(buffer, size, count, file, res);
}

int Arc_WriteVFS(void *buffer, size_t size, size_t count, struct ARC_File *file) {
	if (buffer == NULL || file == NULL) {
		return -1;
	}

	if (size == 0 || count == 0) {
		return 0;
	}

	if (file->node->type == ARC_VFS_N_LINK && file->node->link == NULL) {
		return -1;
	}

	struct ARC_Resource *res = file->node->type == ARC_VFS_N_LINK ? file->node->link->resource : file->node->resource;

	if (res == NULL) {
		return -1;
	}

	return res->driver->write(buffer, size, count, file, res);
}

int Arc_SeekVFS(struct ARC_File *file, long offset, int whence) {
	if (file == NULL) {
		return -1;
	}

	if (file->node->type == ARC_VFS_N_LINK && file->node->link == NULL) {
		return -1;
	}

	struct ARC_Resource *res = file->node->type == ARC_VFS_N_LINK ? file->node->link->resource : file->node->resource;

	if (res == NULL) {
		return -1;
	}

	return res->driver->seek(file, res, offset, whence);
}

int Arc_CloseVFS(struct ARC_File *file) {
	if (file == NULL) {
		return -1;
	}

	ARC_DEBUG(INFO, "Closing %p\n", file);

	struct ARC_VFSNode *node = file->node;

	if (Arc_QLock(&node->branch_lock) != 0) {
		ARC_DEBUG(ERR, "Lock error\n");
		return -1;
	}
	Arc_QYield(&node->branch_lock);

	Arc_MutexLock(&node->property_lock);

	// TODO: Account if node->type == ARC_VFS_N_LINK

	if (node->ref_count > 1) {
		ARC_DEBUG(INFO, "ref_count (%d) > 1, closing file descriptor\n", node->ref_count);

		Arc_UnreferenceResource(file->reference);
		Arc_SlabFree(file);
		node->ref_count--; // TODO: Atomize

		Arc_MutexUnlock(&node->property_lock);

		return 0;
	}

	struct ARC_Resource *res = node->resource;

	if (res == NULL) {
		ARC_DEBUG(ERR, "Node has NULL resource\n")
	}

	if (res->driver->close(file, res) != 0) {
		ARC_DEBUG(ERR, "Failed to physically close file\n");
	}

	struct ARC_VFSNode *parent = node->parent;
	struct ARC_VFSNode *top = node->mount->node;
	vfs_delete_node(node, 0);
	vfs_bottom_up_prune(parent, top);
	Arc_SlabFree(file);

	ARC_DEBUG(INFO, "Closed file successfully\n");

	return 0;
}

int Arc_CreateVFS(char *path, uint32_t mode, int type) {
	if (path == NULL) {
		ARC_DEBUG(ERR, "No path given\n");
		return EINVAL;
	}

	ARC_DEBUG(INFO, "Creating %s (%o, %d)\n", path, mode, type);

	struct vfs_traverse_info info = { .mode = mode, .type = type, .create_level = VFS_FS_CREAT };
	if (*path == '/') {
		info.start = &vfs_root;
	} else {
		// info.start = get_current_directory();
	}

	ARC_DEBUG(INFO, "Creating node graph %s from node %p\n", path, info.start)

	return vfs_traverse(path, &info, 0);
}

int Arc_RemoveVFS(char *filepath, bool physical, bool recurse) {
	if (filepath == NULL) {
		ARC_DEBUG(ERR, "No path given\n");
		return -1;
	}

	ARC_DEBUG(INFO, "Removing %s (%d, %d)\n", filepath, physical, recurse);

	struct vfs_traverse_info info = { .create_level = VFS_NO_CREAT };
	if (*filepath == '/') {
		info.start = &vfs_root;
	} else {
		// info.start = get_current_directory();
	}

	int ret = vfs_traverse(filepath, &info, 0);

	if (ret != 0 || info.node == NULL) {
		ARC_DEBUG(ERR, "%s does not exist in node graph\n", filepath);
		return -1;
	}

	if (recurse == 0 && info.node->type == ARC_VFS_N_DIR) {
		ARC_DEBUG(ERR, "Trying to non-recursively delete directory\n");
		return -1;
	}

	Arc_MutexLock(&info.node->property_lock);
	if (info.node->ref_count > 0 || info.node->is_open == 0) {
		ARC_DEBUG(ERR, "Node %p is still in use\n", info.node);
		Arc_MutexUnlock(&info.node->property_lock);
		return -1;
	}

	if (Arc_QLock(&info.node->branch_lock) != 0) {
		ARC_DEBUG(ERR, "Lock error\n");
	}
	Arc_QYield(&info.node->branch_lock);

	// We now have the node completely under control

	if (physical == 1 && info.mount != NULL) {
		ARC_DEBUG(INFO, "Removing %s physically on mount %p\n", info.mountpath, info.mount);

		struct ARC_Resource *res = info.mount->resource;

		if (res == NULL) {
			ARC_DEBUG(ERR, "Cannot physically remove path, mount resource is NULL\n");
			Arc_MutexUnlock(&info.node->property_lock);
			Arc_QUnlock(&info.node->branch_lock);
			return -1;
		}

		struct ARC_SuperDriverDef *def = (struct ARC_SuperDriverDef *)res->driver->driver;

		if (def->remove(info.mountpath) != 0) {
			ARC_DEBUG(WARN, "Cannot physically remove path\n");
		}
	}

	struct ARC_VFSNode *parent = info.node->parent;
	vfs_delete_node(info.node, recurse);
	vfs_bottom_up_prune(parent, info.mount);

	return 0;
};

int Arc_LinkVFS(char *a, char *b, uint32_t mode) {
	if (a == NULL || b == NULL) {
		ARC_DEBUG(ERR, "Invalid parameters given (%p, %p)\n", a, b);
		return EINVAL;
	}

	struct vfs_traverse_info info_a = { .create_level = VFS_GR_CREAT };
	if (*a == '/') {
		info_a.start = &vfs_root;
	} else {
		// info_a.start = get_current_directory();
	}

	ARC_DEBUG(INFO, "Linking %s -> %s (%o)\n", a, b, mode);

	int ret = vfs_traverse(a, &info_a, 0);

	if (ret != 0) {
		ARC_DEBUG(ERR, "Failed to find %s\n", a);
		return 1;
	}

	struct vfs_traverse_info info_b = { .create_level = VFS_FS_CREAT, .mode = mode, .type = ARC_VFS_N_LINK };
	if (*a == '/') {
		info_b.start = &vfs_root;
	} else {
		// info_b.start = get_current_directory();
	}

	ret = vfs_traverse(b, &info_b, 0);

	if (ret != 0) {
		ARC_DEBUG(ERR, "Failed to find or create %s\n", b);
		return 1;
	}

	struct ARC_VFSNode *src = info_a.node;
	struct ARC_VFSNode *lnk = info_b.node;

	// Resolve link if src is a link
	if (src->type == ARC_VFS_N_LINK) {
		src = src->link;
	}

	Arc_MutexLock(&src->property_lock);
	Arc_MutexLock(&lnk->property_lock);
	src->stat.st_nlink++;
	// src->ref_count is already incremented from the traverse
	lnk->ref_count--;
	lnk->link = src;
	lnk->is_open = src->is_open;
	Arc_MutexUnlock(&src->property_lock);
	Arc_MutexUnlock(&lnk->property_lock);

	// TODO: Make sure that lnk is a classified as a link
	//       on physical filesystem and in node graph
	// TODO: Think about if b already exists
	// TODO: Perms check

	ARC_DEBUG(INFO, "Linked %s (%p, %d) -> %s (%p, %d)\n", a, src, src->ref_count, b, lnk, lnk->ref_count);

	return 0;
}

int Arc_RenameVFS(char *a, char *b) {
	if (a == NULL || b == NULL) {
		ARC_DEBUG(ERR, "Src (%p) or dest (%p) path NULL\n", a, b);
		return -1;
	}

	ARC_DEBUG(INFO, "Renaming %s -> %s\n", a, b);

	struct vfs_traverse_info info_a = { .create_level = VFS_GR_CREAT };

	if (*a == '/') {
		info_a.start = &vfs_root;
	} else {
		// info_a.start = get_current_directory();
	}

	int ret = vfs_traverse(a, &info_a, 0);

	if (ret != 0) {
		ARC_DEBUG(ERR, "Failed to find %s in node graph\n", a);
		return -1;
	}

	struct vfs_traverse_info info_b = { .create_level = VFS_FS_CREAT | VFS_NOLCMP, .mode = info_a.mode };

	if (*b == '/') {
		info_b.start = &vfs_root;
	} else {
		// info_b.start = get_current_directory();
	}

	ret = vfs_traverse(b, &info_b, 0);

	if (ret != 0) {
		ARC_DEBUG(ERR, "Failed to find or create %s in node graph / on disk\n", b);
	}

	struct ARC_VFSNode *node_a = info_a.node;
	struct ARC_VFSNode *node_b = info_b.node;

	// Patch node_a onto node_b
	if (Arc_QLock(&node_b->branch_lock) != 0) {
		ARC_DEBUG(ERR, "Lock error\n");
	}
	Arc_QYield(&node_b->branch_lock);

	if (node_b->children != NULL) {
		node_b->children->prev = node_a;
	}

	node_b->children = node_a;

	Arc_QUnlock(&node_b->branch_lock);

	// Remove node from parent node
	struct ARC_VFSNode *parent = node_a->parent;
	ARC_DEBUG(INFO, "%p\n", parent);
	if (Arc_QLock(&parent->branch_lock)) {
		ARC_DEBUG(ERR, "Lock error\n");
	}
	Arc_QYield(&parent->branch_lock);
	struct ARC_VFSNode *child = parent->children;
	while (child != node_a && child != NULL) {
		child = child->next;
	}

	// Update links, if child != node_a, then we
	// do not need to do this, as the child has
	// already been removed
	if (child == node_a) {
		if (child->next != NULL) {
			child->next->prev = child->prev;
		}

		if (child->prev != NULL) {
			child->prev->next = child->next;
		} else {
			parent->children = child->next;
		}
	}
	Arc_QUnlock(&parent->branch_lock);

	// Patch node in
	if (Arc_QLock(&node_a->branch_lock) != 0) {
		ARC_DEBUG(ERR, "Lock error\n");
	}
	Arc_QYield(&node_a->branch_lock);
	node_a->parent = node_b;
	Arc_QUnlock(&node_a->branch_lock);

	// Rename the resource
	Arc_MutexLock(&node_a->resource->prop_mutex);
	Arc_SlabFree(node_a->resource->name);
	node_a->resource->name = strdup(b);
	Arc_MutexUnlock(&node_a->resource->prop_mutex);

	node_a->ref_count--; // TODO: Atomize
	node_b->ref_count--; // TODO: Atomize

	if (info_a.mount == NULL) {
		// Virtually renamed the files, nothing else
		// to do
		return 0;
	}

	if (info_a.mount != info_b.mount) {
		// Different mountpoints, we need to migrate the files
		// TODO: Physically copy A to B, delete A
		ARC_DEBUG(ERR, "Physical migration unimplemented\n");
		return 0;
	} else {
		// Physically rename file
		struct ARC_SuperDriverDef *def = (struct ARC_SuperDriverDef *)info_a.mount->resource->driver->driver;

		def->rename(info_a.mountpath, info_b.mountpath);
	}

	return 0;
}

#undef VFS_NO_CREAT
#undef VFS_GR_CREAT
#undef VFS_FS_CREAT
#undef VFS_NOLCMP
