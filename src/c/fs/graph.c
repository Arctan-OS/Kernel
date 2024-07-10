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
#include <mm/slab.h>
#include <abi-bits/errno.h>
#include <abi-bits/stat.h>
#include <lib/util.h>
#include <fs/dri_defs.h>
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

uint64_t vfs_idx2idx(int type, struct ARC_Resource *res) {
	switch (type) {
	case ARC_VFS_N_BUFF: {
		return ARC_DRI_BUFFER + 1;
	}

	case ARC_VFS_N_FIFO: {
		return ARC_DRI_FIFO + 1;
	}

	default: {
		if (res == NULL) {
			return (uint64_t)-1;
		}

		return res->dri_index + 1;
	}
	}

	return (uint64_t)-1;
}

/**
 * Internal recursive delete function.
 *
 * Recursively destroy the node.
 *
 * @param struct ARC_VFSNode *node - The node to start recursive destruction from.
 * @return the number of nodes that were not destroyed.
 * */
int Arc_vfs_delete_node_recurse(struct ARC_VFSNode *node) {
	if (node == NULL || node->ref_count > 0 || node->is_open != 0) {
		return 1;
	}

	int err = 0;

	struct ARC_VFSNode *child = node->children;
	while (child != NULL) {
		err += Arc_vfs_delete_node_recurse(child);
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
int Arc_vfs_delete_node(struct ARC_VFSNode *node, bool recurse) {
	if (node == NULL || node->ref_count > 0 || node->is_open != 0) {
		return 1;
	}

	struct ARC_VFSNode *child = node->children;

	int err = 0;

	while (child != NULL && recurse == 1) {
		err = Arc_vfs_delete_node_recurse(child);
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
int Arc_vfs_bottom_up_prune(struct ARC_VFSNode *bottom, struct ARC_VFSNode *top) {
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
			Arc_vfs_delete_node(current, 0);
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
int Arc_vfs_top_down_prune(struct ARC_VFSNode *top, int depth) {
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
int Arc_vfs_traverse(char *filepath, struct vfs_traverse_info *info, int link_depth) {
	(void)link_depth;

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
	info->mountpath = filepath;

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

		if (node->type == ARC_VFS_N_MOUNT && node->mount->fs_type != ARC_VFS_FS_DEV) {
			// Last node was a mountpoint, this is the start
			// of the path on disk, also keep track of the mount node
			info->mount = node;
			info->mountpath = component;
		}

		Arc_QYield(&node->branch_lock);

		// Interpret . dirs
		if (*component == '.' && component_length == 2 && *(component + 1) == '.') {
			// .. dir, go up one
			node = node->parent == NULL ? node : node->parent;
			continue;
		} else if (*component == '.' && component_length == 1) {
			// . dir, skip
			continue;
		}

		// Attempt to find current component among
		// previous node's children (the existing context)
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

			// Node does not exist in node graph
			// Create it
			struct ARC_VFSNode *new = (struct ARC_VFSNode *)Arc_SlabAlloc(sizeof(struct ARC_VFSNode));

			if (new == NULL) {
				ARC_DEBUG(ERR, "Cannot allocate next node\n");
				// goto epic_fail;
			}

			memset(new, 0, sizeof(struct ARC_VFSNode));

			new->name = strndup(component, component_length);

			if (info->mount != NULL) {
				new->mount = info->mount->mount;
			}

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
			// this: a/b/c/d/file.extension, which is relative from
			// info->mount, and is stored in stat_path
			// NOTE: This is cursed
			char *use_path = info->mountpath == NULL ? filepath : info->mountpath;
			int length = i - ((uintptr_t)use_path - (uintptr_t)filepath) + 1;
			char *stat_path = strndup(use_path, use_path[length - 1] == '/' ? --length : length);

			if (info->mount != NULL) {
				ARC_DEBUG(INFO, "Mount is present, statting %s\n", stat_path);

				struct ARC_Resource *res = info->mount->resource;
				if (res == NULL) {
					ARC_DEBUG(INFO, "Resource is NULL for node %p (mount %p)\n", info->mount, info->mount->mount);
					goto skip_stat;
				}

				struct ARC_SuperDriverDef *def = (struct ARC_SuperDriverDef *)res->driver->driver;

				if (def == NULL) {
					ARC_DEBUG(INFO, "Superblock definition is NULL for %p\n", res);
					goto skip_stat;
				}

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

			skip_stat:;

                        // Initialzie resource if needed
			if (new->type != ARC_VFS_N_DIR && new->resource == NULL) {
				ARC_DEBUG(INFO, "Node has no resource, creating one\n");
				// Get the mountpoint's resource and lock it to make sure
				// it does not change
				struct ARC_Resource *res = NULL;

				if (info->mount != NULL) {
					res = info->mount->resource;
					Arc_MutexLock(&res->dri_state_mutex);
				}

				// Determine the index from the type of the file and the mountpoint's
				// index
				uint64_t index = vfs_idx2idx(new->type, res);

				if (index == (uint64_t)-1) {
					// Don't create a resource
					new->resource = NULL;
					goto skip_res;
				}

				// Create a new resource and set the resource field, also
				// unlock the mountpoint's resource
				struct ARC_Resource *nres = Arc_InitializeResource(0, index, (info->overwrite_arg == NULL && res != NULL) ? res->driver_state : info->overwrite_arg);

				if (info->mount != NULL) {
					Arc_MutexUnlock(&res->dri_state_mutex);
				}

				new->resource = nres;

				if (nres == NULL) {
					ARC_DEBUG(ERR, "Failed to create resource\n");
					Arc_SlabFree(stat_path);
					return -1;
				}
			}

			skip_res:;

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
