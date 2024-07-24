/**
 * @file vmm.c
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
#include <mm/vmm.h>
#include <mm/buddy.h>
#include <arch/x86-64/pager.h>

static struct ARC_BuddyMeta vmm_meta = { 0 };

void *vmm_alloc(size_t size) {
	void *virtual = buddy_alloc(&vmm_meta, size);

	if (pager_map((uintptr_t)virtual, 0, size, 0) != 0) {
		buddy_free(&vmm_meta, virtual);

		return NULL;
	}

	return virtual;
}

void *vmm_free(void *address) {
	size_t freed = buddy_free(&vmm_meta, address);

	if (freed == 0) {
		return NULL;
	}

	if (pager_unmap((uintptr_t)address, freed) != 0) {
		return NULL;
	}

	return address;
}

int init_vmm(void *addr, size_t size) {
	return init_buddy(&vmm_meta, addr, size, 12);
}
