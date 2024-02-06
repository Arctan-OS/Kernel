/**
 * @file elf.h
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
#ifndef ARC_ELF_ELF_H
#define ARC_ELF_ELF_H

#include <global.h>

/**
 * Simple ELF64 loader.
 *
 * Very simple ELF loader function for loading a
 * higher-half kernel.
 *
 * @param uint64_t *pml4 - Current PML4 page map to map the file into.
 * @param void *file - 32-bit physical pointer to the start of the ELF file.
 * @return Address at which the file was loaded. Files cannot be loaded at
 * 0x0 or 0x1, as those are used for error codes.
 * */
uint64_t load_elf(uint64_t *pml4, void *elf);

#endif
