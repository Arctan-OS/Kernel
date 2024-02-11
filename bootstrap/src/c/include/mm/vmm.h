/**
 * @file vmm.h
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
 * Simple virtual memory manager.
*/
#ifndef ARC_MM_VMM_H
#define ARC_MM_VMM_H

#include <global.h>

/**
 * Map the given page to the given page frame in the given PML4.
 *
 * New page tables will be created automatically, if overwrite is set to
 * 1 then any existing page is already mapped and present it will be
 * overwritten.
 *
 * @return Returns a pointer to the PML4, upon success it will be the same
 * as uint64_t *pml4.
 * */
uint64_t *map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr, int overwrite);

#endif
