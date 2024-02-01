/**
 * @file global.h
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
#ifndef ARC_GLOBAL_H
#define ARC_GLOBAL_H

#include <multiboot/multiboot2.h>
#include <arctan.h>
#include <mm/freelist.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

#define ARC_HANG for (;;) __asm__("hlt");

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

#define ASSERT(cond) if (!(cond)) {					\
			printf("Assertion %s failed (%s:%d)\n", #cond, __FILE__, __LINE__); \
			for (;;); \
		     }
#define ALIGN(v, a) ((v + (a - 1)) & ~(a - 1))

#define max(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

#define min(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })


int strcmp(char *a, char *b);
int memcpy(void *a, void *b, size_t size);
void memset(void *mem, uint8_t value, size_t size);

extern struct ARC_FreelistMeta physical_mem;
extern uint64_t highest_address;
extern void *kernel_elf;
extern struct ARC_BootMeta _boot_meta;
extern uint8_t __BOOTSTRAP_END__;
extern void *initramfs;
extern uint32_t initramfs_size;
extern uint64_t *pml4;

#endif
