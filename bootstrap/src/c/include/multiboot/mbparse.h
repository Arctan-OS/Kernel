/**
 * @file mbparse.h
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
 * Provides a function for reading the multiboot2 boot information tag
 * structure.
*/
#ifndef ARC_MULTIBOOT_MBPARSE_H
#define ARC_MULTIBOOT_MBPARSE_H

#include <mm/freelist.h>

/**
 * Reads the tags provided by boothloader.
 *
 * Populates the _boot_meta, also initializes the
 * PMM.
 *
 * @param void *mb2i - Pointer to the base of the first tag.
 * @return Error code (0: success).
 * */
int read_mb2i(void *mb2i);

#endif
