/**
 * @file rsdt.c
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

struct rsdp {
	char signature[8];
	uint8_t checksum;
	uint8_t OEMID[6];
	uint8_t revision;
	uint32_t rsdt_addr;
	uint32_t length;
	uint64_t xsdt_addr;
	uint8_t ext_checksum;
	uint8_t resv0[3];
}__attribute__((packed));

struct rsdt {
	struct Arc_RSDTBaseEntry base;
	uint32_t entries[];
}__attribute__((packed));

struct xsdt {
	struct Arc_RSDTBaseEntry base;
	uint64_t entries[];
}__attribute__((packed));

int do_rsdt(void *address) {
	struct rsdt *table = (struct rsdt *)address;

	if (strncmp("RSDT", table->base.signature, 4) != 0) {
		return -1;
	}

	if (Arc_ChecksumACPI(table, table->base.length) != 0) {
		return -1;
	}

	ARC_DEBUG(INFO, "RSDT:\n");
	ARC_DEBUG(INFO, "\tRevision: %d\n", table->base.revision);
	ARC_DEBUG(INFO, "\tLength: %d B\n", table->base.length);
	ARC_DEBUG(INFO, "\tOEMID: %.*s\n", 6, table->base.OEMID);
	ARC_DEBUG(INFO, "\tOEMTID: %.*s\n", 8, table->base.OEMTID);
	ARC_DEBUG(INFO, "\tOEMREV: %d\n", table->base.OEMREV);
	ARC_DEBUG(INFO, "\tCID: %.*s\n", 4, table->base.creator_id);
	ARC_DEBUG(INFO, "\tCREV: %d\n", table->base.creator_rev);

	int entries = (table->base.length - sizeof(struct rsdt)) / 4;
	for (int i = 0; i < entries; i++) {
		struct Arc_RSDTBaseEntry *entry = (void *)ARC_PHYS_TO_HHDM(table->entries[i]);

		int index = -1;
		char *path = NULL;

		ARC_DEBUG(INFO, "%d: %.*s (%d)\n", i, 4, entry->signature, strncmp(entry->signature, "APIC", 4));

		if (strncmp(entry->signature, "APIC", 4) == 0) {
			index = ARC_DRI_IAPIC;
			path = "/dev/acpi/rsdt/apic/";
		}

		if (index == -1 || path == NULL) {
			continue;
		}

		struct ARC_Resource *res = Arc_InitializeResource(path, ARC_DRI_ACPI, index, (void *)entry);
		Arc_CreateVFS(path, 0, ARC_VFS_N_DIR);
		Arc_MountVFS(path, res, ARC_VFS_FS_DEV);
	}

	return 0;
}

int do_xsdt(void *address) {
	struct xsdt *table = (struct xsdt *)address;
	ARC_DEBUG(INFO, "XSDT table\n");
	return 0;
}

int init_rsdt(struct ARC_Resource *res, void *arg) {
	(void)res;

	struct rsdp *rsdp = (struct rsdp *)arg;

	if (rsdp->revision == 0 && Arc_ChecksumACPI(rsdp, 20) == 0) {
		// Revision 0
		return do_rsdt((void *)ARC_PHYS_TO_HHDM(rsdp->rsdt_addr));
	} else if (Arc_ChecksumACPI(rsdp, sizeof(struct rsdp)) == 0) {
		// Revision N
		return do_xsdt((void *)ARC_PHYS_TO_HHDM(rsdp->xsdt_addr));
	} else {
		// Fail
		ARC_DEBUG(INFO, "Invalid checksum\n");
		return -1;
	}

	// No idea how you got here
	return -2;
}

int uninit_rsdt() {
	return 0;
};

ARC_REGISTER_DRIVER(3, rsdt_driver) = {
        .index = ARC_DRI_IRSDT,
        .init = init_rsdt,
	.uninit = uninit_rsdt
};
