/**
 * @file initramfs.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is apart of Arctan.
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

#ifndef ARC_FS_INITRAMFS_H
#define ARC_FS_INITRAMFS_H

#include <stdint.h>
#include <stddef.h>

/**
 * Load a file from the given CPIO image.
 *
 * The caller provides a pointer to a uin64_t
 * which determines the requested load address
 * of the file.
 *
 * @param char *path - The path of the file to load.
 * @param uint64_t *load_address - The requested address to load it at, set on return.
 * @return 0 for success. */
int load_file(char *path, uint64_t *load_address);

#endif
