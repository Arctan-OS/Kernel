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

// TODO: Functions like open, close, read, write, etc... need code to
//       modify the status of the file
// TODO: Internal functions (vfs_*) need to access the physical file
//       system to create / remove directories and files

struct internal_traverse_trace {
	struct ARC_VFSNode *node;
	struct internal_traverse_trace *next;
};

struct internal_traverse_args {
	/// Nodes traveresed to get to target.
	struct internal_traverse_trace *trace;
        /// Node at the end of the trace.
	struct internal_traverse_trace *last_node;
	/// Mountpoint.
	struct ARC_VFSNode *mount;
	/// Absolute path relative to mountpoint.
	char *mountpath;
	/// Number of characters reached before return (equal to strlen(filepath) means last_node = target node).
	size_t success;
};

static int vfs_destroy_node(struct ARC_VFSNode *node) {
	// Destroy the node
	if (node->children != NULL || node->ref_count > 0) {
		// Cannot free node, other nodes / operations depend on it
		return 1;
	}

	Arc_UninitializeResource(node->resource);

	if (node->type == ARC_VFS_N_MOUNT) {
		Arc_SlabFree(node->mount);
	}

	// Update links of parent
	if (node->parent->children != NULL) {
		if (node->prev == NULL) {
			node->parent->children = node->next;
		} else {
			node->prev->next = node->next;
		}

		if (node->next != NULL) {
			node->next->prev = node->prev;
		}
	}

	Arc_SlabFree(node->name);
	Arc_SlabFree(node);

	return 0;
}

static struct ARC_VFSNode *vfs_destroy_node_graph_upto(struct ARC_VFSNode *start, struct ARC_VFSNode *stop) {
	// Destroy the node graph UPTO stop (don't destroy stop)
	while (start != stop) {
		struct ARC_VFSNode *tmp = start->parent;

		if (start->type == ARC_VFS_N_MOUNT) {
			// Ensure that we do not destroy a mount
			return stop;
		}

		if (vfs_destroy_node(start) != 0) {
			return start;
		}

		start = tmp;
	}

	return NULL;
}

static struct ARC_VFSNode *vfs_destroy_node_graph_inclusive(struct ARC_VFSNode *start, struct ARC_VFSNode *stop) {
	// Destroy the node graph UPTO and uncluding stop
	struct ARC_VFSNode *node = vfs_destroy_node_graph_upto(start, stop);

	if (node != NULL) {
		return node;
	}

	if (vfs_destroy_node(stop) != 0) {
		return stop;
	}

	return NULL;
}

static int vfs_destroy_subtrees(struct ARC_VFSNode *top) {
	if (top->ref_count > 0) {
		return 0;
	}

	int count = 0;

	struct ARC_VFSNode *children = top->children;
	while (children != NULL) {
		count += vfs_destroy_subtrees(children);
		children = children->next;
	}

	// Free the node
	Arc_UninitializeResource(top->resource);

	Arc_SlabFree(top->name);

	return count;
}

static int vfs_push_trace(struct internal_traverse_trace *trace) {
	int i = 0;

	struct internal_traverse_trace *cur = trace;

	while (cur != NULL) {
		cur->node->ref_count--;
		i++;
		cur = cur->next;
	}

	return i;
}

static int vfs_pop_trace(struct internal_traverse_trace *trace) {
	int i = 0;

	while (trace != NULL) {
		trace->node->ref_count--;
		i++;
		trace = trace->next;
	}

	return i;
}

static int vfs_free_trace(struct internal_traverse_trace *trace) {
	int i = 0;

	while (trace != NULL) {
		struct internal_traverse_trace *tmp = trace->next;
		Arc_SlabFree(trace);
		i++;
		trace= tmp;
	}

	return i;
}

