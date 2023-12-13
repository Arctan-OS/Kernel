/*
    Arctan - Operating System Kernel
    Copyright (C) 2023  awewsomegamer

    This file is apart of Arctan.

    Arctan is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ALLOC_H
#define ALLOC_H

// Provide functions which utilize pmm, vmm, and mapper functions
// to allocate an address range.

#include <stdint.h>
#include <stddef.h>

struct free_node {
	struct free_node *next;
};

struct pool_descriptor {
	struct free_node *pool_base;
	struct free_node *pool_head;
	size_t object_size;
};

void *alloc_pages(struct pool_descriptor *pool, size_t pages);
void *free_pages(struct pool_descriptor *pool, void *address, size_t pages);

struct pool_descriptor init_pool(void *base, size_t object_size, size_t objects);

#endif
