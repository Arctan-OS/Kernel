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
#include <arch/x86-64/apic/apic.h>
#include <mm/slab.h>

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

struct dri_state {
	uint8_t *entry;
	struct dri_state *next;
};

int init_apic(struct ARC_Resource *res, void *arg) {
	struct apic *apic = (struct apic *)arg;

	if (Arc_ChecksumACPI(apic, sizeof(struct apic)) != 0) {
		ARC_DEBUG(ERR, "Invalid checksum\n");
		return -1;
	}

	Arc_MutexLock(&res->dri_state_mutex);

	struct dri_state *last = NULL;

	size_t bytes = apic->base.length - sizeof(struct apic);
	for (size_t i = 0; i < bytes;) {
		int size = apic->entries[i + 1];

		struct dri_state *state = (struct dri_state *)Arc_SlabAlloc(sizeof(struct dri_state));

		if (res->driver_state == NULL) {
			res->driver_state = state;
		} else {
			last->next = state;
		}
		last = state;

		state->entry = (void *)&apic->entries[i];

		i += size;
	}

	Arc_MutexUnlock(&res->dri_state_mutex);

	return 0;
}

int uninit_apic() {
	return 0;
};

int read_apic(void *buffer, size_t size, size_t count, struct ARC_File *file, struct ARC_Resource *res) {
	(void)file;

	if (buffer == NULL || size == 0 || count < 0 || res == NULL) {
		return -1;
	}

	Arc_MutexLock(&res->dri_state_mutex);
	struct dri_state *entry = res->driver_state;
	for (size_t i = 0; i < count; i++) {
		entry = entry->next;

		if (entry == NULL) {
			Arc_MutexUnlock(&res->dri_state_mutex);
			return -2;
		}
	}
	Arc_MutexUnlock(&res->dri_state_mutex);

	uint8_t entry_size = *((uint8_t *)entry->entry + 1);
	memcpy((uint8_t *)buffer, entry->entry, size < entry_size ? size : entry_size);

	return entry_size;
}

ARC_REGISTER_DRIVER(3, apic_driver) = {
        .index = ARC_DRI_IAPIC,
        .init = init_apic,
	.uninit = uninit_apic,
	.read = read_apic,
};
