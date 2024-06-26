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
#include <mm/slab.h>
#include <fs/vfs.h>
#include <fs/dri_defs.h>
#include <global.h>
#include <util.h>

// TODO: Implement error cases for all functions.

static const char *root = "\0";
static const struct ARC_Resource root_res = { .name = "/" };
static struct ARC_VFSNode vfs_root = { 0 };

struct vfs_traverse_info {
	/// The node to start traversal from.
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
	/// Overwrite for final node's resource if it needs to be created (NULL = no overwrite).
	// TODO: This is fairly hacky, try to come up with a better way
	// to do this
	void *overwrite_arg;
};

#define VFS_DETERMINE_START(info, path) \
        if (*path == '/') { \
		info.start = &vfs_root; \
	} else { \
		ARC_DEBUG(WARN, "Definitely getting current directory\n") \
	}

/**
 * Convert stat.st_type to node->type.
 * */
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

/**
 * Internal recursive delete function.
 *
 * Recursively destroy the node.
 *
 * @param struct ARC_VFSNode *node - The node to start recursive destruction from.
 * @return the number of nodes that were not destroyed.
 * */
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

/**
 * Delete a node from the node graph.
 *
 * General function for deleting nodes. Parent
 * node's children are updated to remove the link
 * to this node.
 *
 * @param struct ARC_VFSNode *node - The node which to destroy.
 * @param bool recurse - Whether to recurse or not.
 * @return number of nodes which were not destroyed.
 * */
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

	if (node->children != NULL) {
		return err + 1;
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
/**
 * Prune unused nodes from bottom to top.
 *
 * A node, starting at a lower level of the graph
 * starts a chain of destruction upward of unused nodes.
 * This frees up memory, at the cost of some caching.
 *
 * @param struct ARC_VFSNode *bottom - Bottom-most node to start from.
 * @param struct ARC_VFSNode *top - Topmost node to end on.
 * @return positive integer indicating the number of nodes freed.
 * */
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

int _vfs_top_down_prune_recurs(struct ARC_VFSNode *node, int depth) {
	if (node->ref_count > 0 || depth <= 0 || node->type == ARC_VFS_N_MOUNT) {
		// Set the sign bit to indicate that this
		// path cannot be freed as there is still
		// something being used on it. All other
		// unused files / folders / whatnot has
		// been freed
		return -1;
	}

	struct ARC_VFSNode *child = node->children;
	int count = 0;

	while (child != NULL) {
		int diff = _vfs_top_down_prune_recurs(child, depth - 1);

		if (diff < 0 && count > 0) {
			count *= -1;
		}

		count += diff;

		child = child->next;
	}

	if (node->children == NULL) {
		// Free this node
	}

	return count;
}

/**
 * Prune unused nodes from top to a depth.
 *
 * Given a starting node, top, work downwards to free
 * any unused nodes to save on some memory. It takes
 * quite a big lock (the top node is locked).
 *
 * @param struct ARC_VFSNode *top - The starting node.
 * @param int depth - How far down the function can traverse.
 * @return poisitive integer indicating the number of nodes freed.
 * */
int vfs_top_down_prune(struct ARC_VFSNode *top, int depth) {
	if (top == NULL) {
		ARC_DEBUG(ERR, "Start node is NULL\n");
		return -1;
	}

	if (depth == 0) {
		return 0;
	}

	if (Arc_QLock(&top->branch_lock) != 0) {
		return -2;
	}
	Arc_QYield(&top->branch_lock);

	struct ARC_VFSNode *node = top->children;
	int count = 0;

	while (node != NULL) {
		count += _vfs_top_down_prune_recurs(node, depth - 1);
		node = node->next;
	}

	return count;
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

uint64_t vfs_idx2idx(int type, uint64_t idx) {
	switch (type) {
	case ARC_VFS_N_BUFF: {
		return ARC_DRI_BUFFER + 1;
	}

	case ARC_VFS_N_FIFO: {
		return ARC_DRI_FIFO + 1;
	}

	case ARC_VFS_N_LINK: {
		return 0xAB;
	}

	default: {
		return idx + 1;
	}
	}

	// Once more, how did you get here?
	return -1;
}

/**
 * Traverse the node graph
 *
 * The ultimate function to traverse the node graph.
 * Links are resolved and opened, resources are created,
 * new nodes are created upon specification.
 *
 * @param char *filepath - The path to to the file.
 * @param struct vfs_traverse_info *info - Input values and return values of the function.
 * @param int link_depth - How many links to resolve.
 * @return 0 upon success, info->node has branch_lock held and ref_count is incremented by
 * one. Caller needs to unlock the branch_lock and decrement ref_count once it is done using
 * the node. A -2 means that no filepath was traversed, as the one given consists of zero characters,
 * therefore the aforementioned state of the node is not true.
 * */
int vfs_traverse(char *filepath, struct vfs_traverse_info *info, int link_depth) {
	if (filepath == NULL || info == NULL || info->start == NULL) {
		return EINVAL;
	}

	size_t max = strlen(filepath);

	if (max == 0) {
		// Definitely created a new node and everything
		// for this empty filepath
		info->node = info->start;
		return -2;
	}

	ARC_DEBUG(INFO, "Traversing %s\n", filepath);

	struct ARC_VFSNode *node = info->start;
	node->ref_count++; // TODO: Atomize
	info->node = node;

	if (Arc_QLock(&node->branch_lock) != 0) {
		 ARC_DEBUG(ERR, "Lock error!\n");
		 return -1;
	}

	size_t x = 0;
	size_t y = 0;

	for (size_t i = 0; i < max; i++) {
		if (i == max - 1 && (info->create_level & VFS_NOLCMP) != 0) {
			// End of the string reached, VFS_NOLCMP set, break
			break;
		}

		if (filepath[i] == '/' || i == max - 1) {
			// Separator encountered, or at the end of the string
			y = i;
		}

		if (y <= x) {
			// If number of characters between last separator
			// and this one is none, skip
			continue;
		}

		char *component = (char *)(filepath + x);

		if (*component == '/') {
			// Component begins with separator, cull it off
			component++;
		}

		size_t component_length = y - x;

		if (filepath[y] == '/') {
			// Component ends with separator, cull it off
			component_length--;
		} else {
			// Component does not end with separaotr,
			// component_length is the difference of two
			// indices, add one to make it a true length
			component_length++;
		}

		// Make note of last separator
		x = y;

		if (component_length <= 0) {
			// The actual component, after all the culling, is
			// non-exsitent, skip
			continue;
		}
	
		if (node->type == ARC_VFS_N_MOUNT) {
			info->mount = node;
			info->mountpath = component;
		}

		Arc_QYield(&node->branch_lock);

		if (*component == '.' && component_length == 2 && *(component + 1) == '.') {
			// .. dir, go up one
			node = node->parent == NULL ? node : node->parent;
			continue;
		} else if (*component == '.' && component_length == 1) {
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

			// Do wacky stuff to determine a path which looks like
			// this: mountpoint/a/b/c/d/file.extension, which is stored
			// in stat_path
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
					// Didn't fail to stat, determine the file's type
					// from the physical filesystem so long that it is
					// not NULL, in which case the old type specified
					// by the caller would be kept
					if (vfs_stat2type(new->stat) != ARC_VFS_NULL) {
						new->type = vfs_stat2type(new->stat);
					}
				}
			}

                        // Initialzie resource if needed
			if (new->type != ARC_VFS_N_DIR && new->resource == NULL) {
				ARC_DEBUG(INFO, "Node has no resource, creating one\n");
				// Get the mountpoint's resource and lock it to make sure
				// it does not change
				struct ARC_Resource *res = info->mount->resource;
				Arc_MutexLock(&res->dri_state_mutex);

				// Group will always be 0, as all filesystem drivers
				// are in group 0, unless if the type of the node is a
				// link
				int group = new->type == ARC_VFS_N_LINK ? 0xAB : 0x00;
				// Determine the index from the type of the file and the mountpoint's
				// index
				uint64_t index = vfs_idx2idx(new->type, res->dri_index);

				// Create a new resource and set the resource field, also
				// unlock the mountpoint's resource
				struct ARC_Resource *nres = Arc_InitializeResource(stat_path, group, index, info->overwrite_arg == NULL ? res->driver_state : info->overwrite_arg);
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

		child->ref_count++; // TODO: Atomize
		node->ref_count--; // TODO: Atomize
		node = child;
		info->node = node;
	}
	// TODO: Fix functions using this function, as they do
	//       not account for the following:
	//            node->ref_count remains incremented by 1
	//            node->branch_lock is held
	//       Lock and ref_count need to be decremented by
	//       caller once node is no longer being used (verify)
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

	ARC_DEBUG(INFO, "Created VFS root (%p)\n", &vfs_root);

	return 0;
}

int Arc_MountVFS(char *mountpoint, struct ARC_Resource *resource, int fs_type) {
	if (mountpoint == NULL || resource == NULL || fs_type == ARC_VFS_NULL) {
		ARC_DEBUG(ERR, "Invalid arguments (%p, %p, %d)\n", mountpoint, resource, fs_type);
		return EINVAL;
	}

	ARC_DEBUG(INFO, "Mounting %p on %s (%d)\n", resource, mountpoint, fs_type);

	// If the directory does not already exist, create it in graph, as a disk write could be saved
	struct vfs_traverse_info info = { .type = ARC_VFS_N_DIR, .create_level = VFS_GR_CREAT };
	VFS_DETERMINE_START(info, mountpoint);

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

	Arc_QUnlock(&mount->branch_lock);

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

	Arc_QUnlock(&mount->branch_lock);

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
	VFS_DETERMINE_START(info, path);

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
	if (node->is_open == 0 && node->type != ARC_VFS_N_DIR) {
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
		desc->node->is_open = 1;
		Arc_MutexUnlock(&node->property_lock);

		node = desc->node;

	}
	Arc_MutexUnlock(&node->property_lock);

	desc->reference = Arc_ReferenceResource(node->resource);

	Arc_QUnlock(&node->branch_lock);

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

	if (res == NULL || res->driver->read == NULL) {
		ARC_DEBUG(ERR, "One or more is NULL: %p %p\n", res, res->driver->read);
		return -1;
	}

	int ret = res->driver->read(buffer, size, count, file, res);

	file->offset += ret;

	return ret;
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
		ARC_DEBUG(ERR, "One or more is NULL: %p %p\n", res, res->driver->write);
		return -1;
	}

	int ret = res->driver->write(buffer, size, count, file, res);

	file->offset += ret;

	return ret;
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

	if (node->ref_count > 1 || (node->type != ARC_VFS_N_FILE && node->type != ARC_VFS_N_LINK)) {
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

	if (res != NULL && res->driver->close(file, res) != 0) {
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

int Arc_CreateVFS(char *path, uint32_t mode, int type, void *arg) {
	if (path == NULL) {
		ARC_DEBUG(ERR, "No path given\n");
		return EINVAL;
	}

	ARC_DEBUG(INFO, "Creating %s (%o, %d)\n", path, mode, type);

	struct vfs_traverse_info info = { .mode = mode, .type = type, .create_level = VFS_FS_CREAT, .overwrite_arg = arg };
	VFS_DETERMINE_START(info, path);

	ARC_DEBUG(INFO, "Creating node graph %s from node %p\n", path, info.start);

	int ret = vfs_traverse(path, &info, 0);

	if (ret == 0) {
		Arc_QUnlock(&info.node->branch_lock);
		info.node->ref_count--; // TODO: Atomize
	}

	return ret;
}

int Arc_RemoveVFS(char *filepath, bool physical, bool recurse) {
	if (filepath == NULL) {
		ARC_DEBUG(ERR, "No path given\n");
		return -1;
	}

	ARC_DEBUG(INFO, "Removing %s (%d, %d)\n", filepath, physical, recurse);

	struct vfs_traverse_info info = { .create_level = VFS_NO_CREAT };
	VFS_DETERMINE_START(info, filepath);

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
	VFS_DETERMINE_START(info_a, a);

	ARC_DEBUG(INFO, "Linking %s -> %s (%o)\n", a, b, mode);

	int ret = vfs_traverse(a, &info_a, 0);

	if (ret != 0) {
		ARC_DEBUG(ERR, "Failed to find %s\n", a);
		return 1;
	}

	struct vfs_traverse_info info_b = { .create_level = VFS_FS_CREAT, .mode = mode, .type = ARC_VFS_N_LINK };
	VFS_DETERMINE_START(info_b, b);

	ret = vfs_traverse(b, &info_b, 0);

	if (ret != 0) {
		ARC_DEBUG(ERR, "Failed to find or create %s\n", b);
		return 1;
	}

	struct ARC_VFSNode *src = info_a.node;
	struct ARC_VFSNode *lnk = info_b.node;

	Arc_QUnlock(&src->branch_lock);
	Arc_QUnlock(&lnk->branch_lock);

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

// TODO: Currently, this function renames the resource of a single file.
//       However, if the "file" that is being renamed is a directory,
//       the resource names of the files in it are not changed. Probably
//       do not need resource names, they take up extra space. Should replace
//       them with just a counter (as a single system is probably not going to have
//       UINT64_MAX resources).
int Arc_RenameVFS(char *a, char *b) {
	if (a == NULL || b == NULL) {
		ARC_DEBUG(ERR, "Src (%p) or dest (%p) path NULL\n", a, b);
		return -1;
	}

	ARC_DEBUG(INFO, "Renaming %s -> %s\n", a, b);

	struct vfs_traverse_info info_a = { .create_level = VFS_GR_CREAT };
	VFS_DETERMINE_START(info_a, a);

	int ret = vfs_traverse(a, &info_a, 0);

	if (ret != 0) {
		ARC_DEBUG(ERR, "Failed to find %s in node graph\n", a);
		return -1;
	}

	// Lock parent ASAP ensuring node_a is not modified
	if (Arc_QLock(&info_a.node->parent->branch_lock) != 0) {
		ARC_DEBUG(ERR, "Lock error\n");
	}
	Arc_QYield(&info_a.node->parent->branch_lock);

	struct vfs_traverse_info info_b = { .create_level = VFS_FS_CREAT | VFS_NOLCMP, .mode = info_a.mode };
	VFS_DETERMINE_START(info_b, b);

	ret = vfs_traverse(b, &info_b, 0);

	if (ret != 0) {
		ARC_DEBUG(ERR, "Failed to find or create %s in node graph / on disk\n", b);
		Arc_QUnlock(&info_a.node->parent->branch_lock);
		return -1;
	}

	struct ARC_VFSNode *node_a = info_a.node;
	struct ARC_VFSNode *node_b = info_b.node;

	if (node_b == node_a->parent) {
		// Node A is already under B, just rename A.
		Arc_QUnlock(&node_b->branch_lock);
		goto rename;
	}

	// Remove node_a from parent node in preparation for patching
	struct ARC_VFSNode *parent = node_a->parent;
	if (node_a->next != NULL) {
		node_a->next->prev = node_a->prev;
	}
	if (node_a->prev != NULL) {
		node_a->prev->next = node_a->next;
	} else {
		parent->children = node_a->next;
	}

	// Patch node_a onto node_b
	// node_b->branch_lock is locked by vfs_traverse
	if (node_b->children != NULL) {
		node_b->children->prev = node_a;
	}

	// Let node_a see the patch
	// node_a->branch_lock is locked by vfs_traverse
	node_a->parent = node_b;
	node_a->next = node_b->children;
	node_a->prev = NULL;
	node_b->children = node_a;

	Arc_QUnlock(&node_b->branch_lock);

	rename:;
	// Rename the resource
	Arc_MutexLock(&node_a->resource->prop_mutex);
	Arc_SlabFree(node_a->resource->name);
	node_a->resource->name = strdup(b);
	Arc_MutexUnlock(&node_a->resource->prop_mutex);

	Arc_QUnlock(&node_a->branch_lock);

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

int vfs_list(struct ARC_VFSNode *node, int recurse, int org) {
	if (node == NULL) {
		return -1;
	}

	struct ARC_VFSNode *child = node->children;
	while (child != NULL) {
		for (int i = 0; i < org - recurse; i++) {
			printf("\t");
		}

		printf("%s\n", child->name);

		if (recurse > 0) {
			vfs_list(child, recurse - 1, org);
		}

		child = child->next;
	}

	return 0;
}

int Arc_ListVFS(char *path, int recurse) {
	if (path == NULL) {
		return -1;
	}

	struct vfs_traverse_info info = { .create_level = VFS_NO_CREAT };

	VFS_DETERMINE_START(info, path);

	if (vfs_traverse(path, &info, 0) != 0) {
		return -1;
	}

	info.node->ref_count--; // TODO: Atomize

	printf("Listing of %s\n", path);
	vfs_list(&vfs_root, recurse, recurse);

	Arc_QUnlock(&info.node->branch_lock);

	return 0;
}

struct ARC_VFSNode *Arc_RelNodeCreateVFS(char *relative_path, struct ARC_VFSNode *start, uint32_t mode, int type, void *arg) {
	if (relative_path == NULL || start == NULL) {
		ARC_DEBUG(ERR, "No path given\n");
		return NULL;
	}

	ARC_DEBUG(INFO, "Creating %p/%s (%o, %d)\n", start, relative_path, mode, type);

	struct vfs_traverse_info info = { .start = start, .mode = mode, .type = type, .create_level = VFS_GR_CREAT, .overwrite_arg = arg };

	int ret = vfs_traverse(relative_path, &info, 0);

	if (ret == 0) {
		Arc_QUnlock(&info.node->branch_lock);
		info.node->ref_count--; // TODO: Atomize
	}

	if (ret != 0) {
		return NULL;
	}

	return info.node;
}
