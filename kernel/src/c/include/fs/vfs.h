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
#define ARC_VFS_N_LINK  5

#define ARC_VFS_FS_EXT2      1
#define ARC_VFS_FS_INITRAMFS 2

#define ARC_VFS_SEEK_SET 1
#define ARC_VFS_SEEK_CUR 2
#define ARC_VFS_SEEK_END 3

#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <lib/resource.h>

struct ARC_VFSFile {
	/// Current offset into the file.
	size_t offset;
	/// Pointer to the parent VFS node.
	struct ARC_VFSNode *node;
	/// Stat
	struct stat stat;
};

struct ARC_VFSMount {
	/// Type of file system.
	int fs_type;
	/// Pointer to the parent VFS node.
	struct ARC_VFSNode *node;
	/// Number of open files under this mount.
	uint64_t open_files;
};

/**
 * A single node in a VFS tree.
 * */
struct ARC_VFSNode {
	/// Pointer to the device. References can be found through consulting resource.
	struct ARC_Resource *resource;
	/// The type of node.
	int type;
	/// Number of references to this node (> 0 means node and children cannot be destroyed).
	uint64_t ref_count;
	/// The name of this node.
	char *name;
	// Stat
	struct stat stat;
	/// Pointer to the link.
	struct ARC_VFSNode *link;
	/// Pointer to the mount structure this node is or is under.
	struct ARC_VFSMount *mount;
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
struct ARC_VFSNode *Arc_MountVFS(char *mountpoint, char *name, struct ARC_Resource *resource, int type);


/**
 * Unmounts the given mountpoint
 *
 * All nodes under the given node will be destroyed and their
 * resources will be uninitialized and closed.
 *
 * @param struct ARC_VFSNode *mount - The mount to unmount
 * @return zero on success.
 * */
int Arc_UnmountVFS(struct ARC_VFSNode *mount);

/**
 * Open the given file with the given perms.
 *
 * @param char *filepath - Path to the file to open.
 * @param int flags - Flags to open the file with.
 * @param uint32_t mode - Permissions to open the file with.
 * @param struct ARC_Reference **reference - A reference to the resource.
 * @return A non-NULL pointer on success.
 * */
struct ARC_VFSFile *Arc_OpenFileVFS(char *filepath, int flags, uint32_t mode, struct ARC_Reference **reference);

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
int Arc_ReadFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSFile *file);

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
int Arc_WriteFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSFile *file);

/**
 * Change the offset in the given file.
 *
 * @param struct ARC_VFSNode *file - The file in which to seek
 * @param long offset - The offset from whence
 * @param int whence - The position from which to apply the offset to.
 * @return zero on success.
 * */
int Arc_SeekFileVFS(struct ARC_VFSFile *file, long offset, int whence);

/**
 * Close the given file in the VFS.
 *
 * @param struct ARC_VFSNode *file - The file to close.
 * @param struct ARC_Reference *reference - The reference which is closing the file.
 * @return zero on success.
 * */
int Arc_CloseFileVFS(struct ARC_VFSFile *file, struct ARC_Reference *reference);

/**
 * Get the status of a file
 *
 * Returns the status of the file found at the given
 * filepath in the stat.
 *
 * @param char *filepath - The filepath of the file to stat.
 * @param struct stat *stat - The place where to put the information.
 * @return zero on success.
 * */
int Arc_StatFileVFS(char *filepath, struct stat *stat);

int Arc_VFSCreate(char *filepath, uint32_t mode, int type);
int Arc_VFSRemove(char *filepath);
int Arc_VFSLink(char *a, char *b);
int Arc_VFSRename(char *a, char *b);

#endif
