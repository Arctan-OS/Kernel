/**
 * @file name.c
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
#include <arch/acpi/caml/parser/name.h>
#include <arch/acpi/caml/ops.h>
#include <arch/acpi/caml/parser/sequencer.h>
#include <mm/allocator.h>
#include <lib/util.h>
#include <fs/vfs.h>
#include <global.h>

char *parse_name_string(struct ARC_cAMLState *state) {
	struct ARC_VFSNode *parent = state->parent;
	char *name = NULL;

	if (*state->buffer == ROOT_CHR) {
		parent = state->root;
		ADVANCE_STATE(state);
	}

	while (*state->buffer == PARENT_PREFIX_CHR) {
		parent = parent->parent;
		ADVANCE_STATE(state);
	}

	state->parent = parent;

	switch (*state->buffer) {
		case DUAL_NAME_PREFIX: {
			ADVANCE_STATE(state);
			name = alloc(10);
			name[9] = 0;
			memcpy(name, state->buffer, 4);
			memcpy(name + 5, state->buffer + 4, 4);
			name[4] = '/';
			ADVANCE_STATE_BY(state, 8);

			break;
		}

		case MULTI_NAME_PREFIX: {
			ADVANCE_STATE(state);
			uint8_t length = *(state->buffer);
			name = alloc(length * 5);
			for (int i = 0; i < length; i++) {
				memcpy(name + i * 5, state->buffer + i * 4, 4);
				name[i * 4] = '/';
			}
			name[length * 5] = 0;
			ADVANCE_STATE_BY(state, length * 4);

			break;
		}

		case 0x00: {
			// NullName
			// NOTE: This is a bit wasteful, but it allows the
			//       caller to blindly free anything that has been
			//       returned
			ADVANCE_STATE(state);
			name = alloc(1);
			*name = 0;

			break;
		}

		default: {
			name = alloc(5);
			name[4] = 0;
			memcpy(name, state->buffer, 4);
			ADVANCE_STATE_BY(state, 4);

			break;
		}
	}

	return name;
}
