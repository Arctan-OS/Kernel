/**
 * @file graph.c
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
 * The manager of the VFS's node graph.
*/
#include <fs/graph.h>
#include <global.h>
#include <stdint.h>
#include <mm/allocator.h>
#include <abi-bits/errno.h>
#include <abi-bits/stat.h>
#include <lib/util.h>
#include <drivers/dri_defs.h>
#include <lib/resource.h>

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

uint64_t vfs_type2idx(int type, struct ARC_VFSNode *mount) {
	switch (type) {
	case ARC_VFS_N_BUFF: {
		return ARC_FDRI_BUFFER;
	}

	case ARC_VFS_N_FIFO: {
		return ARC_FDRI_FIFO;
	}

	default: {
		if (mount == NULL) {
			// If the mount is NULL, then create a buffer
			// that can hold some temporary data
			return ARC_FDRI_BUFFER;
		}
	
		return mount->resource->dri_index + 1;
	}
	}
}


int arc_vfs_delete_node_recurse(struct ARC_VFSNode *node) {
	if (node == NULL || node->ref_count > 0 || node->is_open != 0) {
		return 1;
	}

	int err = 0;

	struct ARC_VFSNode *child = node->children;
	while (child != NULL) {
		err += arc_vfs_delete_node_recurse(child);
		child = child->next;
	}

	free(node->name);
	uninit_resource(node->resource);
	free(node);

	return err;
}


int arc_vfs_delete_node(struct ARC_VFSNode *node, bool recurse) {
	if (node == NULL || node->ref_count > 0 || node->is_open != 0) {
		return 1;
	}

	struct ARC_VFSNode *child = node->children;

	int err = 0;

	while (child != NULL && recurse == 1) {
		err = arc_vfs_delete_node_recurse(child);
		child = child->next;
	}

	if (node->children != NULL) {
		return err + 1;
	}

	free(node->name);
	uninit_resource(node->resource);

	if (node->prev == NULL) {
		node->parent->children = node->next;
	} else {
		node->prev->next = node->next;
	}

	if (node->next != NULL) {
		node->next->prev = node->prev;
	}

	free(node);

	return err;
}

// TODO: This prune function is called when closing or removing a file.
//       First, it needs testing like the remove and close functions.
//       Second, this may become costly in terms of speed, as quite a
//       big lock is grabbed (locking the topmost node).
int arc_vfs_bottom_up_prune(struct ARC_VFSNode *bottom, struct ARC_VFSNode *top) {
	struct ARC_VFSNode *current = bottom;
	int freed = 0;

	if (qlock(&top->branch_lock) != 0) {
		return -1;
	}
	qlock_yield(&top->branch_lock);

	// We have the topmost node held
	// Delete unused directories
	do {
		void *tmp = current->parent;

		if (current->children == NULL && current->ref_count == 0) {
			arc_vfs_delete_node(current, 0);
			freed++;
		}

		current = tmp;
	} while (current != top && current != NULL && current->type != ARC_VFS_N_MOUNT);

	qunlock(&top->branch_lock);

	return freed;
}

