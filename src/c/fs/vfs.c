/**
 * @file vfs.c
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
 * The implementation of the virtual filesystem's behavior.
*/
#include <abi-bits/stat.h>
#include <lib/atomics.h>
#include <abi-bits/errno.h>
#include <lib/resource.h>
#include <mm/slab.h>
#include <fs/vfs.h>
#include <fs/dri_defs.h>
#include <global.h>
#include <lib/util.h>
#include <interface/printf.h>
#include <fs/graph.h>

// TODO: Implement error cases for all functions.

static const char *root = "\0";
static const struct ARC_Resource root_res = { 0 };
static struct ARC_VFSNode vfs_root = { 0 };

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

	if (Arc_vfs_traverse(mountpoint, &info, 0) != 0) {
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

	struct ARC_Mount *mount_struct = (struct ARC_Mount *)Arc_SlabAlloc(sizeof(struct ARC_Mount));

	if (mount_struct == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate mount structure\n");
		Arc_QUnlock(&mount->branch_lock);
		mount->ref_count--; // TODO: Atomize
		return -1;
	}

	mount_struct->node = mount;
	mount_struct->fs_type = fs_type;

	Arc_MutexLock(&mount->property_lock);

	mount->type = ARC_VFS_N_MOUNT;
	mount->resource = resource;
	mount->mount = mount_struct;

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

	int info_ret = Arc_vfs_traverse(path, &info, link_depth);
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
		struct ARC_Resource *res = node->resource;

		if (res == NULL || res->driver->open(desc, res, info.mountpath, 0, mode) != 0) {
			ARC_DEBUG(ERR, "Failed to open file\n");
			return -2;
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
//		ARC_DEBUG(ERR, "One or more is NULL: %p %p\n", res, res->driver->read);
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
	Arc_vfs_delete_node(node, 0);
	Arc_vfs_bottom_up_prune(parent, top);
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

	int ret = Arc_vfs_traverse(path, &info, 0);

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

	int ret = Arc_vfs_traverse(filepath, &info, 0);

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
	Arc_vfs_delete_node(info.node, recurse);
	Arc_vfs_bottom_up_prune(parent, info.mount);

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

	int ret = Arc_vfs_traverse(a, &info_a, 0);

	if (ret != 0) {
		ARC_DEBUG(ERR, "Failed to find %s\n", a);
		return 1;
	}

	struct vfs_traverse_info info_b = { .create_level = VFS_FS_CREAT, .mode = mode, .type = ARC_VFS_N_LINK };
	VFS_DETERMINE_START(info_b, b);

	ret = Arc_vfs_traverse(b, &info_b, 0);

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

	int ret = Arc_vfs_traverse(a, &info_a, 0);

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

	ret = Arc_vfs_traverse(b, &info_b, 0);

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
	// Rename the node
	Arc_SlabFree(node_a->name);
	char *end = (char *)(a + strlen(a) - 1);
	// Advance back to before ending '/', if there is one
	// if a = /imaginary_path/, this ought to place end at the arrow
	//                      ^
	while (end != a && *end == '/') {
		end--;
	}

	int cnt = 1;

	// Advance back to next / or the beginning of string, from the last example:
	// /imaginary_path/
	// ^-----cnt-----^
	while (end != a && *end != '/') {
		end--;
		cnt++;
	}

	// /imaginary_path/
	//  ^-----cnt----^
	end++;
	cnt--;

	node_a->name = strndup(end, cnt);

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

		printf("%s (0x%X, 0x%X B)\n", child->name, child->stat.st_mode, child->stat.st_size);

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

	if (Arc_vfs_traverse(path, &info, 0) != 0) {
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

	int ret = Arc_vfs_traverse(relative_path, &info, 0);

	if (ret == 0) {
		Arc_QUnlock(&info.node->branch_lock);
		info.node->ref_count--; // TODO: Atomize
	}

	if (ret != 0) {
		return NULL;
	}

	return info.node;
}
