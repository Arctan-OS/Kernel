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
#include <fs/initramfs.h>
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

void *Arc_FindFileInitramfs(void *fs, char *filename) {
	struct ARC_HeaderCPIO *header = (struct ARC_HeaderCPIO *)fs;
	uint64_t offset = 0;

	while (header->magic == 0070707) {
		uint16_t namesize = (header->namesize) + (header->namesize % 2);

		char *name = (char *)((uintptr_t)header + sizeof(struct ARC_HeaderCPIO));

		if (strcmp(name, filename) != 0) {
			goto next;
		}

		uint32_t filesize = (header->filesize[0] << 16) | header->filesize[1];
		uint32_t data_off = sizeof(struct ARC_HeaderCPIO) + namesize;

		ARC_DEBUG(INFO, "Found file \"%s\"\n", filename);

		return (void *)(fs + offset + data_off);

next:;
		offset += sizeof(struct ARC_HeaderCPIO) + filesize + namesize;
		header = (struct ARC_HeaderCPIO *)(fs + offset);
	}

	ARC_DEBUG(ERR, "Could not find file \"%s\"\n", filename);

	return NULL;
}
