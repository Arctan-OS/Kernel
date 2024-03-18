/**
 * @file initramfs.c
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
*/
#include <abi-bits/errno.h>
#include "lib/perms.h"
#include <fs/vfs.h>
#include <mm/allocator.h>
#include <lib/resource.h>
#include <global.h>
#include <util.h>

#define ARC_NAME_OFFSET (sizeof(struct ARC_HeaderCPIO))
#define ARC_NAME_SIZE(header) (header->namesize + (header->namesize & 1))
#define ARC_DATA_OFFSET(header) (ARC_NAME_OFFSET + ARC_NAME_SIZE(header))
#define ARC_DATA_SIZE(header) (((header->filesize[0] << 16) | header->filesize[1]) + (((header->filesize[0] << 16) | header->filesize[1]) & 1))

struct ARC_HeaderCPIO {
	uint16_t magic;
	uint16_t device;
	uint16_t inode;
	uint16_t mode;
	uint16_t uid;
	uint16_t gid;
	uint16_t nlink;
	uint16_t rdev;
	uint16_t mod_time[2];
	uint16_t namesize;
	uint16_t filesize[2];
}__attribute__((packed));

void *Arc_FindFileInitramfs(void *fs, char *filename) {
	if (fs == NULL || filename == NULL) {
		ARC_DEBUG(ERR, "Either fs %p or filename %p is NULL\n", fs, filename);
		return NULL;
	}

	struct ARC_HeaderCPIO *header = (struct ARC_HeaderCPIO *)fs;
	uint64_t offset = 0;

	while (header->magic == 0070707) {
		char *name = ((char *)header) + ARC_NAME_OFFSET;

		if (strcmp(name, filename) != 0) {
			goto next;
		}

		ARC_DEBUG(INFO, "Found file \"%s\"\n", name);

		return (void *)header;

		next:;
		offset += ARC_DATA_OFFSET(header) + ARC_DATA_SIZE(header);
		header = (struct ARC_HeaderCPIO *)(fs + offset);

	}

	ARC_DEBUG(ERR, "Could not find file \"%s\"\n", filename);

	return NULL;
}

int initramfs_empty() {
	return 0;
}

int initramfs_open(struct ARC_VFSNode *file, int flags, uint32_t mode) {
	if (Arc_CheckCurPerms(mode) != 0) {
		return EPERM;
	}

	struct ARC_VFSFile *spec = file->spec;

	struct ARC_HeaderCPIO *header = Arc_FindFileInitramfs(file->resource->args, file->resource->name);

	if (header == NULL) {
		ARC_DEBUG(ERR, "Failed to open file\n");

		return 1;
	}

	spec->size = (header->filesize[0] << 16) | header->filesize[1];
	spec->address = (void *)header;

	return 0;
}

int initramfs_read(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	struct ARC_VFSFile *spec = file->spec;

	if (spec->address == NULL) {
		return 0;
	}

	struct ARC_HeaderCPIO *header = (struct ARC_HeaderCPIO *)spec->address;

	uint8_t *data = (uint8_t *)(spec->address + ARC_DATA_OFFSET(header));

	// Copy file data to buffer
	for (size_t i = 0; i < size * count; i++) {
		uint8_t value = 0;

		if (i + spec->offset < spec->size) {
			value = *((uint8_t *)(data + spec->offset + i));
		}

		*((uint8_t *)(buffer + i)) = value;
	}

	return count;
}

int initramfs_write() {
	ARC_DEBUG(ERR, "Read only file system\n");

	return 1;
}

int initramfs_seek(struct ARC_VFSNode *file, long offset, int whence) {
	struct ARC_VFSFile *spec = file->spec;

	switch (whence) {
	case ARC_VFS_SEEK_SET: {
		spec->offset = offset;

		return 0;
	}

	case ARC_VFS_SEEK_CUR: {
		spec->offset += offset;

		if (spec->offset >= spec->size) {
			spec->offset = spec->size;
		}

		return 0;
	}

	case ARC_VFS_SEEK_END: {
		spec->offset = spec->size - offset - 1;

		if (spec->offset < 0) {
			spec->offset = 0;
		}

		return 0;
	}
	}

	return 0;
}

int initramfs_stat(char *filename) {
	return 0;
}

ARC_REGISTER_DRIVER(0, initramfs_super) = {
	.index = 0,
	.init = initramfs_empty,
	.uninit = initramfs_empty,
	.open = initramfs_open,
	.close = initramfs_empty,
	.read = initramfs_read,
	.write = initramfs_write,
	.seek = initramfs_seek,
	.stat = initramfs_stat,
	.mount = initramfs_empty,
	.unmount = initramfs_empty
};

ARC_REGISTER_DRIVER(0, initramfs_file) = {
	.index = 1,
	.init = initramfs_empty,
	.uninit = initramfs_empty,
	.open = initramfs_open,
	.close = initramfs_empty,
	.read = initramfs_read,
	.write = initramfs_write,
	.seek = initramfs_seek,
	.stat = initramfs_empty,
	.mount = initramfs_empty,
	.unmount = initramfs_empty
};
