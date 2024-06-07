/**
 * @file apic.c
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
#include <stdint.h>
#include <global.h>
#include <util.h>
#include <arch/x86-64/acpi.h>

int init_apic(struct ARC_Resource *res, void *arg) {
	(void)res;
	(void)arg;
	ARC_DEBUG(INFO, "The APIC %p\n", arg);
	return 0;
}

int uninit_apic() {
	return 0;
};

ARC_REGISTER_DRIVER(3, apic_driver) = {
        .index = ARC_DRI_IAPIC,
        .init = init_apic,
	.uninit = uninit_apic
};
