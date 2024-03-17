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
#include "fs/vfs.h"
#include <lib/resource.h>
#include <global.h>
#include <util.h>

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

int initramfs_empty() {
	return 0;
}

int initramfs_read(void *buffer, size_t size, size_t count, struct ARC_VFSNode *file) {
	ARC_DEBUG(INFO, "Reading file %p into buffer %p (%d %d)\n", file, buffer, size, count);

	return 0;
}

int initramfs_write() {
	ARC_DEBUG(INFO, "Read only file system\n");

	return 1;
}

int initramfs_seek(struct ARC_VFSNode *file, long offset, int whence) {
	ARC_DEBUG(INFO, "Seeking initramfs file to %ld bytes from %d", offset, whence);

	return 0;
}

ARC_REGISTER_DRIVER(0, initramfs_super) = {
	.index = 0,
	.init = initramfs_empty,
	.uninit = initramfs_empty,
	.open = initramfs_empty,
	.close = initramfs_empty,
	.read = initramfs_read,
	.write = initramfs_write,
	.seek = initramfs_seek,
};

ARC_REGISTER_DRIVER(0, initramfs_file) = {
	.index = 1,
	.init = initramfs_empty,
	.uninit = initramfs_empty,
	.open = initramfs_empty,
	.close = initramfs_empty,
	.read = initramfs_read,
	.write = initramfs_write,
	.seek = initramfs_seek,
};

void *Arc_FindFileInitramfs(void *fs, char *filename) {
	struct ARC_HeaderCPIO *header = (struct ARC_HeaderCPIO *)fs;
	uint64_t offset = 0;

	while (header->magic == 0070707) {
		uint16_t namesize = (header->namesize) + (header->namesize % 2);

		char *name = (char *)((uintptr_t)header + sizeof(struct ARC_HeaderCPIO));

		uint32_t filesize = (header->filesize[0] << 16) | header->filesize[1];
		uint32_t data_off = sizeof(struct ARC_HeaderCPIO) + namesize;

		if (strcmp(name, filename) != 0) {
			goto next;
		}

		ARC_DEBUG(INFO, "Found file \"%s\"\n", filename);

		return (void *)(fs + offset + data_off);

next:;
		offset += sizeof(struct ARC_HeaderCPIO) + filesize + namesize;
		header = (struct ARC_HeaderCPIO *)(fs + offset);
	}

	ARC_DEBUG(ERR, "Could not find file \"%s\"\n", filename);

	return NULL;
}
