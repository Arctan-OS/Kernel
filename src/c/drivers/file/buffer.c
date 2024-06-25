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

int buffer_init(struct ARC_Resource *res, void *arg) {
	ARC_DEBUG(INFO, "Creating a buffer\n");
	return 0;
}

int buffer_empty() {
	return 0;
}

int buffer_read() {
	return 0;
}

int buffer_write() {
	return 0;
}

int buffer_seek() {
	return 0;
}

ARC_REGISTER_DRIVER(0, buffer) = {
        .index = 5,
	.read = buffer_read,
	.write = buffer_write,
	.seek = buffer_seek,
};
