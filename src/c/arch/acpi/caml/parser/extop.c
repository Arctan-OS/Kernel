/**
 * @file extop.c
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
#include <arch/acpi/caml/parser/name.h>
#include <arch/acpi/caml/parser/package.h>
#include <arch/acpi/caml/parser/data.h>
#include <mm/allocator.h>
#include <global.h>

int extop_parse(struct ARC_cAMLState *state) {
	uint8_t lead = *state->buffer;

	ADVANCE_STATE(state);

	switch (lead) {
		case EXTOP_REGION_OP: {
			ARC_DEBUG(INFO, "Region\n");

			char *name = parse_name_string(state);

			void *pret = NULL;
			uint64_t space = parse_computational_data(state, &pret);
			uint64_t offset = parse_computational_data(state, &pret);
			uint64_t length = parse_computational_data(state, &pret);

			ARC_DEBUG(INFO, "\tName: %s\n", name);
			ARC_DEBUG(INFO, "\tSpace: 0x%"PRIx64"\n", space);
			ARC_DEBUG(INFO, "\tOffset: 0x%"PRIx64"\n", offset);
			ARC_DEBUG(INFO, "\tLength: 0x%"PRIx64"\n", length);

			free(name);

			break;
		}

		case EXTOP_DEVICE_OP: {
			ARC_DEBUG(INFO, "Device\n");

			size_t package_length = parse_package_length(state);
			size_t delta = state->max;

			char *name = parse_name_string(state);

			delta -= state->max;
			package_length -= delta;

			ARC_DEBUG(INFO, "\tName: %s\n", name);

			free(name);

			ADVANCE_STATE_BY(state, package_length);

			break;
		}

		case EXTOP_FIELD_OP: {
			ARC_DEBUG(INFO, "Field\n");

			size_t package_length = parse_package_length(state);
			size_t delta = state->max;

			char *name = parse_name_string(state);
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
