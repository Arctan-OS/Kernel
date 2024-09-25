/**
 * @file sequencer.c
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
#include <arch/acpi/caml/parser/sequencer.h>
#include <arch/acpi/caml/parser/extop.h>
#include <mm/allocator.h>
#include <global.h>

static int cannot_parse_error_handle(struct ARC_cAMLState *state) {
	if (state == NULL || state->buffer == NULL) {
		ARC_DEBUG(ERR, "State or buffer (%p, %p) is NULL\n", state, state->buffer);
		return -2;
	}

	ARC_DEBUG(ERR, "Cannot parse byte 0x%02x\n", *state->buffer);

	return -1;
}

static int (*sequencer_table[256])(struct ARC_cAMLState *state) = {
        [0x00] = NULL,
        [0x01] = NULL,
        [0x02] = cannot_parse_error_handle,
        [0x03] = cannot_parse_error_handle,
        [0x04] = cannot_parse_error_handle,
        [0x05] = cannot_parse_error_handle,
        [0x06] = NULL,
        [0x07] = cannot_parse_error_handle,
        [0x08] = NULL,
        [0x09] = cannot_parse_error_handle,
        [0x0A] = NULL,
        [0x0B] = NULL,
        [0x0C] = NULL,
        [0x0D] = NULL,
        [0x0E] = NULL,
        [0x0F] = cannot_parse_error_handle,

        [0x10] = NULL,
        [0x11] = NULL,
        [0x12] = NULL,
        [0x13] = NULL,
        [0x14] = NULL,
        [0x15] = NULL,
        [0x16] = cannot_parse_error_handle,
        [0x17] = cannot_parse_error_handle,
        [0x18] = cannot_parse_error_handle,
        [0x19] = cannot_parse_error_handle,
        [0x1A] = cannot_parse_error_handle,
        [0x1B] = cannot_parse_error_handle,
        [0x1C] = cannot_parse_error_handle,
        [0x1D] = cannot_parse_error_handle,
        [0x1E] = cannot_parse_error_handle,
        [0x1F] = cannot_parse_error_handle,

        [0x20] = cannot_parse_error_handle,
        [0x21] = cannot_parse_error_handle,
        [0x22] = cannot_parse_error_handle,
        [0x23] = cannot_parse_error_handle,
        [0x24] = cannot_parse_error_handle,
        [0x25] = cannot_parse_error_handle,
        [0x26] = cannot_parse_error_handle,
        [0x27] = cannot_parse_error_handle,
        [0x28] = cannot_parse_error_handle,
        [0x29] = cannot_parse_error_handle,
        [0x2A] = cannot_parse_error_handle,
        [0x2B] = cannot_parse_error_handle,
        [0x2C] = cannot_parse_error_handle,
        [0x2D] = cannot_parse_error_handle,
        [0x2E] = cannot_parse_error_handle, // Dual Name Prefix
        [0x2F] = cannot_parse_error_handle, // Multi Name Prefix

        [0x30] = cannot_parse_error_handle, // Digit Characters Begin
        [0x31] = cannot_parse_error_handle,
        [0x32] = cannot_parse_error_handle,
        [0x33] = cannot_parse_error_handle,
        [0x34] = cannot_parse_error_handle,
        [0x35] = cannot_parse_error_handle,
        [0x36] = cannot_parse_error_handle,
        [0x37] = cannot_parse_error_handle,
        [0x38] = cannot_parse_error_handle,
        [0x39] = cannot_parse_error_handle, // Digit Characters End
        [0x3A] = cannot_parse_error_handle,
        [0x3B] = cannot_parse_error_handle,
        [0x3C] = cannot_parse_error_handle,
        [0x3D] = cannot_parse_error_handle,
        [0x3E] = cannot_parse_error_handle,
        [0x3F] = cannot_parse_error_handle,

        [0x40] = cannot_parse_error_handle,
        [0x41] = cannot_parse_error_handle, // ASCII Characters Begin
        [0x42] = cannot_parse_error_handle,
        [0x43] = cannot_parse_error_handle,
        [0x44] = cannot_parse_error_handle,
        [0x45] = cannot_parse_error_handle,
        [0x46] = cannot_parse_error_handle,
        [0x47] = cannot_parse_error_handle,
        [0x48] = cannot_parse_error_handle,
        [0x49] = cannot_parse_error_handle,
        [0x4A] = cannot_parse_error_handle,
        [0x4B] = cannot_parse_error_handle,
        [0x4C] = cannot_parse_error_handle,
        [0x4D] = cannot_parse_error_handle,
        [0x4E] = cannot_parse_error_handle,
        [0x4F] = cannot_parse_error_handle,

        [0x50] = cannot_parse_error_handle,
        [0x51] = cannot_parse_error_handle,
        [0x52] = cannot_parse_error_handle,
        [0x53] = cannot_parse_error_handle,
        [0x54] = cannot_parse_error_handle,
        [0x55] = cannot_parse_error_handle,
        [0x56] = cannot_parse_error_handle,
        [0x57] = cannot_parse_error_handle,
        [0x58] = cannot_parse_error_handle,
        [0x59] = cannot_parse_error_handle,
        [0x5A] = cannot_parse_error_handle, // ASCII Characters End
        [0x5B] = extop_parse, // Ext Op Prefix
        [0x5C] = cannot_parse_error_handle, // Root Character
        [0x5D] = cannot_parse_error_handle,
        [0x5E] = cannot_parse_error_handle, // Parent Prefix Character
        [0x5F] = cannot_parse_error_handle, // ASCII Character

        [0x60] = NULL,
        [0x61] = NULL,
        [0x62] = NULL,
        [0x63] = NULL,
        [0x64] = NULL,
        [0x65] = NULL,
        [0x66] = NULL,
        [0x67] = NULL,
        [0x68] = NULL,
        [0x69] = NULL,
        [0x6A] = NULL,
        [0x6B] = NULL,
        [0x6C] = NULL,
        [0x6D] = NULL,
        [0x6E] = NULL,
        [0x6F] = cannot_parse_error_handle,

        [0x70] = NULL,
        [0x71] = NULL,
        [0x72] = NULL,
        [0x73] = NULL,
        [0x74] = NULL,
        [0x75] = NULL,
        [0x76] = NULL,
        [0x77] = NULL,
        [0x78] = NULL,
        [0x79] = NULL,
        [0x7A] = NULL,
        [0x7B] = NULL,
        [0x7C] = NULL,
        [0x7D] = NULL,
        [0x7E] = NULL,
        [0x7F] = NULL,

        [0x80] = NULL,
        [0x81] = NULL,
        [0x82] = NULL,
        [0x83] = NULL,
        [0x84] = NULL,
        [0x85] = NULL,
        [0x86] = NULL,
        [0x87] = NULL,
        [0x88] = NULL,
        [0x89] = NULL,
        [0x8A] = NULL,
        [0x8B] = NULL,
        [0x8C] = NULL,
        [0x8D] = NULL,
        [0x8E] = NULL,
        [0x8F] = NULL,

        [0x90] = NULL,
        [0x91] = NULL,
        [0x92] = NULL,
        [0x93] = NULL,
        [0x94] = NULL,
        [0x95] = NULL,
        [0x96] = NULL,
        [0x97] = NULL,
        [0x98] = NULL,
        [0x99] = NULL,
        [0x9A] = cannot_parse_error_handle,
        [0x9B] = cannot_parse_error_handle,
        [0x9C] = NULL,
        [0x9D] = NULL,
        [0x9E] = NULL,
        [0x9F] = NULL,

        [0xA0] = NULL,
        [0xA1] = NULL,
        [0xA2] = NULL,
        [0xA3] = NULL,
        [0xA4] = NULL,
        [0xA5] = NULL,
        [0xA6] = cannot_parse_error_handle,
        [0xA7] = cannot_parse_error_handle,
        [0xA8] = cannot_parse_error_handle,
        [0xA9] = cannot_parse_error_handle,
        [0xAA] = cannot_parse_error_handle,
        [0xAB] = cannot_parse_error_handle,
        [0xAC] = cannot_parse_error_handle,
        [0xAD] = cannot_parse_error_handle,
        [0xAE] = cannot_parse_error_handle,
        [0xAF] = cannot_parse_error_handle,

        [0xB0] = cannot_parse_error_handle,
        [0xB1] = cannot_parse_error_handle,
        [0xB2] = cannot_parse_error_handle,
        [0xB3] = cannot_parse_error_handle,
        [0xB4] = cannot_parse_error_handle,
        [0xB5] = cannot_parse_error_handle,
        [0xB6] = cannot_parse_error_handle,
        [0xB7] = cannot_parse_error_handle,
        [0xB8] = cannot_parse_error_handle,
        [0xB9] = cannot_parse_error_handle,
        [0xBA] = cannot_parse_error_handle,
        [0xBB] = cannot_parse_error_handle,
        [0xBC] = cannot_parse_error_handle,
        [0xBD] = cannot_parse_error_handle,
        [0xBE] = cannot_parse_error_handle,
        [0xBF] = cannot_parse_error_handle,

        [0xC0] = cannot_parse_error_handle,
        [0xC1] = cannot_parse_error_handle,
        [0xC2] = cannot_parse_error_handle,
        [0xC3] = cannot_parse_error_handle,
        [0xC4] = cannot_parse_error_handle,
        [0xC5] = cannot_parse_error_handle,
        [0xC6] = cannot_parse_error_handle,
        [0xC7] = cannot_parse_error_handle,
        [0xC8] = cannot_parse_error_handle,
        [0xC9] = cannot_parse_error_handle,
        [0xCA] = cannot_parse_error_handle,
        [0xCB] = cannot_parse_error_handle,
        [0xCC] = NULL,
        [0xCD] = cannot_parse_error_handle,
        [0xCE] = cannot_parse_error_handle,
        [0xCF] = cannot_parse_error_handle,

        [0xD0] = cannot_parse_error_handle,
        [0xD1] = cannot_parse_error_handle,
        [0xD2] = cannot_parse_error_handle,
        [0xD3] = cannot_parse_error_handle,
        [0xD4] = cannot_parse_error_handle,
        [0xD5] = cannot_parse_error_handle,
        [0xD6] = cannot_parse_error_handle,
        [0xD7] = cannot_parse_error_handle,
        [0xD8] = cannot_parse_error_handle,
        [0xD9] = cannot_parse_error_handle,
        [0xDA] = cannot_parse_error_handle,
        [0xDB] = cannot_parse_error_handle,
        [0xDC] = cannot_parse_error_handle,
        [0xDD] = cannot_parse_error_handle,
        [0xDE] = cannot_parse_error_handle,
        [0xDF] = cannot_parse_error_handle,

        [0xE0] = cannot_parse_error_handle,
        [0xE1] = cannot_parse_error_handle,
        [0xE2] = cannot_parse_error_handle,
        [0xE3] = cannot_parse_error_handle,
        [0xE4] = cannot_parse_error_handle,
        [0xE5] = cannot_parse_error_handle,
        [0xE6] = cannot_parse_error_handle,
        [0xE7] = cannot_parse_error_handle,
        [0xE8] = cannot_parse_error_handle,
        [0xE9] = cannot_parse_error_handle,
        [0xEA] = cannot_parse_error_handle,
        [0xEB] = cannot_parse_error_handle,
        [0xEC] = cannot_parse_error_handle,
        [0xED] = cannot_parse_error_handle,
        [0xEE] = cannot_parse_error_handle,
        [0xEF] = cannot_parse_error_handle,

        [0xF0] = cannot_parse_error_handle,
        [0xF1] = cannot_parse_error_handle,
        [0xF2] = cannot_parse_error_handle,
        [0xF3] = cannot_parse_error_handle,
        [0xF4] = cannot_parse_error_handle,
        [0xF5] = cannot_parse_error_handle,
        [0xF6] = cannot_parse_error_handle,
        [0xF7] = cannot_parse_error_handle,
        [0xF8] = cannot_parse_error_handle,
        [0xF9] = cannot_parse_error_handle,
        [0xFA] = cannot_parse_error_handle,
        [0xFB] = cannot_parse_error_handle,
        [0xFC] = cannot_parse_error_handle,
        [0xFD] = cannot_parse_error_handle,
        [0xFE] = cannot_parse_error_handle,
        [0xFF] = NULL,
};


// Scope: A region of code
// Lower 32-bits: Offset from current byte to the first byte outside
//                of the current scope
// Upper 32-bits: Attributes
static uint64_t *scope_stack = NULL;
static int sequencer_ptr = 0;

int sequencer_pop_scope() {

}

int sequencer_push_scope(uint64_t scope) {
	scope_stack[sequencer_ptr++] = scope;

	return 0;
}

int sequencer_begin(struct ARC_cAMLState *start_state) {
	size_t current = 0;

	scope_stack = alloc(PAGE_SIZE);

	while (current < start_state->max) {
		sequencer_table[start_state->buffer[current]](start_state);
	}

	free(scope_stack);
	scope_stack = NULL;

	return 0;
}
