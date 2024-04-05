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

struct internal_qlock_node {
	uint64_t tid;
	struct internal_qlock_node *next;
};

int Arc_QLockInit(struct ARC_QLock **lock) {
	*lock = Arc_SlabAlloc(sizeof(struct ARC_QLock));

	if (*lock == NULL) {
		return 1;
	}

	memset(*lock, 0, sizeof(struct ARC_QLock));

	// Queue lock header is now created, Arc_QLock
	// Arc_QUnlock, and Arc_QYield can now be called

	return 0;
}

int Arc_QLockInitStatic(struct ARC_QLock **head) {
	memset(*head, 0, sizeof(struct ARC_QLock));

	return 0;
}

int Arc_QLock(struct ARC_QLock *head) {
	// Mutex Lock
	//   Yield if head is locked

	register struct internal_qlock_node *next = (struct internal_qlock_node *)Arc_SlabAlloc(sizeof(struct internal_qlock_node));

	if (next == NULL) {
		// Mutex Unlock
		return 1;
	}

	memset(next, 0, sizeof(struct internal_qlock_node));

	next->tid = 0; // get_current_thread_id();

	if (head->last == NULL) {
		((struct internal_qlock_node *)head->last)->next = next;
	} else {
		head->last = next;
	}

	head->last = next;

	// Mutex Unlock

	// Yield to head->next while head->next != next

	return 0;
}

int Arc_QUnlock(struct ARC_QLock *head) {
	// Mutex Lock

	struct internal_qlock_node *next = ((struct internal_qlock_node *)head->next)->next;

	// if (head->next->tid != get_current_thread_id()) { return 1; }

	Arc_SlabFree(head->next);

	head->next = next;

	if (next == NULL) {
		head->last = NULL;
	}

	// Mutex Unlock

	return 0;
}
