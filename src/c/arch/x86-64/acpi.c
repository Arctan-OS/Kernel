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
#include <arch/x86-64/acpi.h>
#include <arch/x86-64/apic.h>
#include <global.h>
#include <util.h>

struct acpi_rsdp {
        char signature[8];
        uint8_t checksum;
        char OEMID[6];
        uint8_t resv0;
        uint32_t rsdt_addr;
}__attribute__((packed));

struct acpi_rsdp2 {
        char signature[8];
        uint8_t checksum;
        char OEMID[6];
        uint8_t revision;
        uint32_t rsdt_addr;
        uint32_t length;
        uint64_t xsdt_addr;
        uint8_t extended_checksum;
        char resv0[3];
}__attribute__((packed));

struct rsdt_entry_header_v1 {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char OEMID[6];
        char OEMID_table[8];
        uint32_t OEM_rev;
        uint32_t creator_id;
        uint32_t creator_rev;
}__attribute__((packed));

struct rsdt_header_v1 {
        struct rsdt_entry_header_v1 entry;
        uint32_t entries[];
}__attribute__((packed));

struct rsdt_apic_v1 {
        struct rsdt_entry_header_v1 entry;
        uint32_t lapic_address;
        uint32_t flags;
}__attribute__((packed));

struct apic_lapic_v1 {
        uint8_t type;
        uint8_t length;
        uint8_t lapic_processor_id;
        uint8_t lapic_id;
        uint32_t flags;
}__attribute((packed));

struct apic_ioapic_v1 {
        uint8_t type;
        uint8_t length;
        uint8_t ioapic_id;
        uint8_t resv0;
        uint32_t ioapic_address;
        uint32_t gsi;
}__attribute__((packed));

int rsdp_do_checksum_v1(void *in, int length) {
        int8_t *data = (int8_t *)in;
        int8_t sum = *data;
        for (int i = 1; i < length; i++) {
                sum += *(data + i);
        }

        return sum;
}

int rsdp_parse_apic_v1(struct rsdt_apic_v1 *entry) {
        ARC_DEBUG(INFO, "Parsing APIC information (%s):\n", (entry->flags & 1) ? "PC-AT Compatible" : "not PC-AT Compatible");

        uint8_t *data = ((uint8_t *)entry) + sizeof(struct rsdt_apic_v1);
        int size = entry->entry.length - sizeof(struct rsdt_apic_v1);
        for (int i = 0; i < size;) {
                if (data[i] == 0 && data[i + 1] == 8) {
                        struct apic_lapic_v1 *lapic = (struct apic_lapic_v1 *)(data + i);
                        ARC_DEBUG(INFO, "\tFound LAPIC %d belonging to processor %d (%s)\n", lapic->lapic_id, lapic->lapic_processor_id, (lapic->flags & 1) ? "enabled" : "disabled");
                        i += lapic->length;
                } else if (data[i] == 1 && data[i + 1] == 12) {
                        struct apic_ioapic_v1 *ioapic = (struct apic_ioapic_v1 *)(data + i);
                        ARC_DEBUG(INFO, "\tFound IOAPIC %d at 0x%x, GSI: %d\n", ioapic->ioapic_id, ioapic->ioapic_address, ioapic->gsi);
                        Arc_DefineIOAPIC(ioapic->ioapic_id, ioapic->ioapic_address, ioapic->gsi);
                        i += ioapic->length;
                } else {
                        i++;
                }
        }


        return 0;
}

int rsdp_parse_v1(struct acpi_rsdp *rsdp) {
        struct rsdt_header_v1 *rsdt = (struct rsdt_header_v1 *)ARC_PHYS_TO_HHDM(rsdp->rsdt_addr);

        if (rsdp_do_checksum_v1(rsdt, rsdt->entry.length) != 0) {
                ARC_DEBUG(ERR, "RSDT invalid checksum\n");
                return -1;
        }

        int entry_count = (rsdt->entry.length - sizeof(struct rsdt_entry_header_v1)) / 4;
        for (int i = 0; i < entry_count; i++) {
                struct rsdt_entry_header_v1 *entry = (struct rsdt_entry_header_v1 *)ARC_PHYS_TO_HHDM(rsdt->entries[i]);

                ARC_DEBUG(INFO, "Table %.*s at %p:\n", 4, entry->signature, entry);
                ARC_DEBUG(INFO, "\tLength: %d\n", entry->length);
                ARC_DEBUG(INFO, "\tRevision: %d\n", entry->revision);
                ARC_DEBUG(INFO, "\tOEMID: %.*s\n", 6, entry->OEMID);
                ARC_DEBUG(INFO, "\tOEMID Table: %.*s\n", 8, entry->OEMID_table);

                if (rsdp_do_checksum_v1(entry, entry->length) == 0) {
                        ARC_DEBUG(INFO, "\tStatus: checksum pass\n");
                } else {
                        ARC_DEBUG(ERR, "\tStatus: checksum fail, skipping\n");
                        continue;
                }

                if (strncmp(entry->signature, "APIC", 4) == 0) {
                        rsdp_parse_apic_v1((struct rsdt_apic_v1 *)entry);
                }

        }

        return 0;
}

int Arc_InitializeACPI(uint64_t rsdp_ptr, int rsdp_ver) {
        if (rsdp_ptr == 0) {
                ARC_DEBUG(ERR, "No ACPI given\n");
                return -1;
        }

        ARC_DEBUG(INFO, "Initializing ACPI\n");

        if (rsdp_ver == 1) {
                struct acpi_rsdp *rsdp = (struct acpi_rsdp *)ARC_PHYS_TO_HHDM(rsdp_ptr);

                if (rsdp_do_checksum_v1(rsdp, sizeof(struct acpi_rsdp)) != 0) {
                        ARC_DEBUG(ERR, "RSDP invalid checksum\n");
                        return -1;
                }

                ARC_DEBUG(INFO, "Found RSDP (%p, %d)\n", rsdp, rsdp_ver);
                ARC_DEBUG(INFO, "\tOEMID: %s\n", rsdp->OEMID);
                ARC_DEBUG(INFO, "\tRSDT: 0x%x\n", rsdp->rsdt_addr);

                rsdp_parse_v1(rsdp);
        } else {
                ARC_DEBUG(ERR, "RSDPv2 unimplemented\n");
                return -1;
        }

        ARC_DEBUG(INFO, "Successfully initialized ACPI\n");

        return 0;
}
