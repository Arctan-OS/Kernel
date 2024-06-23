/**
 * @file ops.h
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
#ifndef ARC_ARCH_X86_64_ACPI_CAML_OPS_H
#define ARC_ARCH_X86_64_ACPI_CAML_OPS_H

#define ZERO_OP 0x00
#define ONE_OP  0x01
#define ALIAS_OP 0x06
#define NAME_OP 0x08
#define BYTE_PREFIX 0xA
#define WORD_PREFIX 0xB
#define DWORD_PREFIX 0xC
#define STRING_PREFIX 0xD
#define QWORD_PREFIX 0xE
#define SCOPE_OP 0x10
#define BUFFER_OP 0x11
#define PACKAGE_OP 0x12
#define VAR_PACKAGE_OP 0x13
#define METHOD_OP 0x14
#define EXTERN_OP 0x15
#define DUAL_NAME_PREFIX 0x2E
#define MULTI_NAME_PREFIX 0x2F
#define IS_DIGIT_CHR(byte) (byte >= 0x30 && byte <= 0x39)
#define IS_NAME_CHR(byte) ((byte >= 0x41 && byte <= 0x5A) || byte == 0x5F)

#define EXTOP_PREFIX 0x5B
#define EXTOP_MUTEX_OP 0x01
#define EXTOP_EVENT_OP 0x02
#define EXTOP_COND_REF_OF_OP 0x12
#define EXTOP_CREATE_FIELD_OP 0x13
#define EXTOP_LOAD_TABLE_OP 0x1F
#define EXTOP_LOAD_OP 0x20
#define EXTOP_STALL_OP 0x21
#define EXTOP_SLEEP_OP 0x22
#define EXTOP_ACQUIRE_OP 0x23
#define EXTOP_SIGNAL_OP 0x24
#define EXTOP_WAIT_OP 0x25
#define EXTOP_RESET_OP 0x26
#define EXTOP_RELEASE_OP 0x27
#define EXTOP_FROM_BCD_OP 0x28
#define EXTOP_TO_BCD_OP 0x29
#define EXTOP_REVISION_OP 0x30
#define EXTOP_DEBUG_OP 0x31
#define EXTOP_FATAL_OP 0x32
#define EXTOP_TIMER_OP 0x33
#define EXTOP_REGION_OP 0x80
#define EXTOP_FIELD_OP 0x81
#define EXTOP_DEVICE_OP 0x82
#define EXTOP_PWR_RESET_OP 0x84
#define EXTOP_THERMAL_ZONE_OP 0x85
#define EXTOP_IDX_FIELD_OP 0x86
#define EXTOP_BANK_FIELD_OP 0x87
#define EXTOP_DATA_REGION_OP 0x88

#define ROOT_CHR 0x5C
#define PARENT_PREFIX_CHR 0x5E

#define ONES_OP 0xFF

#endif
