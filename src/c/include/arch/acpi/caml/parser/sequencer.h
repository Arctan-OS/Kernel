/**
 * @file sequencer.h
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
#ifndef ARC_ARCH_ACPI_CAML_PARSER_SEQUENCER_H
#define ARC_ARCH_ACPI_CAML_PARSER_SEQUENCER_H

#define ADVANCE_STATE(state) \
	state->buffer++; \
	state->max--;

#define ADVANCE_STATE_BY(state, by) \
	state->buffer += by; \
	state->max -= by;

#define REGRESS_STATE(state) \
	state->buffer--; \
	state->max++;

#define REGRESS_STATE_BY(state, by) \
	state->buffer -= by; \
	state->max += by;

#include <arch/acpi/caml/parse.h>

uint64_t sequencer_pop_scope();
int sequencer_push_scope(uint64_t scope);

int sequencer_begin(struct ARC_cAMLState *state);

#endif
