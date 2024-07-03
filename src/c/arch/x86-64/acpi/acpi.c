/**
 * @file acpi.c
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
#include "arctan.h"
#include <arch/x86-64/acpi/acpi.h>
#include <fs/vfs.h>
#include <global.h>

int Arc_ChecksumACPI(void *data, size_t length) {
	int8_t *bytes = (int8_t *)data;
	int8_t sum = *bytes;

	for (size_t i = 1; i < length; i++) {
		sum += bytes[i];
	}

	return sum;
}

int Arc_InitializeACPI(uint64_t rsdp_ptr) {
	if (rsdp_ptr == 0) {
		ARC_DEBUG(ERR, "NULL address provided\n");
		return -1;
	}

        if (Arc_CreateVFS("/dev/acpi/", 0, ARC_VFS_N_DIR, NULL) != 0) {
                ARC_DEBUG(ERR, "Failed to create ACPI directory\n");
                return -1;
        }

	rsdp_ptr = ARC_PHYS_TO_HHDM(rsdp_ptr);

        Arc_CreateVFS("/dev/acpi/rsdt", 0, ARC_VFS_N_DIR, NULL);
        struct ARC_Resource *rsdt = Arc_InitializeResource("/dev/acpi/rsdt", ARC_DRI_ACPI, ARC_DRI_IRSDT, (void *)rsdp_ptr);
        Arc_MountVFS("/dev/acpi/rsdt", rsdt, ARC_VFS_FS_DEV);

        return 0;
}
