/**
 * @file smp.h
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
#include <stdint.h>
#include <mp/sched/context.h>
#include <stdarg.h>

struct ARC_ProcessorDescriptor {
	uint32_t processor;
	uint32_t acpi_uid;
	uint32_t acpi_flags;
	uint32_t bist;
	uint32_t model_info;
	uint32_t status;
	// Bit | Description
	// 0   | Initialized
	// 1   | Holding
	struct ARC_Registers registers;
	ARC_GenericMutex register_lock;
	uint32_t flags;
	// Bit | Description
	// 0   | 1: Signals external modification of register state, cleared once
	//          changes have been accepted
	// 1   | 1: Write all registers to the registers member of this structure
	// 2   | 1: Timer values have been changed, cleared once changes have
	//          been accepted
	struct ARC_ProcessorDescriptor *next;
	uint32_t timer_ticks;
	uint32_t timer_mode;
	ARC_GenericMutex timer_lock;
};

// NOTE: The index in Arc_ProcessorList corresponds to the LAPIC ID
//       acquired from CPUID 0x1, ebx >> 24
extern struct ARC_ProcessorDescriptor Arc_ProcessorList[256];
// NOTE: Since the processor structure includes a next pointer,
//       this allows us to also traverse the list by processor,
//       while still being able to address the list by LAPIC ID
//       in O(1) time
extern struct ARC_ProcessorDescriptor *Arc_BootProcessor;
extern uint32_t Arc_ProcessorCounter;

int smp_hold(struct ARC_ProcessorDescriptor *processor);

/**
 * Write the given context to the given processor.
 *
 * NOTE: processor->register_lock is expected to be held.
 * */
int smp_context_write(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs);
/**
 * Write the processor's context to the given registers structure.
 *
 * NOTE: processor->register_lock is expected to be held.
 * */
int smp_context_save(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs);

int smp_jmp(struct ARC_ProcessorDescriptor *processor, void *function, uint32_t argc, ...);

int smp_far_jmp(struct ARC_ProcessorDescriptor *processor, uint32_t cs, void *function, uint32_t argc, ...);


int smp_list_aps();

/**
 * Initialize an AP into an SMP system
 *
 * Initializes the given AP into an SMP system by creating the
 * relevant processor descriptors and sending INIT and START IPIs.
 *
 * NOTE: This functions is meant to only be called from the BSP.
 * NOTE: This function should be called to initialize the BSP as well
 *       since it needs to be inserted into the descriptor list.
 * @param uint32_t lapic - The ID of the LAPIC to initialize.
 * @param uint32_t acpi_uid - The UID given to the processor by ACPI.
 * @param uint32_t acpi_flags - The flags given by ACPI.
 * @param uint32_t version - The version of the LAPIC.
 * @return zero upon success.
 * */
int init_smp(uint32_t lapic, uint32_t acpi_uid, uint32_t acpi_flags, uint32_t version);