// TODO: Resolve links
static int vfs_traverse_node_graph(char *filepath, struct ARC_VFSNode *start_node, struct internal_traverse_args *args) {
	if (start_node == NULL) {
		start_node = &vfs_root;
	}

	if (args == NULL) {
		return -1;
	}

	// Create first link of trace
	struct internal_traverse_trace *latest = (struct internal_traverse_trace *)Arc_SlabAlloc(sizeof(struct internal_traverse_trace));

	if (latest == NULL) {
		return -2;
	}

	memset(latest, 0, sizeof(struct internal_traverse_trace));
	latest->node = start_node;
	args->trace = latest;
	args->last_node = latest;

	struct ARC_VFSNode *node = start_node;

	// Lock starting node
	node->ref_count++;

	size_t max = strlen(filepath);

	for (size_t i = 0; i < max; i++) {
		if (filepath[i] != '/') {
			// Component, continue on
			continue;
		}

		// Reached a divisor, check length of component
		size_t j = 0;
		for (; (j + i) < max - 1 && filepath[i + j + 1] != '/'; j++);

		if (j - i <= 0) {
			// No component, continue on
			continue;
		}

		char *component = (char *)(filepath + i + 1);

		if (node->type == ARC_VFS_N_MOUNT) {
			// Last node was a mount, take notes
			args->mount = node;
			args->mountpath = component;
		}

		// Interpret .. and . dirs
		if (strncmp(component, "..", j) == 0) {
			node = node->parent == NULL ? node : node->parent;
			continue;
		} else if (strncmp(component, ".", j) == 0) {
			continue;
		}

		struct ARC_VFSNode *child = node->children;

		while (child != NULL) {
			if (strncmp(component, child->name, j) == 0) {
				// Found next component
				break;
			}

			child = child->next;
		}

		if (child == NULL) {
			// Could not find next component
			args->success = i;
			return 1;
		}

		// Move to next node, lock it
		node = child;
		node->ref_count++;

		// Add a new point to the trace
		latest = (struct internal_traverse_trace *)Arc_SlabAlloc(sizeof(struct internal_traverse_trace));

		if (latest == NULL) {
			goto epic_fail;
		}

		memset(latest, 0, sizeof(struct internal_traverse_trace));

		latest->next = args->trace;
		latest->node = node;
		args->trace = latest;
	}

	return 0;

	epic_fail:;
	// TODO: Account for this condition
	return -2;
}

static int vfs_create_node_graph(char *filepath, struct internal_traverse_args *args, int type) {
	if (filepath == NULL || args == NULL) {
		ARC_DEBUG(ERR, "Invalid filepath or args provided\n");
		return -1;
	}

	ARC_DEBUG(INFO, "Creating node graph for \"%s\"\n", filepath);


	struct ARC_SuperDriverDef *mnt_superdef = NULL;

	if (args->mount != NULL && args->mount->resource != NULL && args->mount->resource->driver != NULL) {
		mnt_superdef = (struct ARC_SuperDriverDef *)args->mount->resource->driver->driver;
	}

	struct ARC_VFSNode *node = args->trace->node;

	size_t max = strlen(filepath);

	for (size_t i = args->success; i < max; i++) {
		if (filepath[i] != '/') {
			continue;
		}

		size_t j = 0;
		for (; (j + i) < max - 1 && filepath[i + j + 1] != '/'; j++);

		if (j - i <= 0) {
			continue;
		}

		char *component = (char *)(filepath + i + 1);

		// Interpret .. and . dirs
		if (strncmp(component, "..", j) == 0) {
			node = node->parent == NULL ? node : node->parent;
			continue;
		} else if (strncmp(component, ".", j) == 0) {
			continue;
		}

		struct ARC_VFSNode *_node = (struct ARC_VFSNode *)Arc_SlabAlloc(sizeof(struct ARC_VFSNode));

		if (_node == NULL) {
			args->success = i;
			return -2;
		}

		memset(_node, 0, sizeof(struct ARC_VFSNode));

		_node->type = ARC_VFS_N_DIR;
		_node->parent = node;

		if (node->children != NULL) {
			_node->next = node->children;
		}
		node->children = _node;

		_node->mount = node->mount;

		char c = filepath[j + i + 1];
		filepath[j + i + 1] = 0;

		_node->name = strdup(component);

		if (mnt_superdef != NULL) {
			mnt_superdef->stat(args->mount->resource, filepath + args->success + 1, &_node->stat);
		}

		filepath[j + i + 1] = c;

		struct internal_traverse_trace *latest = (struct internal_traverse_trace *)Arc_SlabAlloc(sizeof(struct internal_traverse_trace));
		latest->next = args->trace;
		latest->node = _node;
		args->trace = latest;

		node = _node;
		node->ref_count++;
	}

	node->type = type;

	return 0;
}

