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
#ifndef ARC_VFS_H
#define ARC_VFS_H

#define ARC_VFS_NULL  0

#define ARC_VFS_N_FILE  1
#define ARC_VFS_N_DIR   2
#define ARC_VFS_N_MOUNT 3
#define ARC_VFS_N_ROOT  4

#define ARC_VFS_FS_EXT2      1
#define ARC_VFS_FS_INITRAMFS 2

#include <stddef.h>

/**
 * A single node in a VFS tree.
 * */
struct ARC_VFSNode {
	/// The ID of the disk (virtual address in case of initramfs).
	void *disk;
	/// The address of this node on disk.
	void *address;
	/// The type of file system.
	int fs_type;
	/// The type of node.
	int type;
	/// The name of this node.
	char *name;
	/// Pointer to the head of the children linked list.
	struct ARC_VFSNode *children;
	/// Pointer to the next element in the current linked list.
	struct ARC_VFSNode *next;
};

typedef int (*VFS_Hook)(struct ARC_VFSNode);

/**
 * A structure containing hooks into a FS driver.
 *
 * Contains pointers to various functions.
 * */
struct ARC_FSDriver {
	void (*find_file)(VFS_Hook hook, char *name);
	void (*open_file)(VFS_Hook hook, char *name);
	void (*close_file)(VFS_Hook hook, void *file);
	void (*read_file)(VFS_Hook hook, void *file, size_t size, size_t count, void *buffer);
	void (*write_file)(VFS_Hook hook, void *file, size_t size, size_t count, void *buffer);
};

/**
 * Initialize the given VFS to be a root.
 *
 * @param struct ARC_VFSNode *vfs - The VFS node to initialize as root
 * @return 0: success
 * */
int Arc_InitializeVFS(struct ARC_VFSNode *vfs);


/**
 * Create a new mounted device under the given mountpoint.
 *
 * i.e. mount A at /mounts/A
 *
 * @param struct ARC_VFSNode *mountpoint - The VFS node under which to mount (/mounts).
 * @param char *name - The name of the mountpoint (A).
 * @param void *disk - The disk the file system is on.
 * @param void *address - The address at which the file system starts on disk.
 * @param int type - The type of the file system.
 * @return A non-NULL pointer to the VFS node that describes the mounted device.
 * */
struct ARC_VFSNode *Arc_MountVFS(struct ARC_VFSNode *mountpoint, char *name, void *disk, void *address, int type);

#endif
