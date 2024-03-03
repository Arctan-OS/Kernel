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

#define ARC_VFS_SEEK_SET 1
#define ARC_VFS_SEEK_CUR 2
#define ARC_VFS_SEEK_END 3

#include <stddef.h>
#include <stdint.h>
#include <lib/resource.h>

struct ARC_VFSFile {
	/// Current offset into the file.
	size_t offset;
	/// Size of the file.
	size_t size;
	/// Address of the file on disk.
	void *address;
	/// Pointer to the parent VFS node.
	struct ARC_VFSNode *node;
	/// State required by the file driver.
	void *state;
};

struct ARC_VFSMount {
	/// Type of file system.
	int fs_type;
	/// Address of the superblock on disk.
	void *super_address;
	/// Pointer to the parent VFS node.
	struct ARC_VFSNode *node;
	/// State required by the file system driver.
	void *state;
};

/**
 * A single node in a VFS tree.
 * */
struct ARC_VFSNode {
	/// Pointer to the device. References can be found through consulting resource.
	struct ARC_Resource *resource;
	/// The type of node.
	int type;
	/// The name of this node.
	char *name;
	/// Specific structure (like ARC_VFSFile / ARC_VFSMount)
	void *spec;
	/// Pointer to the parent of the current node.
	struct ARC_VFSNode *parent;
	/// Pointer to the head of the children linked list.
	struct ARC_VFSNode *children;
	/// Pointer to the next element in the current linked list.
	struct ARC_VFSNode *next;
	/// Pointer to the previous element in the current linked list.
	struct ARC_VFSNode *prev;
};

/**
 * Initalize the VFS root.
 *
 * @return 0: success
 * */
int Arc_InitializeVFS();


/**
 * Create a new mounted device under the given mountpoint.
 *
 * i.e. mount A at /mounts/A
 *
 * @param struct ARC_VFSNode *mountpoint - The VFS node under which to mount (/mounts).
 * @param char *name - The name of the mountpoint (A).
 * @param struct ARC_Resource *resource - The resource by which to address the mountpoint.
 * @param int type - The type of the file system.
 * @return A non-NULL pointer to the VFS node that describes the mounted device.
 * */
struct ARC_VFSNode *Arc_MountVFS(struct ARC_VFSNode *mountpoint, char *name, struct ARC_Resource *resource, int type);

/**
 * Open the given file with the given perms.
 *
 * @param char *filepath - Path to the file to open.
 * @param char *perms - Permissions to open the file with.
 * @return A non-NULL pointer on success.
 * */
struct ARC_VFSNode *Arc_OpenFileVFS(char *filepath, int flags, uint32_t mode);

/**
 * Read the given file.
 *
 * Reads /count words of /a size bytes from /a file into
 * /a buffer.
 *
 * @param void *buffer - The buffer into which to read the file data.
 * @param size_t size - The size of each word to read.
 * @param size_t count - The number of words to read.
 * @param struct ARC_VFSNode *file - The file to read.
 * @return The number of words read.
 * */
int Arc_ReadFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file);

/**
 * Write to the given file.
 *
 * Writes /count words of /a size bytes from /a buffer into
 * /a file.
 *
 * @param void *buffer - The buffer from which to read the data.
 * @param size_t size - The size of each word to write.
 * @param size_t count - The number of words to write.
 * @param struct ARC_VFSNode *file - The file to write.
 * @return The number of words written.
 * */
int Arc_WriteFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file);

/**
 * Close the given file in the VFS.
 *
 * @param struct ARC_VFSNode *file - The file to close.
 * */
int Arc_CloseFileVFS(struct ARC_VFSNode *file);

#endif
