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
#include <mm/allocator.h>
#include <stdint.h>
#include <lib/util.h>

#define ADVANCE_STATE(state) state->buffer++; state->max--;
#define REGRESS_STATE(state) state->buffer--; state->max++;
#define ADVANCE_STATE_BY(state, cnt) state->buffer += cnt; state->max -= cnt;
#define REGRESS_STATE_BY(state, cnt) state->buffer -= cnt; state->maxt += cnt;
#define CHECK_STATE(state, action) \
		if (state->buffer == NULL || state->max <= 0 || state->current == NULL || state->root == NULL) { \
		ARC_DEBUG(ERR, "Invalid state detected! (%p, %ld, %p, %p)\n", state->buffer, state->max, state->current, state->root); \
		action; \
	}

#define CALL_CHECK(call) if (call == 0) { return 0; }

struct caml_state {
	uint8_t *buffer;
	size_t max;
	struct ARC_VFSNode *root;
	struct ARC_VFSNode *current;
	uint64_t uret; // Bytes, Words, Dwords, Qwords, Zero, One, Ones
	void *pret; // Pointers
};

int caml_parse_pkg(struct caml_state *state);

size_t caml_parse_pkg_len(struct caml_state *state) {
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

int caml_parse_computational_data(struct caml_state *state) {
	uint8_t lead = *state->buffer;

	ADVANCE_STATE(state);

	switch (lead) {
		case ZERO_OP:
		case ONE_OP:
		case ONES_OP: {
			state->uret = lead;

			return 0;
		}

		case BYTE_PREFIX: {
			state->uret = *state->buffer;
			ADVANCE_STATE(state);

			return 0;
		}

		case WORD_PREFIX: {
			state->uret = *state->buffer;
			ADVANCE_STATE(state);
			state->uret |= *state->buffer << 8;
			ADVANCE_STATE(state);

			return 0;
		}

		case DWORD_PREFIX: {
			state->uret = *state->buffer;
			ADVANCE_STATE(state);
			state->uret |= *state->buffer << 8;
			ADVANCE_STATE(state);
			state->uret |= *state->buffer << 16;
			ADVANCE_STATE(state);
			state->uret |= *state->buffer << 24;
			ADVANCE_STATE(state);

			return 0;
		}

		case QWORD_PREFIX: {
			state->uret = *state->buffer;
			ADVANCE_STATE(state);
			state->uret |= *state->buffer << 8;
			ADVANCE_STATE(state);
			state->uret |= *state->buffer << 16;
			ADVANCE_STATE(state);
			state->uret |= *state->buffer << 24;
			ADVANCE_STATE(state);
			state->uret = (uint64_t)(*state->buffer) << 32;
			ADVANCE_STATE(state);
			state->uret |= (uint64_t)*state->buffer << 40;
			ADVANCE_STATE(state);
			state->uret |= (uint64_t)(*state->buffer) << 48;
			ADVANCE_STATE(state);
			state->uret |= (uint64_t)(*state->buffer) << 56;
			ADVANCE_STATE(state);

			return 0;
		}

		case STRING_PREFIX: {
			size_t length = 1;
			for (; state->buffer[length - 1] != 0; length++);

			char *str = (char *)alloc(length);
			memcpy(str, state->buffer, length);
			state->pret = str;

			ADVANCE_STATE_BY(state, length + 2); // + 1 for the NULL char, + 1 to get to the next byte

			return 0;
		}

		default: {
			ARC_DEBUG(ERR, "Unhandled lead 0x%x\n", lead);
		}
	}

	REGRESS_STATE(state);

	return 1;
}

int caml_parse_def_pkg(struct caml_state *state) {
	(void)state;
	ARC_DEBUG(WARN, "Unimplemented function\n");
	return 0;
}

int caml_parse_def_var_pkg(struct caml_state *state) {
	(void)state;
	ARC_DEBUG(WARN, "Unimplemented function\n");
	return 0;
}

int caml_parse_data_obj(struct caml_state *state) {
	CALL_CHECK(caml_parse_computational_data(state));
	CALL_CHECK(caml_parse_def_pkg(state));
	CALL_CHECK(caml_parse_def_var_pkg(state));

	return 1;
}

int caml_parse_obj_ref(struct caml_state *state) {
	(void)state;
	ARC_DEBUG(WARN, "Unimplemented function\n");

	return 0;
}

int caml_parse_data_ref_obj(struct caml_state *state) {
	CALL_CHECK(caml_parse_data_obj(state));
	CALL_CHECK(caml_parse_obj_ref(state));

	// Didn't parse anything
	return 1;
}

int caml_parse_term_list(struct caml_state *state, size_t size, struct ARC_VFSNode *root) {
	// In the grammer, a TermList is just comprised
	// of a linear set of bytes which is basically
	// equivalent to how parsing is started. This means
	// that a new state can just be allocated to
	// do all of the parsing for within this scope

	// Save state
	struct ARC_VFSNode *_current = state->current;
	uint64_t _uret = state->uret;
	void *_pret = state->pret;
	size_t _max = state->max - size;

	// Set state
	state->max = size;
	state->current = root;

	while (caml_parse_pkg(state) > 0) {
		// Basically the same thing
		CHECK_STATE(state, break);
	}

	// Restore state
	state->current = _current;
	state->uret = _uret;
	state->pret = _pret;
	state->max = _max;

	return 0;
}

char *caml_parse_name_str(struct caml_state *state) {
	struct ARC_VFSNode *parent = state->current;
	char *name = NULL;

	if (*state->buffer == ROOT_CHR) {
		parent = state->root;
		ADVANCE_STATE(state);
	}

	while (*state->buffer == PARENT_PREFIX_CHR) {
		parent = parent->parent;
		ADVANCE_STATE(state);
	}

	state->current = parent;

	switch (*state->buffer) {
		case DUAL_NAME_PREFIX: {
			ADVANCE_STATE(state);
			name = alloc(8);
			memcpy(name, state->buffer, 8);
			ADVANCE_STATE_BY(state, 8);

			break;
		}

		case MULTI_NAME_PREFIX: {
			ADVANCE_STATE(state);
			uint8_t length = *(state->buffer);
			name = alloc(length * 4);
			memcpy(name, state->buffer, length * 4);
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
			name = alloc(4);
			memcpy(name, state->buffer, 4);
			ADVANCE_STATE_BY(state, 4);

			break;
		}
	}

	return name;
}

int caml_parse_simple_name(struct caml_state *state) {
	(void)state;

	// NameString | ArgObj | LocalObj
	ARC_DEBUG(WARN, "Definitely implemented\n");
	return 0;
}

int caml_parse_super_name(struct caml_state *state) {
	(void)state;

	// SimpleName | DebugObj | ReferenceTypeOpcode
	ARC_DEBUG(WARN, "Definitely implemented\n");
	return 0;
}

int caml_parse_target(struct caml_state *state) {
	(void)state;

	// SuperName | NullName
	ARC_DEBUG(WARN, "Definitely implemented\n");
	return 0;
}

int caml_extops(struct caml_state *state) {
	uint8_t lead = *state->buffer;

	ADVANCE_STATE(state);

	switch (lead) {
		case EXTOP_REGION_OP: {
			ARC_DEBUG(INFO, "Region\n");

			char *name = caml_parse_name_str(state);

			uint64_t space = -1;
			caml_parse_computational_data(state);
			space = state->uret;

			uint64_t offset = -1;
			caml_parse_computational_data(state);
			offset = state->uret;

			uint64_t length = -1;
			caml_parse_computational_data(state);
			length = state->uret;

			ARC_DEBUG(INFO, "\tName: %s\n", name);
			ARC_DEBUG(INFO, "\tSpace: 0x%"PRIx64"\n", space);
			ARC_DEBUG(INFO, "\tOffset: 0x%"PRIx64"\n", offset);
			ARC_DEBUG(INFO, "\tLength: 0x%"PRIx64"\n", length);

			free(name);

			break;
		}

		case EXTOP_DEVICE_OP: {
			ARC_DEBUG(INFO, "Device\n");

			size_t package_length = caml_parse_pkg_len(state);
			size_t delta = state->max;

			char *name = caml_parse_name_str(state);

			delta -= state->max;
			package_length -= delta;

			ARC_DEBUG(INFO, "\tName: %s\n", name);

			free(name);

			ADVANCE_STATE_BY(state, package_length);

			break;
		}

		case EXTOP_FIELD_OP: {
			ARC_DEBUG(INFO, "Field\n");

			size_t package_length = caml_parse_pkg_len(state);
			size_t delta = state->max;

			char *name = caml_parse_name_str(state);
			uint8_t flags = *(uint8_t *)state->buffer;
			ADVANCE_STATE(state);

			// TODO: Parse FieldList

			delta -= state->max;
			package_length -= delta;

			ARC_DEBUG(INFO, "\tName: %s\n", name);
			ARC_DEBUG(INFO, "\tFlags: 0x%x\n", flags);

			free(name);

			ADVANCE_STATE_BY(state, package_length);

			break;
		}

		default: {
			ARC_DEBUG(ERR, "Unhandled EXTOP: 0x%x\n", *state->buffer);
			ADVANCE_STATE(state);

			break;
		}
	}

	return 0;
}

int caml_parse_pkg(struct caml_state *state) {
	uint8_t lead = *state->buffer;
	size_t org_max = state->max;

	ADVANCE_STATE(state);

	switch (lead) {
	case SCOPE_OP: {
		ARC_DEBUG(INFO, "Reading scope %p:\n", state->buffer);

		size_t package_size = caml_parse_pkg_len(state);

		size_t delta = state->max;
		char *name = caml_parse_name_str(state);

		delta -= state->max;
		package_size -= delta;

		ARC_DEBUG(INFO, "\tPkgLength: %lu\n", package_size);
		ARC_DEBUG(INFO, "\tName: %s\n", name);

		struct ARC_VFSNode *node = strlen(name) > 0 ? vfs_create_rel(name, state->current, 0, ARC_VFS_N_DIR, NULL) : state->current;

		if (node == NULL) {
			ARC_DEBUG(ERR, "Failed to create new node\n");
		}

		caml_parse_term_list(state, package_size, node);

//		ADVANCE_STATE_BY(state, package_size);

		free(name);

		break;
	}

	case NAME_OP: {
		ARC_DEBUG(INFO, "Reading name %p:\n", state->buffer);

		char *name = caml_parse_name_str(state);
		caml_parse_data_ref_obj(state);

		ARC_DEBUG(INFO, "\tName: %s\n", name);
		ARC_DEBUG(INFO, "\tUret: %lu\n", state->uret);
		ARC_DEBUG(INFO, "\tPret: %p\n", state->pret);

		struct ARC_VFSNode *node = strlen(name) > 0 ? vfs_create_rel(name, state->current, 0, ARC_VFS_N_DIR, NULL) : state->current;

		if (node == NULL) {
			ARC_DEBUG(ERR, "Failed to create new node\n");
		}

		free(name);

		break;
	}

	case METHOD_OP: {
		ARC_DEBUG(INFO, "Reading method %p:\n", state->buffer);

		size_t package_size = caml_parse_pkg_len(state);
		size_t delta = state->max;

		char *name = caml_parse_name_str(state);
		uint8_t flags = *state->buffer;
		ADVANCE_STATE(state);

		delta -= state->max;
		package_size -= delta;

		ARC_DEBUG(INFO, "\tPkgLength: %lu\n", package_size);
		ARC_DEBUG(INFO, "\tName: %s\n", name);
		ARC_DEBUG(INFO, "\tFlags: 0x%x\n", flags);

		ADVANCE_STATE_BY(state, package_size);

		if (strlen(name) > 0) {
			vfs_create_rel(name, state->current, 0, ARC_VFS_N_BUFF, &package_size);

			// TODO: Work out the absolute path to the node that has just
			//       been created and write data to it

			ARC_DEBUG(WARN, "Definitely wrote data to buffer\n");
		}

		free(name);

		break;
	}

	case ALIAS_OP: {
		char *a = caml_parse_name_str(state);
		char *b = caml_parse_name_str(state);

		ARC_DEBUG(INFO, "Found alias \"%s\" -> \"%s\"", a, b);

		free(a);
		free(b);

		break;
	}

	case EXTOP_PREFIX: {
		caml_extops(state);

		break;
	}

	// Parse out NamedObjects too

	default: {
		ARC_DEBUG(ERR, "Unhandled lead 0x%x\n", lead);

		return 0;
	}
	}

	return org_max - state->max;
}

int caml_parse_def_block(uint8_t *buffer, size_t size) {
	struct caml_state *state = (struct caml_state *)alloc(sizeof(struct caml_state));
	memset(state, 0, sizeof(struct caml_state));

	struct ARC_File *file = NULL;
	if (vfs_open("/dev/acpi/", 0, 0, 0, (void *)&file) != 0) {
		return -1;
	}

	state->root = file->node;
	state->current = state->root;
	state->buffer = buffer;
	state->max = size;

	CHECK_STATE(state, return -1);

	ARC_DEBUG(INFO, "Parsing definition block %p (%lu B)\n", buffer, size);

	while (caml_parse_pkg(state) > 0) {
		CHECK_STATE(state, return -1);
	}

	vfs_close(file);

	free(state);

	return 0;
}
