/**
 * @file method.c
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
#include <arch/acpi/caml/parser/method.h>
#include <arch/acpi/caml/parser/package.h>
#include <arch/acpi/caml/parser/name.h>
#include <arch/acpi/caml/ops.h>
#include <arch/acpi/caml/parser/sequencer.h>
#include <mm/allocator.h>
#include <lib/util.h>
#include <lib/perms.h>
#include <fs/vfs.h>
#include <global.h>

int parse_method(struct ARC_cAMLState *state) {
	ARC_DEBUG(INFO, "Reading method %p:\n", state->buffer);

	size_t package_size = parse_package_length(state);
	size_t delta = state->max;

	char *name = parse_name_string(state);
	uint8_t flags = *state->buffer;
	ADVANCE_STATE(state);

	delta -= state->max;
	package_size -= delta;

	ARC_DEBUG(INFO, "\tPkgLength: %lu\n", package_size);
	ARC_DEBUG(INFO, "\tName: %s\n", name);
	ARC_DEBUG(INFO, "\tFlags: 0x%x\n", flags);

	if (strlen(name) > 0) {
		struct ARC_VFSNode *node = vfs_create_rel(name, state->parent, ARC_STD_PERM, ARC_VFS_N_BUFF, NULL);
		struct ARC_File *fake = (struct ARC_File *)alloc(sizeof(*fake));

		if (fake == NULL) {
			goto skip;
		}

		memset(fake, 0, sizeof(*fake));

		fake->node = node;
		state->jit_buffer = fake;
		state->jit_limit = package_size;
	} else {
		skip:;
		ARC_DEBUG(ERR, "\tSkipped method\n");
		ADVANCE_STATE_BY(state, package_size);
	}

	free(name);

	return 0;
}
