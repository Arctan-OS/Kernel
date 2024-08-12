/**
 * @file abstract.c
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
#include <mp/sched/abstract.h>
#include <global.h>
#include <arch/x86-64/apic/lapic.h>

uint64_t get_current_tid() {

	return ((uint64_t)lapic_get_id() << 56) | 1;
}

int yield_cpu(int64_t tid) {
	if (get_current_tid() == tid) {
		return 0;
	}

	ARC_DEBUG(INFO, "Yielding CPU\n");

	return 0;
}
