/**
 * @file madt.c
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
#include <arch/x86-64/ioapic.h>

#define ENTRY_TYPE_LAPIC               0x00
#define ENTRY_TYPE_IOAPIC              0x01
#define ENTRY_TYPE_INT_OVERRIDE_SRC    0x02
#define ENTRY_TYPE_NMI_SOURCE          0x03
#define ENTRY_TYPE_LAPIC_NMI           0x04
#define ENTRY_TYPE_LAPIC_ADDR_OVERRIDE 0x05
#define ENTRY_TYPE_IOSAPIC             0x06
#define ENTRY_TYPE_LSAPIC              0x07
#define ENTRY_TYPE_PIS                 0x08
#define ENTRY_TYPE_Lx2APIC             0x09
#define ENTRY_TYPE_Lx2APIC_NMI         0x0A
#define ENTRY_TYPE_GICC                0x0B
#define ENTRY_TYPE_GICD                0x0C
#define ENTRY_TYPE_GIC_MSI             0x0D
#define ENTRY_TYPE_GICR                0x0E
#define ENTRY_TYPE_ITS                 0x0F
#define ENTRY_TYPE_MP_WAKEUP           0x10

struct apic {
	struct Arc_RSDTBaseEntry base;
	uint32_t lapic_addr;
	uint32_t flags;
	uint8_t entries[];
}__attribute__((packed));

struct apic_entry_lapic {
	uint8_t type;
	uint8_t length;
	uint8_t acpi_uid;
	uint8_t id;
	uint32_t flags;
}__attribute__((packed));

struct apic_entry_ioapic {
	uint8_t type;
	uint8_t length;
	uint8_t id;
	uint8_t resv0;
	uint32_t address;
	uint32_t gsi_base;
}__attribute__((packed));

int init_apic(struct ARC_Resource *res, void *arg) {
	(void)res;
	struct apic *apic = (struct apic *)arg;

	if (Arc_ChecksumACPI(apic, sizeof(struct apic)) != 0) {
		ARC_DEBUG(ERR, "Invalid checksum\n");
		return -1;
	}

	size_t bytes = apic->base.length - sizeof(struct apic);
	for (size_t i = 0; i < bytes;) {
		int size = apic->entries[i + 1];

		switch (apic->entries[i]) {
		case ENTRY_TYPE_LAPIC: {
			struct apic_entry_lapic *entry = (struct apic_entry_lapic *)&apic->entries[i];
			ARC_DEBUG(INFO, "Found LAPIC (%d, %s, %s)\n", entry->id, (entry->flags & 1) ? "enabled" : "disabled", (entry->flags & 1) ? "ignore" : (((entry->flags >> 1) & 1) ? "online capable" : "not online capable"));

			break;
		}

		case ENTRY_TYPE_IOAPIC: {
			struct apic_entry_ioapic *entry = (struct apic_entry_ioapic *)&apic->entries[i];
			ARC_DEBUG(INFO, "Found IOAPIC (%d, 0x%x, %d)\n", entry->id, entry->address, entry->gsi_base);
			Arc_DefineIOAPIC(entry->id, entry->address, entry->gsi_base);

			break;
		}

		default: {
			ARC_DEBUG(WARN, "Unimplemented MADT type 0x%x\n", apic->entries[i]);
		}
		}

		i += size;
	}

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
