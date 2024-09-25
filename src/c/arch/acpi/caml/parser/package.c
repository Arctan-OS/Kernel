/**
 * @file package.c
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
#include <arch/acpi/caml/parser/package.h>
#include <arch/acpi/caml/parser/sequencer.h>

size_t parse_package_length(struct ARC_cAMLState *state) {
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
