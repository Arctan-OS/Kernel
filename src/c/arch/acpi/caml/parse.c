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
#include <arch/acpi/caml/parse.h>
#include <arch/acpi/caml/parser/sequencer.h>
#include <lib/perms.h>
#include <global.h>

int caml_parse(uint8_t *buffer, size_t size) {
	struct ARC_File *root = NULL;

	if (vfs_open("/dev/acpi/", 0, ARC_STD_PERM, &root) != 0) {
		ARC_DEBUG(ERR, "Failed to open ACPI root\n");
		return -1;
	}

	struct ARC_cAMLState state = {
  	        .buffer = buffer,
	        .max = size,
		.root = root->node,
		.parent = root->node
        };

	if (sequencer_begin(&state) != 0) {
		ARC_DEBUG(ERR, "Failed to parse AML\n");
	}

	vfs_close(root);

	return 0;
}
