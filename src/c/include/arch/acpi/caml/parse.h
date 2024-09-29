/**
 * @file parse.h
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
#ifndef ARC_ARCH_ACPI_CAML_PARSE_H
#define ARC_ARCH_ACPI_CAML_PARSE_H

#include <stddef.h>
#include <stdint.h>

struct ARC_cAMLState {
	// Buffer containing AML
	uint8_t *buffer;
	// Length of buffer containing AML
	size_t max;
	// The root directory of the ACPI tree
	struct ARC_VFSNode *root;
	// The current scope's working directory
	struct ARC_VFSNode *parent;
	// JIT buffer to write JITed code to (THIS IS A FAKE FILE)
	struct ARC_File *jit_buffer;
	// End first AML byte that should no longer be JIT compiled
	// into the JIT buffer
	size_t jit_limit;
};

int caml_parse(uint8_t *buffer, size_t size);

#endif