int Arc_InitializeVFS() {
	vfs_root.name = (char *)root;
	vfs_root.children = NULL;
	vfs_root.next = NULL;
	vfs_root.parent = NULL;
	vfs_root.resource = (struct ARC_Resource *)&root_res;
	vfs_root.type = ARC_VFS_N_ROOT;
	vfs_root.mount = NULL;

	ARC_DEBUG(INFO, "Created VFS root\n");

	return 0;
}

struct ARC_VFSNode *Arc_MountVFS(char *mountpath, char *name, struct ARC_Resource *resource, int type) {
	if (mountpath == NULL || name == NULL || resource == NULL || type == ARC_VFS_NULL) {
		ARC_DEBUG(ERR, "Insufficient arguments (mountpath, name, resource, or type)\n");
		return NULL;
	}

	struct internal_traverse_args args = { 0 };

	int ret = vfs_traverse_node_graph(mountpath, NULL, &args);

	if (ret == 1) {
		ARC_DEBUG(ERR, "Could not find path \"%s\"\n", mountpath);
		return NULL;
	}

	struct ARC_VFSNode *mountpoint = args.last_node->node;

	if (mountpoint == NULL) {
		ARC_DEBUG(ERR, "No mountpoint found\n");
		return NULL;
	}

	struct ARC_VFSNode *node = (struct ARC_VFSNode *)Arc_SlabAlloc(sizeof(struct ARC_VFSNode));

	if (node == NULL) {
		ARC_DEBUG(ERR, "Could not allocate node\n");
		return NULL;
	}

	struct ARC_VFSMount *mount = (struct ARC_VFSMount *)Arc_SlabAlloc(sizeof(struct ARC_VFSMount));

	if (mount == NULL) {
		ARC_DEBUG(ERR, "Could not allocate mount structure\n");
		Arc_SlabFree(node);
		return NULL;
	}

	memset(node, 0, sizeof(struct ARC_VFSNode));

	mount->node = node;
	mount->fs_type = type;

	node->mount = mount;

	node->name = strdup(name);
	node->parent = mountpoint;

	if (mountpoint->children != NULL) {
		mountpoint->children->prev = node;
	}

	node->next = mountpoint->children;
	mountpoint->children = node;
	node->type = ARC_VFS_N_MOUNT;
	node->resource = resource;

	vfs_pop_trace(args.trace);
	vfs_free_trace(args.trace);

	ARC_DEBUG(INFO, "Mounted \"%s\" (res: %p) on \"%s\"\n", name, resource, mountpath);

	return node;
}

int Arc_UnmountVFS(struct ARC_VFSNode *mount) {
	if (mount->type != ARC_VFS_N_MOUNT) {
		ARC_DEBUG(ERR, "Given node is not a mount\n");
		return EINVAL;
	}

	if (mount->mount->open_files > 0 || mount->ref_count > 0) {
		ARC_DEBUG(ERR, "Mount is still in use\n");
		return 1;
	}

	// Destroy all nodes under mount
	vfs_destroy_subtrees(mount);

	// Destroy mount
	vfs_destroy_node(mount);

	return 0;
}

// TODO: Handle potential relative path
struct ARC_VFSFile *Arc_OpenFileVFS(char *filepath, int flags, uint32_t mode, struct ARC_Reference **reference) {
	if (filepath == NULL || reference == NULL) {
		ARC_DEBUG(ERR, "No filepath or reference fillable provided\n");
		return NULL;
	}

	ARC_DEBUG(INFO, "Opening file \"%s\"\n", filepath);

	if (filepath[0] != '/') {
		ARC_DEBUG(ERR, "\"%s\" is not absolute, cannot open (for now)\n", filepath);
		return NULL;
	}

	struct internal_traverse_args args = { 0 };
	int ret = vfs_traverse_node_graph(filepath, NULL, &args);

	if (ret < 0) {
		ARC_DEBUG(ERR, "Could not traverse node graph\n");
		return NULL;
	}

	struct ARC_VFSFile *file = (struct ARC_VFSFile *)Arc_SlabAlloc(sizeof(struct ARC_VFSFile));

	if (file == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate file descriptor\n");
		goto end;
	}

