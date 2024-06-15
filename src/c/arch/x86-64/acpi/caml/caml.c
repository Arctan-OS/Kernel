/**
 * @file caml.c
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
#include <arch/x86-64/acpi/caml/caml.h>
#include <arch/x86-64/acpi/caml/ops.h>
#include <fs/vfs.h>
#include <global.h>

char *cAML_ParseNamestring(uint8_t *buffer, size_t max) {
	return 0;
}

int cAML_ParsePackage(uint8_t *buffer, size_t max) {
	size_t i = 0;
	uint8_t lead = buffer[i++];
	uint8_t pkglead = buffer[i];
	uint8_t count = (pkglead >> 6) & 0b11;
	size_t package_size = pkglead & 0x3F;

	if (count > 0) {
		package_size = pkglead & 0xF;
		for (int j = 0; j < count; j++) {
			package_size |= buffer[i + j + 1] << (j * 8 + 4);
		}
	}

	package_size++;
	i += count + 1;

	switch (lead) {
	case SCOPE_OP: {
		ARC_DEBUG(INFO, "Reading scope %p: Lead(0x%X) PkgLength(0x%X)\n", buffer, lead, package_size);

		break;
	}
	case NAME_OP: {
		ARC_DEBUG(INFO, "Reading name %p: Lead(0x%X) PkgLength(0x%X)\n", buffer, lead, package_size);

		cAML_ParseNamestring(&buffer[i], max - i);

		break;
	}
	}

	return package_size;
}

int cAML_ParseDefinitionBlock(uint8_t *buffer, size_t size) {
	ARC_DEBUG(INFO, "Parsing defintion block %p (%d B)\n", buffer, size);

	for (size_t i = 0; i < size;) {
		size_t ret = cAML_ParsePackage(&buffer[i], size - i);

		if (ret == 0) {
			ARC_DEBUG(INFO, "Infinite loop detected, breaking\n");
			break;
		}

		i += ret;
	}

	return 0;
}
