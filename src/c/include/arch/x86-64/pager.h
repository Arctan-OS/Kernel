/**
 * @file pager.h
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
#ifndef ARC_ARCH_X86_64_PAGER_H
#define ARC_ARCH_X86_64_PAGER_H


/// The overwrite flag.
#define ARC_VMM_OVERW_FLAG (1 << 31)
/// The create flag.
#define ARC_VMM_CREAT_FLAG (1 << 30)

// PAT << 7 or 12 (2 MB ad 1 GB)
// PWT << 2
// PCD << 3
#define ARC_PAGER_PAT_WB(npte) ((0 << ((npte * 5) + 7)) | (0 << 3) | (0 << 2))
#define ARC_PAGER_PAT_UC(npte) ((0 << ((npte * 5) + 7)) | (0 << 3) | (1 << 2))
#define ARC_PAGER_PAT_UCD(npte) ((0 << ((npte * 5) + 7)) | (1 << 3) | (1 << 2))
#define ARC_PAGER_PAT_WC(npte) ((0 << ((npte * 5) + 7)) | (1 << 3) | (1 << 2))
#define ARC_PAGER_PAT_WT(npte) ((1 << ((npte * 5) + 7)) | (0 << 3) | (0 << 2))
#define ARC_PAGER_PAT_WP(npte) ((1 << ((npte * 5) + 7)) | (0 << 3) | (1 << 2))

#include <stdint.h>
#include <stddef.h>

int Arc_MapPager(uint64_t virtual, uint64_t physical, size_t size, uint32_t attributes) ;
int Arc_UnmapPager(uint64_t virtual, size_t size);
int Arc_SetAttrsPager(uint64_t virtual, size_t size, uint32_t attributes);

void Arc_InitPager();

#endif
