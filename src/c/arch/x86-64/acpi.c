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
        uint8_t revision;
        uint32_t rsdt_addr;
        uint32_t length;
        uint64_t xsdt_addr;
        uint8_t extended_checksum;
        char resv0[3];
}__attribute__((packed));

struct rsdt_entry_header {
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

struct rsdt_header {
        struct rsdt_entry_header entry;
        uint32_t entries[];
}__attribute__((packed));

// NOTE: Use XSDT if present over RSDT
struct xsdt_header {
        struct rsdt_entry_header entry;
        uint64_t entries[];
}__attribute__((packed));

struct entry_apic {
        struct rsdt_entry_header entry;
        uint32_t lapic_address;
        uint32_t flags;
}__attribute__((packed));

struct apic_entry_lapic {
        uint8_t type;
        uint8_t length;
        uint8_t lapic_processor_id;
        uint8_t lapic_id;
        uint32_t flags;
}__attribute((packed));

struct apic_entry_ioapic {
        uint8_t type;
        uint8_t length;
        uint8_t ioapic_id;
        uint8_t resv0;
        uint32_t ioapic_address;
        uint32_t gsi;
}__attribute__((packed));

int rsdp_do_checksum(void *in, int length) {
        int8_t *data = (int8_t *)in;
        int8_t sum = *data;
        for (int i = 1; i < length; i++) {
                sum += *(data + i);
        }

        return sum;
}

int rsdp_parse_apic_v1(struct entry_apic *entry) {
        ARC_DEBUG(INFO, "Parsing APIC information (%s):\n", (entry->flags & 1) ? "PC-AT Compatible" : "not PC-AT Compatible");

        uint8_t *data = ((uint8_t *)entry) + sizeof(struct entry_apic);
        int size = entry->entry.length - sizeof(struct entry_apic);
        for (int i = 0; i < size;) {
                if (data[i] == 0 && data[i + 1] == 8) {
                        struct apic_entry_lapic *lapic = (struct apic_entry_lapic *)(data + i);
                        ARC_DEBUG(INFO, "\tFound LAPIC %d belonging to processor %d (%s)\n", lapic->lapic_id, lapic->lapic_processor_id, (lapic->flags & 1) ? "enabled" : "disabled");
                        i += lapic->length;
                } else if (data[i] == 1 && data[i + 1] == 12) {
                        struct apic_entry_ioapic *ioapic = (struct apic_entry_ioapic *)(data + i);
                        ARC_DEBUG(INFO, "\tFound IOAPIC %d at 0x%x, GSI: %d\n", ioapic->ioapic_id, ioapic->ioapic_address, ioapic->gsi);
                        Arc_DefineIOAPIC(ioapic->ioapic_id, ioapic->ioapic_address, ioapic->gsi);
                        i += ioapic->length;
                } else {
                        i++;
                }
        }

        return 0;
}

int rsdp_parse(struct acpi_rsdp *rsdp) {
        int entry_count = 0;
        uint32_t *entries32 = NULL;
        uint64_t *entries64 = NULL;

        if (rsdp->revision == 0) {
                // No XSDT
                if (rsdp_do_checksum(rsdp, 20) != 0) {
                        ARC_DEBUG(ERR, "RSDP invalid checksum\n");
                        return -1;
                }

                struct rsdt_header *rsdt = (struct rsdt_header *)ARC_PHYS_TO_HHDM(rsdp->rsdt_addr);

                if (rsdp_do_checksum(rsdt, rsdt->entry.length) != 0) {
                        ARC_DEBUG(ERR, "RSDT invalid checksum\n");
                        return -1;
                }

                entry_count = (rsdt->entry.length - sizeof(struct rsdt_entry_header)) / 4;
                entries32 = rsdt->entries;
        } else {
                if (rsdp_do_checksum(rsdp, sizeof(struct acpi_rsdp)) != 0) {
                        ARC_DEBUG(ERR, "RSDP invalid checksum\n");
                        return -1;
                }

                struct xsdt_header *xsdt = (struct xsdt_header *)ARC_PHYS_TO_HHDM(rsdp->xsdt_addr);

                if (rsdp_do_checksum(xsdt, sizeof(struct xsdt_header)) != 0) {
                        ARC_DEBUG(ERR, "XSDT invalid checksum\n");
                        return -1;
                }

                entry_count = (xsdt->entry.length - sizeof(struct rsdt_entry_header)) / 8;
                entries64 = xsdt->entries;
        }

        for (int i = 0; i < entry_count; i++) {
                struct rsdt_entry_header *entry = NULL;
                if (entries32 != NULL) {
                        entry = (struct rsdt_entry_header *)ARC_PHYS_TO_HHDM(entries32[i]);
                } else {
                        entry = (struct rsdt_entry_header *)ARC_PHYS_TO_HHDM(entries64[i]);
                }

                ARC_DEBUG(INFO, "Table %.*s at %p:\n", 4, entry->signature, entry);
                ARC_DEBUG(INFO, "\tLength: %d\n", entry->length);
                ARC_DEBUG(INFO, "\tRevision: %d\n", entry->revision);
                ARC_DEBUG(INFO, "\tOEMID: %.*s\n", 6, entry->OEMID);
                ARC_DEBUG(INFO, "\tOEMID Table: %.*s\n", 8, entry->OEMID_table);

                if (rsdp_do_checksum(entry, entry->length) == 0) {
                        ARC_DEBUG(INFO, "\tStatus: checksum pass\n");
                } else {
                        ARC_DEBUG(ERR, "\tStatus: checksum fail, skipping\n");
                        continue;
                }

                if (strncmp(entry->signature, "APIC", 4) == 0) {
                        rsdp_parse_apic_v1((struct entry_apic *)entry);
                }
        }

        return 0;
}

int Arc_InitializeACPI(uint64_t rsdp_ptr) {
        if (rsdp_ptr == 0) {
                ARC_DEBUG(ERR, "No ACPI given\n");
                return -1;
        }

        ARC_DEBUG(INFO, "Initializing ACPI\n");
        struct acpi_rsdp *rsdp = (struct acpi_rsdp *)ARC_PHYS_TO_HHDM(rsdp_ptr);

        ARC_DEBUG(INFO, "Found RSDP (%p, %d)\n", rsdp, rsdp->revision);
        ARC_DEBUG(INFO, "\tOEMID: %s\n", rsdp->OEMID);
        ARC_DEBUG(INFO, "\tRSDT: 0x%x\n", rsdp->rsdt_addr);

        rsdp_parse(rsdp);

        // TODO: Other things


        ARC_DEBUG(INFO, "Successfully initialized ACPI\n");

        return 0;
}
