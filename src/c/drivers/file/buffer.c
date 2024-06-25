/**
 * @file buffer.c
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
 * Driver for RAM files or buffers which are accesible by the VFS.
*/
#include <lib/resource.h>
#include <global.h>
#include <mm/slab.h>
#include <util.h>

struct buffer_dri_state {
	size_t size;
	void *buffer;
};

int buffer_init(struct ARC_Resource *res, void *arg) {
	size_t size = *(size_t *)arg;
	struct buffer_dri_state *state = (struct buffer_dri_state *)Arc_SlabAlloc(sizeof(struct buffer_dri_state));
	state->buffer = (void *)Arc_SlabAlloc(size);
	state->size = size;

	res->driver_state = state;

	return 0;
}

int buffer_uninit(struct ARC_Resource *res) {
	struct buffer_dri_state *state = (struct buffer_dri_state *)res->driver_state;

	Arc_SlabFree(state->buffer);
	Arc_SlabFree(state);

	return 0;
}

int buffer_empty() {
	return 0;
}

int buffer_open(struct ARC_File *file, struct ARC_Resource *res, char *path, int flags, uint32_t mode) {
	(void)mode;
	(void)flags;

	struct buffer_dri_state *state = (struct buffer_dri_state *)res->driver_state;

	file->node->stat.st_size = state->size;

	return 0;
}

int buffer_read(void *buffer, size_t size, size_t count, struct ARC_File *file, struct ARC_Resource *res) {
	if (buffer == NULL || size == 0 || count == 0 || file == NULL || res == NULL) {
		return -1;
	}

	// Calculate the number of bytes the caller wants
	// in terms of size
	size_t wanted = file->offset + size * count;
	// Calculate the difference between the wanted and the
	// actual size of the file
	size_t delta = wanted - (size_t)file->node->stat.st_size;
	// If the delta is greater than 0, take off delta bytes
	// from the wanted, and give it to the caller, otherwise
	// just give the caller what it wanted
	size_t given = delta > 0 ? wanted - delta : wanted;

	// Do the actual giving
	memcpy(buffer, res->driver_state + file->offset, given);

	return given;
}

int buffer_write(void *buffer, size_t size, size_t count, struct ARC_File *file, struct ARC_Resource *res) {
	if (buffer == NULL || size == 0 || count == 0 || file == NULL || res == NULL) {
		return -1;
	}


	// Calculate the number of bytes the caller wants
	// in terms of size
	size_t wanted = size * count;
	// Calculate the difference between the wanted and the
	// actual size of the file
	size_t delta = wanted - ((size_t)file->node->stat.st_size + file->offset);
	// If the delta is greater than 0, take off delta bytes
	// from the wanted, and give it to the caller, otherwise
	// just give the caller what it wanted
	size_t given = delta > 0 ? wanted - delta : wanted;

	// Do the actual receiving
	memcpy(res->driver_state + file->offset, buffer, given);

	return given;
}

int buffer_seek(struct ARC_File *file, struct ARC_Resource *res, long offset, int whence) {
	(void)res;

	if (file == NULL) {
		return 1;
	}

	long size = file->node->stat.st_size;

	switch (whence) {
	case ARC_VFS_SEEK_SET: {
		if (offset < size) {
			file->offset = offset;
		}

		return 0;
	}

	case ARC_VFS_SEEK_CUR: {
		file->offset += offset;

		if (file->offset >= size) {
			file->offset = size;
		}

		return 0;
	}

	case ARC_VFS_SEEK_END: {
		file->offset = size - offset - 1;

		if (file->offset < 0) {
			file->offset = 0;
		}

		return 0;
	}
	}
	return 0;
}

ARC_REGISTER_DRIVER(0, buffer) = {
        .index = 5,
	.init = buffer_init,
	.uninit = buffer_uninit,
	.open = buffer_open,
	.read = buffer_read,
	.write = buffer_write,
	.seek = buffer_seek,
};
