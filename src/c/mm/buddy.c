/**
 * @file buddy.c
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

#include <mm/buddy.h>
#include <global.h>
void *Arc_BuddyAlloc(size_t size) {
        // Get the top node of the current context
        // Descend down tree until suitable size is reached
        // If the smallest size is reached:
        //    If size is still vastly smaller than
        //    size, subdivide
        // Return block
        return NULL;
}

int Arc_BuddyFree(void *address) {
        // Traverse tree, if a node is found
        // with base == address, check for children
        //   If children:
        //      Traverse to next child with
        //      base == address, continue
        //      until there are no more children
        // Mark node as free
        return 0;
}

int Arc_InitBuddy(void *vaddr, size_t total, int subdivisions) {
        ARC_DEBUG(INFO, "Initializing new buddy allocator (%d bytes %d) at %p\n\n", vaddr);

        // Round down the size to nearest 2^subdivisions
        // Allocate rounded size
        // Create root

        return 0;
}
