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
#include <mm/slab.h>
#include <util.h>

struct ARC_VFSNode *root = NULL;
struct ARC_VFSNode *current = NULL;

// TODO: Create a structure which will hold the
//       current state of the parser, with this
//       state the buffer will be advanced across
//       as each cAML_Parse... function is called.
//       This state should also hold the above
//       variables (root and current)

int cAML_ParseDataObject(uint8_t *buffer, size_t max) {
	return 0;
}

int cAML_ParseDataRefObject(uint8_t *buffer, size_t max) {
	return 0;
}

char *cAML_ParseNamestring(uint8_t *buffer, size_t max) {
	struct ARC_VFSNode *parent = current;
	char *name = NULL;

	if (*buffer == ROOT_CHR) {
		printf("Root character\n");
		parent = root;
		buffer++;
	}

	while (*buffer == PARENT_PREFIX_CHR && *(buffer++) == PARENT_PREFIX_CHR) {
		parent = parent->parent;
	}

	switch (*buffer) {
	case DUAL_NAME_PREFIX: {
		buffer++;
		name = Arc_SlabAlloc(8);
		memcpy(name, buffer, 8);

		break;
	}

	case MULTI_NAME_PREFIX: {
		uint8_t length = *(++buffer);
		buffer++;
		name = Arc_SlabAlloc(length * 4);
		memcpy(name, buffer, length * 4);

		break;
	}

	default: {
		name = Arc_SlabAlloc(4);
		memcpy(name, buffer, 4);

		break;
	}
	}

	return name;
}

int cAML_ParsePackage(uint8_t *buffer, size_t max) {
	size_t i = 0;
	uint8_t lead = buffer[i++];
	size_t package_size = 0;

	switch (lead) {
	case SCOPE_OP: {
		uint8_t pkglead = buffer[i];
		uint8_t count = (pkglead >> 6) & 0b11;
		package_size = pkglead & 0x3F;

		if (count > 0) {
			package_size = pkglead & 0xF;
			for (int j = 0; j < count; j++) {
				package_size |= buffer[i + j + 1] << (j * 8 + 4);
			}
		}

		ARC_DEBUG(INFO, "Reading scope %p: Lead(0x%X) PkgLength(0x%X)\n", buffer, lead, package_size);

		package_size++;
		i += count + 1;

		break;
	}
	case NAME_OP: {
		ARC_DEBUG(INFO, "Reading name %p: Lead(0x%X) PkgLength(0x%X)\n", buffer, lead, package_size);

		char *name = cAML_ParseNamestring(&buffer[i], max - i);
		ARC_DEBUG(INFO, "\tName: %s\n", name);

		break;
	}
	}

	return package_size;
}

int cAML_ParseDefinitionBlock(uint8_t *buffer, size_t size) {
	ARC_DEBUG(INFO, "Parsing defintion block %p (%d B)\n", buffer, size);

	root = Arc_GetNodeVFS("/dev/acpi/rsdt/fadt/dsdt/", 0);
	current = root;

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