	struct ARC_VFSNode *node = args.trace->node;

	if (node->type == ARC_VFS_N_LINK && node->link != NULL) {
		ARC_DEBUG(INFO, "Found node is a link to %p\n", node->link);
		node = node->link;
	}

	struct ARC_VFSNode *mount = args.mount;
	struct ARC_Resource *res = mount->resource;

	if (ret == 1) {
		// Create path to node
		struct ARC_VFSNode *last = node;
		int ret = vfs_create_node_graph(filepath, &args, ARC_VFS_N_FILE);
		node = args.trace->node;

		if (ret == -2) {
			node_graph_fail:;
			ARC_DEBUG(INFO, "Failed to create node graph\n");

			vfs_pop_trace(args.trace);
			vfs_free_trace(args.trace);

			Arc_SlabFree(file);

			vfs_destroy_node_graph_upto(node, last);

			return NULL;
		} else if (ret == -1) {
			Arc_SlabFree(file);
			file = NULL;

			goto end;
		}

		if (mount == NULL || res == NULL) {
			ARC_DEBUG(INFO, "Failed to find mount information\n");
			goto node_graph_fail;
		}

		struct ARC_Resource *nres = Arc_InitializeResource(filepath, res->dri_group, res->dri_index + 1, res->driver_state);

		nres->vfs_state = file;
		node->resource = nres;

		if (nres->driver->open(nres, args.mountpath, flags, mode) != 0) {
			ARC_DEBUG(ERR, "Driver failed to open file\n");
			goto node_graph_fail;
		}
	}

	// struct ARC_SuperDriverDef *def = res->driver->driver;
	// struct stat stat;
	// def->stat(res, args.mountpath, &stat);
 	// TODO: Check permissions, ensure user has perms to open file with requested flags
 	// TODO: Account for already open files and existing stat

	// Create reference
	*reference = Arc_ReferenceResource(node->resource);

	// Link
	file->node = node;

	// Note mount file
	node->mount->open_files++;

	ARC_DEBUG(INFO, "Opened file (node: %p, fd: %p)\n", node, file);

	end:;
	// Free up trace
	vfs_pop_trace(args.trace);
	vfs_free_trace(args.trace);

	return file;
}

int Arc_ReadFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSFile *file) {
	if (buffer == NULL || file == NULL) {
		return 0;
	}

	if (size == 0 || count == 0) {
		return 0;
	}

	// Lock
	struct ARC_Resource *res = file->node->resource;
	struct ARC_VFSFile *tmp = res->vfs_state;
	res->vfs_state = file;
	int ret = res->driver->read(buffer, size, count, res);
	res->vfs_state = tmp;
	// Unlock

	return ret;
}

int Arc_WriteFileVFS(void *buffer, size_t size, size_t count, struct ARC_VFSFile *file) {
	if (buffer == NULL || file == NULL) {
		return 0;
	}

	if (size == 0 || count == 0) {
		return 0;
	}

	// Lock
	struct ARC_Resource *res = file->node->resource;
	struct ARC_VFSFile *tmp = res->vfs_state;
	res->vfs_state = file;
	int ret = res->driver->write(buffer, size, count, res);
	res->vfs_state = tmp;
	// Unlock

	return ret;
}

int Arc_SeekFileVFS(struct ARC_VFSFile *file, long offset, int whence) {
	if (file == NULL) {
		ARC_DEBUG(INFO, "No file given (seek)\n");
		return EINVAL;
	}

	struct ARC_VFSNode *node = file->node;

	// Lock
	struct ARC_Resource *res = node->resource;
	struct ARC_VFSFile *tmp = res->vfs_state;
	res->vfs_state = file;
	int ret = res->driver->seek(res, offset, whence);
	res->vfs_state = tmp;
	// Unlock

	return ret;
}

int Arc_CloseFileVFS(struct ARC_VFSFile *file, struct ARC_Reference *reference) {
	if (file == NULL || reference == NULL) {
		ARC_DEBUG(ERR, "File or reference is NULL\n");
		return EINVAL;
	}

	Arc_UnreferenceResource(reference);
	Arc_SlabFree(file);

	if (reference->resource->ref_count > 0) {
		// Some other files are still using this file,
		// no need to close it, just free the given file
		// descriptor
		return 0;
	}

	// This file is no longer used, this resource can be
	// uninitialized
	Arc_UninitializeResource(reference->resource);

	// Try to destroy the tree up to the root (don't destroy it
	// even if you can)
	vfs_destroy_node_graph_upto(file->node, &vfs_root);

	return 0;
}

