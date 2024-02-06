/**
 * @file ctrl_regs.h
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
#ifndef ARC_X86_CTRL_REGS_H
#define ARC_X86_CTRL_REGS_H

#include <stdint.h>

/// Last read value of CR0.
extern uint64_t _x86_CR0;
/// Last read value of CR1.
extern uint64_t _x86_CR1;
/// Last read value of CR2.
extern uint64_t _x86_CR2;
/// Last read value of CR3.
extern uint64_t _x86_CR3;
/// Last read value of CR4.
extern uint64_t _x86_CR4;
/// Reads CR0 into _x86_CR0.
extern void _x86_getCR0();
/// Writes _x86_CR0 to CR0.
extern void _x86_setCR0();
/// Reads CR1 into _x86_CR1.
extern void _x86_getCR1();
/// Writes _x86_CR1 to CR1.
extern void _x86_setCR1();
/// Reads CR2 into _x86_CR2.
extern void _x86_getCR2();
/// Writes _x86_CR2 to CR2.
extern void _x86_setCR2();
/// Reads CR3 into _x86_CR3.
extern void _x86_getCR3();
/// Writes _x86_CR3 to CR3.
extern void _x86_setCR3();
/// Reads CR4 into _x86_CR4.
extern void _x86_getCR4();
/// Writes _x86_CR4 to CR4.
extern void _x86_setCR4();

#endif