int vfs_top_down_prune_recurs(struct ARC_VFSNode *node, int depth) {
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
		int diff = vfs_top_down_prune_recurs(child, depth - 1);

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


int arc_vfs_top_down_prune(struct ARC_VFSNode *top, int depth) {
	if (top == NULL) {
		ARC_DEBUG(ERR, "Start node is NULL\n");
		return -1;
	}

	if (depth == 0) {
		return 0;
	}

	if (qlock(&top->branch_lock) != 0) {
		return -2;
	}
	qlock_yield(&top->branch_lock);

	struct ARC_VFSNode *node = top->children;
	int count = 0;

	while (node != NULL) {
		count += vfs_top_down_prune_recurs(node, depth - 1);
		node = node->next;
	}

	return count;
}


int vfs_open_vfs_link(struct ARC_VFSNode *node, struct ARC_VFSNode **ret, int link_depth) {
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

int arc_vfs_traverse(char *filepath, struct arc_vfs_traverse_info *info, int link_depth) {
	if (filepath == NULL || info == NULL) {
		return -1;
	}

	size_t max = strlen(filepath);

	if (max == 0) {
		return -1;
	}

	ARC_DEBUG(INFO, "Traversing %s\n", filepath);

	struct ARC_VFSNode *node = info->start;
	node->ref_count++; // TODO: Atomize

	if (qlock(&node->branch_lock) != 0) {
		ARC_DEBUG(ERR, "Lock error\n");
		return -1;
	}

	size_t last_sep = 0;
	for (size_t i = 0; i < max; i++) {
		bool is_last = (i == max - 1);

		if (!is_last && filepath[i] != '/') {
			// Not at end or not on separator character
			// so skip
			continue;
		}

		if (is_last && (info->create_level & ARC_VFS_NOLCMP) != 0) {
			// This is the last compoenent, and the
			// ARC_VFS_NOLCMP is set, therefore skip
			continue;
		}
		// The following code will only run if:
		//     This is the last character (and therefore the last component)
		//     unless ARC_VFS_NOLCMP is set
		//     If a separator ('/') character is found

		// Determine the component which may or may not be
		// enclosed in separator characters
		//     COMPONENT
		//     ^-size--^
		//     /COMPONENT
		//      ^-size--^
		//     /COMPONENT/
		//      ^-size--^
		int component_size = i - last_sep;
		char *component = filepath + last_sep;

		last_sep = i;

		if (*component == '/') {
			component++;
			component_size--;
		}

		if (is_last && filepath[i] != '/') {
			component_size++;
		}

		if (component_size <= 0) {
			// There is no component
			continue;
		}

		// Wait to acquire lock on current node
		qlock_yield(&node->branch_lock);

		struct ARC_VFSNode *next = NULL;

		if (node->type == ARC_VFS_N_MOUNT) {
			info->mount = node;
			info->mountpath = component;
		}

		if (component_size == 2 && component[0] == '.' && component[1] == '.') {
			next = node->parent;
			goto resolve;
		} else if (component_size == 1 && component[0] == '.') {
			continue;
		}

		next = node->children;

		while (next != NULL) {
			if (strncmp(component, next->name, component_size) == 0) {
				break;
			}

			next = next->next;
		}

		if (next == NULL) {
			// Node is still NULL, cannot find it in
			// current context
			if ((info->create_level & ARC_VFS_NO_CREAT) != 0) {
				// The node does not exist, but the ARC_VFS_NO_CREAT
				// is set, therefore just return
				ARC_DEBUG(ERR, "ARC_VFS_NO_CREAT specified\n");
				return i;
			}

			// NOTE: ARC_VFS_GR_CREAT is not really used

			next = (struct ARC_VFSNode *)alloc(sizeof(struct ARC_VFSNode));

			if (next == NULL) {
				ARC_DEBUG(ERR, "Failed to allocate new node\n");
				return i;
			}

			memset(next, 0, sizeof(struct ARC_VFSNode));

			// Insert it into graph
			next->parent = node;
			if (node->children != NULL) {
				next->next = node->children;
				node->children->prev = next;
			}
			node->children = next;

			// Set properties
			next->name = strndup(component, component_size);
			next->type = is_last ? info->type : ARC_VFS_N_DIR;
			init_static_qlock(&next->branch_lock);
			init_static_mutex(&next->property_lock);

			ARC_DEBUG(INFO, "Created new node %s (%p)\n", next->name, next);

			// TODO: Set stat according to current information

			// Check for node on physical filesystem
			if (info->mount == NULL) {
				goto skip_stat;
			}

			next->mount = info->mount;

			// This is the path to the file from the mountpoint
			// Now calculate the size, mounthpath starts at a component
			// therefore there is no need to advance info->mountpath, the length
			// between the current and mountpath pointers are calculated and one is
			// added if the current component is the last component
			size_t phys_path_size = (size_t)((uintptr_t)(filepath + i) - (uintptr_t)info->mountpath) + is_last;
			char *phys_path = strndup(info->mountpath, phys_path_size);

			ARC_DEBUG(INFO, "Node is under a mountpoint, statting %s on node %p\n", phys_path, info->mount);

			struct ARC_Resource *res = info->mount->resource;
			struct ARC_SuperDriverDef *def = (struct ARC_SuperDriverDef *)res->driver->driver;

			if (def->stat(res, phys_path, &next->stat) == 0) {
				// Stat succeeded, file exists on filesystem,
				// set type
				next->type = vfs_stat2type(next->stat);
			} else if ((info->create_level & ARC_VFS_FS_CREAT) != 0){
				// ARC_VFS_FS_CREAT is specified and the stat failed,
				// create the node in the physical filesystem
				def->create(phys_path, 0, next->type);
			}

			skip_stat:;

			// Create resource for node if it needs one
			if (next->type == ARC_VFS_N_DIR) {
				goto skip_resource;
			}

			if (info->mount != NULL) {
				mutex_lock(&info->mount->resource->dri_state_mutex);
			}

			// Figure out the index from the given type
			uint64_t index = vfs_type2idx(next->type, info->mount);
			// If the resource is NULL, the driver will definitely be
			// in the super/file group, otherwise use the mount's group
			int group = (info->mount == NULL ? 0 : info->mount->resource->dri_group);
			// Determine the arguments
			void *args = (info->overwrite_arg == NULL && info->mount != NULL ? info->mount->resource->driver_state : info->overwrite_arg);

			next->resource = init_resource(group, index, args);

			if (info->mount != NULL) {
				mutex_unlock(&info->mount->resource->dri_state_mutex);

			}
			skip_resource:;
		}

		resolve:;

		// Next node should not be NULL
		if (qlock(&next->branch_lock) != 0) {
			ARC_DEBUG(ERR, "Lock error!\n");
			// TODO: Do something
		}
		qunlock(&node->branch_lock);

		node->ref_count--; // TODO: Atomize
		node = next;
		node->ref_count++; // TODO: Atomize
	}

	ARC_DEBUG(INFO, "Successfully traversed %s\n", filepath);

	// Returned state of node:
	//    - branch_lock is held
	//    - ref_count is incremented by 1
	// Caller must release branch_lock and
	// decrement ref_count when it is done
	// wth the node

	info->node = node;

	return 0;
}