int Arc_StatFileVFS(char *filepath, struct stat *stat) {
	struct internal_traverse_args args = { 0 };
	int ret = vfs_traverse_node_graph(filepath, NULL, &args);

	if (ret < 0) {
		return ret;
	}

	struct ARC_Resource *res = args.mount->resource;
	struct ARC_SuperDriverDef *super_def = (struct ARC_SuperDriverDef *)res->driver->driver;

	super_def->stat(res, args.mountpath, stat);

	vfs_pop_trace(args.trace);
	vfs_free_trace(args.trace);

	return 0;
}

int Arc_VFSCreate(char *filepath, uint32_t mode, int type) {
	struct internal_traverse_args args = { 0 };
	int ret = vfs_traverse_node_graph(filepath, NULL, &args);

	if (ret == 0) {
		// Node is already in VFS, return 0
		vfs_pop_trace(args.trace);
		vfs_free_trace(args.trace);

		return 0;
	}

	if (ret < 0) {
		// Failed to traverse node graph
		return ret;
	}

	ret = vfs_create_node_graph(filepath, &args, type);

	if (ret != 0) {
		// Error
		return ret;
	}

	struct ARC_VFSNode *node = args.trace->node;

	// Set times
	node->stat.st_mode = mode;

	return 0;
}

int Arc_VFSRemove(char *filepath) {
	if (filepath == NULL) {
		return 1;
	}

	struct internal_traverse_args args = { 0 };
	int ret = vfs_traverse_node_graph(filepath, NULL, &args);

	if (ret == 0) {
		vfs_pop_trace(args.trace);
		vfs_free_trace(args.trace);

		return 1;
	} else if (ret < 0) {
		return 1;
	}

	// File is most definitely not open, and we did not get here
	// because we failed to traverse the graph

	if (args.mount == NULL || args.mount->resource == NULL || args.mount->resource->driver == NULL) {
		vfs_pop_trace(args.trace);
		vfs_free_trace(args.trace);

		return 1;
	}

	struct ARC_DriverDef *def = args.mount->resource->driver;
	struct ARC_SuperDriverDef *super_def = (struct ARC_SuperDriverDef *)def->driver;

	ret = super_def->remove(filepath);

	vfs_pop_trace(args.trace);
	vfs_free_trace(args.trace);

	return ret;
}

int Arc_VFSLink(char *a, char *b) {
	if (a == NULL || b == NULL) {
		return 1;
	}

	ARC_DEBUG(INFO, "Linking \"%s\" with \"%s\"\n", a, b)

	struct internal_traverse_args args_a = { 0 };
	struct internal_traverse_args args_b = { 0 };

	int ret_a = vfs_traverse_node_graph(a, NULL, &args_a);
	int ret_b = vfs_traverse_node_graph(b, NULL, &args_b);

	if (ret_a < 0 || ret_b < 0) {
		// Looking up A or B has failed, cannot continue
		goto epic_fail;
	}

	if (ret_a == 1) {
		// Source to link to does not exist
		// TODO: Open it so it exists in VFS context
		goto epic_fail;
	}

	if (ret_b == 1) {
		// NOTE: Probably should have a custom type for hard links
		ret_b = vfs_create_node_graph(b, &args_b, ARC_VFS_N_LINK);
	}

	if (ret_b != 0) {
		// Failed to create file
		goto epic_fail;
	}

	args_b.trace->node->link = args_a.trace->node;
	ARC_DEBUG(INFO, "Created link\n");

	return 0;

	epic_fail:;
	ARC_DEBUG(ERR, "Failed to create link\n");

	vfs_pop_trace(args_a.trace);
	vfs_pop_trace(args_b.trace);
	vfs_free_trace(args_a.trace);
	vfs_free_trace(args_b.trace);

	return 1;
}

int Arc_VFSRename(char *a, char *b) {
	// TODO: Implement
	(void)a;
	(void)b;
	return 0;
}
