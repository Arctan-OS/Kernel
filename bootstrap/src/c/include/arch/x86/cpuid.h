/**
 * @file cpuid.h
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
 * A file to check and enable various CPU features.
*/
#ifndef ARC_ARCH_X86_CPUID_H
#define ARC_ARCH_X86_CPUID_H

/**
 * Check for CPU features.
 *
 * Checks for various CPU features, currently also enables
 * these features. if PAE, Extended CPUID functions, or LM
 * are not supported, this function will hang.
 *
 * @return An integer specifying which features are supported.
 * */
int check_features();
/**
 * Unused
 * */
int enable_features();

#endif
