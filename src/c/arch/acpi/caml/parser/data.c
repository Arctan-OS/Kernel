/**
 * @file data.c
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
#include <arch/acpi/caml/parser/extop.h>
#include <arch/acpi/caml/parser/sequencer.h>
#include <arch/acpi/caml/ops.h>
#include <lib/util.h>
#include <mm/allocator.h>
#include <global.h>

uint64_t parse_computational_data(struct ARC_cAMLState *state, void **pret) {
	uint8_t lead = *state->buffer;
	uint64_t ret = 0;

	ADVANCE_STATE(state);
	switch (lead) {
		case ZERO_OP:
		case ONE_OP:
		case ONES_OP: {
			ret = lead;
			break;
		}

		case BYTE_PREFIX: {
			ret = *state->buffer;
			ADVANCE_STATE(state);

			break;
		}

		case WORD_PREFIX: {
			ret = *state->buffer;
			ADVANCE_STATE(state);
			ret |= *state->buffer << 8;
			ADVANCE_STATE(state);

			break;
		}

		case DWORD_PREFIX: {
			ret = *state->buffer;
			ADVANCE_STATE(state);
			ret |= *state->buffer << 8;
			ADVANCE_STATE(state);
			ret |= *state->buffer << 16;
			ADVANCE_STATE(state);
			ret |= *state->buffer << 24;
			ADVANCE_STATE(state);

			break;
		}

		case QWORD_PREFIX: {
			ret = *state->buffer;
			ADVANCE_STATE(state);
			ret |= *state->buffer << 8;
			ADVANCE_STATE(state);
			ret |= *state->buffer << 16;
			ADVANCE_STATE(state);
			ret |= *state->buffer << 24;
			ADVANCE_STATE(state);
			ret = (uint64_t)(*state->buffer) << 32;
			ADVANCE_STATE(state);
			ret |= (uint64_t)*state->buffer << 40;
			ADVANCE_STATE(state);
			ret |= (uint64_t)(*state->buffer) << 48;
			ADVANCE_STATE(state);
			ret |= (uint64_t)(*state->buffer) << 56;
			ADVANCE_STATE(state);

			break;
		}

		case STRING_PREFIX: {
			size_t length = 1;
			for (; state->buffer[length - 1] != 0; length++);

			char *str = (char *)alloc(length);
			memcpy(str, state->buffer, length);
			*pret = str;

			ADVANCE_STATE_BY(state, length + 2); // + 1 for the NULL char, + 1 to get to the next byte

			break;
		}

		default: {
			ARC_DEBUG(ERR, "Unhandled lead 0x%x\n", lead);
			return 0;
		}
	}

	return ret;
}
