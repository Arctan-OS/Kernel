/**
 * @file atomics.c
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
#include <mm/allocator.h>
#include <lib/atomics.h>
#include <util.h>

int Arc_QLockInit(struct ARC_QLock **head) {
	*head = Arc_SlabAlloc(sizeof(struct ARC_QLock));

	if (*head == NULL) {
		return 1;
	}

	memset(*head, 0, sizeof(struct ARC_QLock));

	// Queue lock header is now created, Arc_QLock
	// Arc_QUnlock, and Arc_QYield can now be called

	return 0;
}

int Arc_QLock(struct ARC_QLock *head) {
	// Mutex Lock
	//   Yield if head is locked

	uint64_t *next = (uint64_t *)Arc_SlabAlloc(sizeof(uint64_t));

	if (next == NULL) {
		// Mutex Unlock
		return 1;
	}

	*next = 0;

	if (head->last != NULL) {
		*(head->last) = (uint64_t)next;
	} else {
		head->next = next;
	}

	head->last = next;

	// Mutex Unlock

	// Yield while head->next != next

	return 0;
}

int Arc_QUnlock(struct ARC_QLock *head) {
	// Mutex Lock

	uint64_t *next = (uint64_t *)*(head->next);

	Arc_SlabFree(head->next);

	head->next = next;

	if (next == NULL) {
		head->last = NULL;
	}

	// Mutex Unlock

	return 0;
}
