/**
 * @file util.h
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
#ifndef ARC_UTIL_H
#define ARC_UTIL_H

#include <global.h>

#ifdef ARC_DEBUG_ENABLE

#include <interface/printf.h>

#define ARC_DEBUG_NAME_STR "[BOOTSTRAP "__FILE__"]"
#define ARC_DEBUG_NAME_SEP_STR " : "
#define ARC_DEBUG_INFO_STR "[INFO]"
#define ARC_DEBUG_WARN_STR "[WARNING]"
#define ARC_DEBUG_ERR_STR  "[ERROR]"

#define ARC_DEBUG(__level__, ...) ARC_DEBUG_##__level__(__VA_ARGS__)
#define ARC_DEBUG_INFO(...) printf(ARC_DEBUG_INFO_STR ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __VA_ARGS__);
#define ARC_DEBUG_WARN(...) printf(ARC_DEBUG_WARN_STR ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __VA_ARGS__);
#define ARC_DEBUG_ERR(...)  printf(ARC_DEBUG_ERR_STR  ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __VA_ARGS__);

#else

#define ARC_DEBUG(__level, ...) ;

#endif // ARC_DEBUG_ENABLE

int strcmp(char *a, char *b);
int memcpy(void *a, void *b, size_t size);
void memset(void *mem, uint8_t value, size_t size);

#endif
