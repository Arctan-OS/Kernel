/**
 * @file arctan.h
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
#ifndef ARC_ARCTAN_H
#define ARC_ARCTAN_H

#define ARC_HHDM_VADDR   0xFFFFC00000000000 // 192 TiB
#define ARC_PHYS_TO_HHDM(physical) ((uintptr_t)(physical) + (uintptr_t)ARC_HHDM_VADDR)
#define ARC_HHDM_TO_PHYS(hhdm) ((uintptr_t)(hhdm) - (uintptr_t)ARC_HHDM_VADDR)

#include <stdint.h>
#include <stddef.h>

struct ARC_KernMeta {

}__attribute__((packed));

struct ARC_BootMeta {
	uint32_t mb2i; // Physical address of MBI2 structure
	uint32_t pmm_state; // Physical pointer to the state of the bootstrapper's PMM (of type struct ARC_FreelsitMeta)
	struct Arc_KernMeta *state; // State of the last kernel
}__attribute__((packed));

#endif
