/**
 * @file dsdt.c
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
#include <lib/resource.h>
#include <arch/x86-64/acpi.h>
#include <global.h>

int init_dsdt(struct ARC_Resource *res, void *arg) {
	ARC_DEBUG(INFO, "Initializing DSDT: %p\n", arg);
	return 0;
}

int uninit_dsdt() {
	return 0;
}

ARC_REGISTER_DRIVER(3, dsdt) = {
        .index = ARC_DRI_IDSDT,
	.init = init_dsdt,
	.uninit = uninit_dsdt,
};
