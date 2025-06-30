/**
 * @file global.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Kernel - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan-OS/Kernel.
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
#ifndef ARC_GLOBAL_H
#define ARC_GLOBAL_H

#include <arctan.h>
#include <util.h>
#include <config.h>

extern struct ARC_KernelMeta *Arc_KernelMeta;
extern struct ARC_BootMeta *Arc_BootMeta;

extern uint8_t __KERNEL_START__;
extern uint8_t __KERNEL_END__;
extern uint8_t __KERNEL_STACK__;

#endif
