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

#define ROOT_CHR 0x5C
#define PARENT_PREFIX_CHR 0x5E

#endif
