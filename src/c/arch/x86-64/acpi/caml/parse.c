/**
 * @file parse.c
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
#include <arch/x86-64/acpi/caml/parse.h>
#include <arch/x86-64/acpi/caml/ops.h>
#include <fs/vfs.h>
#include <global.h>
#include <mm/slab.h>
#include <util.h>

#define ADVANCE_STATE(state) state->buffer++; state->max--;
#define ADVANCE_STATE_BY(state, cnt) state->buffer += cnt; state->max -= cnt;
#define CHECK_STATE(state) \
        if (state->buffer == NULL || state->max <= 0 || state->current == NULL || state->root == NULL) { \
		ARC_DEBUG(ERR, "Invalid state detected! (%p, %ld, %p, %p)\n", state->buffer, state->max, state->current, state->root); \
		return -1; \
	}

struct caml_state {
	uint8_t *buffer;
	size_t max;
	struct ARC_VFSNode *root;
	struct ARC_VFSNode *current;
};

size_t cAML_ParsePkgLength(struct caml_state *state) {
	uint8_t pkglead = *state->buffer;
	uint8_t count = (pkglead >> 6) & 0b11;
	size_t package_size = pkglead & 0x3F;

	ADVANCE_STATE(state);

	if (count > 0) {
		package_size = pkglead & 0xF;
		for (int i = 0; i < count; i++) {
			package_size |= *state->buffer << (i * 8 + 4);
			ADVANCE_STATE(state);
		}
	}

	// Exclude size of PkgLength header, as buffer
	// has already been advanced over it
	return package_size - (count + 1);
}

int cAML_ParseDataObject(struct caml_state *state) {
	return 0;
}

int cAML_ParseDataRefObject(struct caml_state *state) {
	return 0;
}

char *cAML_ParseNamestring(struct caml_state *state) {
	struct ARC_VFSNode *parent = state->current;
	char *name = NULL;

	if (*state->buffer == ROOT_CHR) {
		printf("Root character\n");
		parent = state->root;
		ADVANCE_STATE(state);
	}

	while (*state->buffer == PARENT_PREFIX_CHR) {
		parent = parent->parent;
		ADVANCE_STATE(state);
	}

	switch (*state->buffer) {
	case DUAL_NAME_PREFIX: {
		ADVANCE_STATE(state);
		name = Arc_SlabAlloc(8);
		memcpy(name, state->buffer, 8);
		ADVANCE_STATE_BY(state, 8);

		break;
	}

	case MULTI_NAME_PREFIX: {
		ADVANCE_STATE(state);
		uint8_t length = *(state->buffer);
		name = Arc_SlabAlloc(length * 4);
		memcpy(name, state->buffer, length * 4);
		ADVANCE_STATE_BY(state, length * 4);

		break;
	}

	default: {
		name = Arc_SlabAlloc(4);
		memcpy(name, state->buffer, 4);

		ADVANCE_STATE_BY(state, 4);

		break;
	}
	}

	return name;
}

int cAML_ParsePackage(struct caml_state *state) {
	uint8_t lead = *state->buffer;
	size_t org_max = state->max;

	ADVANCE_STATE(state);

	switch (lead) {
	case SCOPE_OP: {
		size_t package_size = cAML_ParsePkgLength(state);

		ARC_DEBUG(INFO, "Reading scope %p: Lead(0x%X) PkgLength(0x%X)\n", state->buffer, lead, package_size);

		ADVANCE_STATE_BY(state, package_size);

		break;
	}

	case NAME_OP: {
		ARC_DEBUG(INFO, "Reading name %p: Lead(0x%X)\n", state->buffer, lead);

		char *name = cAML_ParseNamestring(state);
		ARC_DEBUG(INFO, "\tName: %s\n", name);

		Arc_RelNodeCreateVFS(name, state->current, 0, ARC_VFS_N_DIR);

		org_max = state->max;

		break;
	}

	default: {
		ARC_DEBUG(ERR, "Unhandled lead 0x%X\n", lead);
	}
	}

	return org_max - state->max;
}

int cAML_ParseDefinitionBlock(uint8_t *buffer, size_t size) {
	struct caml_state *state = (struct caml_state *)Arc_SlabAlloc(sizeof(struct caml_state));

	state->root = Arc_GetNodeVFS("/dev/acpi/", 0);
	state->current = state->root;
	state->buffer = buffer;
	state->max = size;

	CHECK_STATE(state);

	ARC_DEBUG(INFO, "Parsing definition block %p (%d B)\n", buffer, size);

	while (cAML_ParsePackage(state) > 0) {
		CHECK_STATE(state);
	}

	Arc_SlabFree(state);

	return 0;
}
