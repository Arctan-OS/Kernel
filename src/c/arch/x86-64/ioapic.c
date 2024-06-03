/**
 * @file ioapic.c
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
#include <arch/x86-64/ioapic.h>
#include <global.h>
#include <mm/slab.h>
#include <mm/vmm.h>
#include <stdint.h>

struct ioapic_definition {
        uint32_t id;
        uint8_t version;
        uint32_t address;
        uint32_t gsi;
        uint8_t max_redir_entry;
        struct ioapic_definition *next;
};
// Backwards (last entry is the first one defined)
struct ioapic_definition *ioapic_list = NULL;

struct ioapic_register {
        /// Register select
        uint32_t ioregsel __attribute__((aligned(16)));
        /// Data
        uint32_t iowin __attribute__((aligned(16)));
}__attribute__((packed));

struct ioapic_redir_tbl {
        uint8_t int_vec;
        uint8_t del_mod : 3;
        uint8_t dest_mod : 1;
        uint8_t del_stat : 1;
        uint8_t int_pol : 1;
        uint8_t irr : 1;
        uint8_t trigger : 1;
        uint8_t mask : 1;
        uint64_t resv0 : 39;
        uint8_t destination;
}__attribute__((packed));

int Arc_DefineIOAPIC(uint32_t id, uint32_t address, uint32_t gsi) {
        struct ioapic_definition *def = (struct ioapic_definition *)Arc_SlabAlloc(sizeof(struct ioapic_definition));

        if (def == NULL) {
                return -1;
        }

        def->id = id;
        def->address = address;
        def->gsi = gsi;

        if (ioapic_list != NULL) {
                def->next = ioapic_list;
        }

        ioapic_list = def;

        return 0;
}

int Arc_InitIOAPIC() {
        ARC_DEBUG(INFO, "Initializing IOAPIC(s)\n");

        // TODO: Mask all IRQs on 8259

        ARC_DEBUG(INFO, "IOAPIC Definition:\n");
        struct ioapic_definition *def = ioapic_list;
        while (def != NULL) {
                struct ioapic_register *reg = (struct ioapic_register *)ARC_PHYS_TO_HHDM(def->address);

                Arc_MapPageVMM(def->address, (uintptr_t)reg, ARC_VMM_CREAT_FLAG | 3 | ARC_VMM_PAT_UC(0));

                reg->ioregsel = 0x01;
                int version = reg->iowin & 0xFF;
                int mre = (reg->iowin >> 16) & 0xFF;
                ARC_DEBUG(INFO, "\tIOAPIC %d (%p, %d) VERSION: %d MRE: %d+1\n", def->id, reg, def->gsi, version, mre);

                def->version = version;
                def->max_redir_entry = mre + 1;

                // TODO: Map IOAPICs to send interrupts above
                //       reserved interrupt vectors

                def = def->next;
        }
        ARC_DEBUG(INFO, "Initialized IOAPIC(s)\n");

        return 0;
}
