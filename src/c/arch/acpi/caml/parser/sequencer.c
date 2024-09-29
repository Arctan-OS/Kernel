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
#include <arch/acpi/caml/parser/scope.h>
#include <arch/acpi/caml/parser/method.h>
#include <mm/allocator.h>
#include <global.h>

#ifdef ARC_TARGET_ARCH_X86_64
#include <arch/x86-64/acpi/compiler.h>
#endif

static int (*sequencer_table[256])(struct ARC_cAMLState *state) = {
        [0x00] = NULL,
        [0x01] = NULL,
        [0x02] = NULL,
        [0x03] = NULL,
        [0x04] = NULL,
        [0x05] = NULL,
        [0x06] = NULL,
        [0x07] = NULL,
        [0x08] = NULL,
        [0x09] = NULL,
        [0x0A] = NULL,
        [0x0B] = NULL,
        [0x0C] = NULL,
        [0x0D] = NULL,
        [0x0E] = NULL,
        [0x0F] = NULL,

        [0x10] = parse_scope,
        [0x11] = NULL,
        [0x12] = NULL,
        [0x13] = NULL,
        [0x14] = parse_method,
        [0x15] = NULL,
        [0x16] = NULL,
        [0x17] = NULL,
        [0x18] = NULL,
        [0x19] = NULL,
        [0x1A] = NULL,
        [0x1B] = NULL,
        [0x1C] = NULL,
        [0x1D] = NULL,
        [0x1E] = NULL,
        [0x1F] = NULL,

        [0x20] = NULL,
        [0x21] = NULL,
        [0x22] = NULL,
        [0x23] = NULL,
        [0x24] = NULL,
        [0x25] = NULL,
        [0x26] = NULL,
        [0x27] = NULL,
        [0x28] = NULL,
        [0x29] = NULL,
        [0x2A] = NULL,
        [0x2B] = NULL,
        [0x2C] = NULL,
        [0x2D] = NULL,
        [0x2E] = NULL, // Dual Name Prefix
        [0x2F] = NULL, // Multi Name Prefix

        [0x30] = NULL, // Digit Characters Begin
        [0x31] = NULL,
        [0x32] = NULL,
        [0x33] = NULL,
        [0x34] = NULL,
        [0x35] = NULL,
        [0x36] = NULL,
        [0x37] = NULL,
        [0x38] = NULL,
        [0x39] = NULL, // Digit Characters End
        [0x3A] = NULL,
        [0x3B] = NULL,
        [0x3C] = NULL,
        [0x3D] = NULL,
        [0x3E] = NULL,
        [0x3F] = NULL,

        [0x40] = NULL,
        [0x41] = NULL, // ASCII Characters Begin
        [0x42] = NULL,
        [0x43] = NULL,
        [0x44] = NULL,
        [0x45] = NULL,
        [0x46] = NULL,
        [0x47] = NULL,
        [0x48] = NULL,
        [0x49] = NULL,
        [0x4A] = NULL,
        [0x4B] = NULL,
        [0x4C] = NULL,
        [0x4D] = NULL,
        [0x4E] = NULL,
        [0x4F] = NULL,

        [0x50] = NULL,
        [0x51] = NULL,
        [0x52] = NULL,
        [0x53] = NULL,
        [0x54] = NULL,
        [0x55] = NULL,
        [0x56] = NULL,
        [0x57] = NULL,
        [0x58] = NULL,
        [0x59] = NULL,
        [0x5A] = NULL, // ASCII Characters End
        [0x5B] = extop_parse, // Ext Op Prefix
        [0x5C] = NULL, // Root Character
        [0x5D] = NULL,
        [0x5E] = NULL, // Parent Prefix Character
        [0x5F] = NULL, // ASCII Character

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
        [0x6F] = NULL,

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
        [0x98] = compile_to_hex_string,
        [0x99] = NULL,
        [0x9A] = NULL,
        [0x9B] = NULL,
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
        [0xA6] = NULL,
        [0xA7] = NULL,
        [0xA8] = NULL,
        [0xA9] = NULL,
        [0xAA] = NULL,
        [0xAB] = NULL,
        [0xAC] = NULL,
        [0xAD] = NULL,
        [0xAE] = NULL,
        [0xAF] = NULL,

        [0xB0] = NULL,
        [0xB1] = NULL,
        [0xB2] = NULL,
        [0xB3] = NULL,
        [0xB4] = NULL,
        [0xB5] = NULL,
        [0xB6] = NULL,
        [0xB7] = NULL,
        [0xB8] = NULL,
        [0xB9] = NULL,
        [0xBA] = NULL,
        [0xBB] = NULL,
        [0xBC] = NULL,
        [0xBD] = NULL,
        [0xBE] = NULL,
        [0xBF] = NULL,

        [0xC0] = NULL,
        [0xC1] = NULL,
        [0xC2] = NULL,
        [0xC3] = NULL,
        [0xC4] = NULL,
        [0xC5] = NULL,
        [0xC6] = NULL,
        [0xC7] = NULL,
        [0xC8] = NULL,
        [0xC9] = NULL,
        [0xCA] = NULL,
        [0xCB] = NULL,
        [0xCC] = NULL,
        [0xCD] = NULL,
        [0xCE] = NULL,
        [0xCF] = NULL,

        [0xD0] = NULL,
        [0xD1] = NULL,
        [0xD2] = NULL,
        [0xD3] = NULL,
        [0xD4] = NULL,
        [0xD5] = NULL,
        [0xD6] = NULL,
        [0xD7] = NULL,
        [0xD8] = NULL,
        [0xD9] = NULL,
        [0xDA] = NULL,
        [0xDB] = NULL,
        [0xDC] = NULL,
        [0xDD] = NULL,
        [0xDE] = NULL,
        [0xDF] = NULL,

        [0xE0] = NULL,
        [0xE1] = NULL,
        [0xE2] = NULL,
        [0xE3] = NULL,
        [0xE4] = NULL,
        [0xE5] = NULL,
        [0xE6] = NULL,
        [0xE7] = NULL,
        [0xE8] = NULL,
        [0xE9] = NULL,
        [0xEA] = NULL,
        [0xEB] = NULL,
        [0xEC] = NULL,
        [0xED] = NULL,
        [0xEE] = NULL,
        [0xEF] = NULL,

        [0xF0] = NULL,
        [0xF1] = NULL,
        [0xF2] = NULL,
        [0xF3] = NULL,
        [0xF4] = NULL,
        [0xF5] = NULL,
        [0xF6] = NULL,
        [0xF7] = NULL,
        [0xF8] = NULL,
        [0xF9] = NULL,
        [0xFA] = NULL,
        [0xFB] = NULL,
        [0xFC] = NULL,
        [0xFD] = NULL,
        [0xFE] = NULL,
        [0xFF] = NULL,
};

int sequencer_begin(struct ARC_cAMLState *state) {
	if (state == NULL || state->buffer == NULL) {
		ARC_DEBUG(ERR, "State or buffer (%p, %p) is NULL\n", state, state->buffer);
		return -2;
	}

	size_t max = state->max;

	while (state->max > 0 && state->max <= max) {
		uint8_t i = *state->buffer;
		ADVANCE_STATE(state);

		if (sequencer_table[i] == NULL) {
			ARC_DEBUG(ERR, "Unrecognized bytecode 0x%02x\n", i);
			for (int i = -17; i < 16; i++) {
				printf("%c%02X%c ", (i == -1 ? '[' : '\0'), state->buffer[i], (i == -1 ? ']' : '\0'));
			}
			printf("\n");

			if (state->jit_buffer != NULL) {
				free(state->jit_buffer);
				state->jit_buffer = NULL;
			}

			break;
		}

		sequencer_table[i](state);
	}

	return 0;
}
