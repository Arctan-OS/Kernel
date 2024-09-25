/**
 * @file scope.c
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
#include <arch/acpi/caml/parser/scope.h>
#include <arch/acpi/caml/parser/package.h>
#include <arch/acpi/caml/parser/name.h>
#include <arch/acpi/caml/parser/sequencer.h>
#include <mm/allocator.h>
#include <fs/vfs.h>
#include <lib/util.h>
#include <lib/perms.h>
#include <global.h>

int parse_scope(struct ARC_cAMLState *state) {
	size_t length = parse_package_length(state);

	char *name = parse_name_string(state);

	ARC_DEBUG(INFO, "\tPkgLength: %lu\n", length);
	ARC_DEBUG(INFO, "\tName: %s\n", name);

	sequencer_push_scope(length);

	struct ARC_VFSNode *node = strlen(name) > 0 ? vfs_create_rel(name, state->parent, ARC_STD_PERM, ARC_VFS_N_DIR, NULL) : state->parent;

	free(name);

	if (node == NULL) {
		ARC_DEBUG(ERR, "Failed to create new directory\n");
		return -1;
	}

	return 0;
}
