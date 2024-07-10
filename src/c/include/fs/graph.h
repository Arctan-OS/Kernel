/**
 * @file graph.h
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
 * This file handles the management of the VFS's node graph.
*/
#ifndef ARC_VFS_GRAPH_H
#define ARC_VFS_GRAPH_H

#include <stdbool.h>
#include <fs/vfs.h>

#define VFS_DETERMINE_START(info, path) \
        if (*path == '/') { \
		info.start = &vfs_root; \
	} else { \
		ARC_DEBUG(WARN, "Definitely getting current directory\n") \
	}

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

int Arc_vfs_delete_node_recurse(struct ARC_VFSNode *node);
int Arc_vfs_delete_node(struct ARC_VFSNode *node, bool recurse);
int Arc_vfs_bottom_up_prune(struct ARC_VFSNode *bottom, struct ARC_VFSNode *top);
int Arc_vfs_top_down_prune(struct ARC_VFSNode *top, int depth);
int Arc_vfs_traverse(char *filepath, struct vfs_traverse_info *info, int link_depth);

#endif
