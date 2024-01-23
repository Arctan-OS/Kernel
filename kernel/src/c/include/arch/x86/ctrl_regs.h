/**
 * @file ctrl_regs.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
 *
 * This file is apart of Arctan.
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
#ifndef ARC_X86_CTRL_REGS_H
#define ARC_X86_CTRL_REGS_H

#include <stdint.h>

extern uint64_t _x86_CR0;
extern uint64_t _x86_CR1;
extern uint64_t _x86_CR2;
extern uint64_t _x86_CR3;
extern uint64_t _x86_CR4;

extern void _x86_getCR0();
extern void _x86_setCR0();

extern void _x86_getCR1();
extern void _x86_setCR1();

extern void _x86_getCR2();
extern void _x86_setCR2();

extern void _x86_getCR3();
extern void _x86_setCR3();

extern void _x86_getCR4();
extern void _x86_setCR4();

#endif
