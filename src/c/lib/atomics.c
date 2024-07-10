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
#include <mm/slab.h>
#include <lib/atomics.h>
#include <lib/util.h>
#include <mp/sched/abstract.h>
#include <global.h>
struct internal_qlock_node {
	int64_t tid;
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

int Arc_QLockStaticInit(struct ARC_QLock *head) {
	memset(head, 0, sizeof(struct ARC_QLock));

	return 0;
}

int Arc_QLock(struct ARC_QLock *head) {
	if (head->is_frozen) {
		ARC_DEBUG(ERR, "Head %p is frozen!\n", head);
		return -3;
	}

	int64_t tid = Arc_GetCurrentTID();

	if (head->next != NULL && ((struct internal_qlock_node *)head->next)->tid == tid) {
		return 0;
	}

	register struct internal_qlock_node *next = (struct internal_qlock_node *)Arc_SlabAlloc(sizeof(struct internal_qlock_node));

	if (next == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate next link\n");
		return -2;
	}

	memset(next, 0, sizeof(struct internal_qlock_node));

	next->tid = tid;

	Arc_MutexUnlock(&head->lock);

	if (head->last == NULL) {
		head->next = next;
	} else {
		((struct internal_qlock_node *)head->last)->next = next;
	}

	head->last = next;

	Arc_MutexUnlock(&head->lock);

	return 0;
}

void Arc_QYield(struct ARC_QLock *head) {
	int64_t current_tid = ((struct internal_qlock_node *)head->next)->tid;

	if (current_tid == Arc_GetCurrentTID()) {
		return;
	}

	while (current_tid != Arc_GetCurrentTID()) {
		Arc_YieldCPU(current_tid);
	}
}

int Arc_QUnlock(struct ARC_QLock *head) {
	struct internal_qlock_node *next = ((struct internal_qlock_node *)head->next)->next;

	if (head->next == NULL || ((struct internal_qlock_node *)head->next)->tid != Arc_GetCurrentTID()) {
		ARC_DEBUG(ERR, "Lock is not owned by %d or is owned by no-one\n", Arc_GetCurrentTID());
		return -1;
	}

	Arc_MutexLock(&head->lock);

	Arc_SlabFree(head->next);

	head->next = next;

	if (next == NULL) {
		head->last = NULL;
	}

	Arc_MutexUnlock(&head->lock);

	return 0;
}

int Arc_QFreeze(struct ARC_QLock *head) {
	// Freeze the lock, let no new owners in,
	// but advance through all of the other owners
	// already in the queue.
	//
	// If the owner we are currently getting through
	// calls this function, return an error, as it is
	// already frozen.
	//
	// Once all other owners have been removed from the
	// queue, call original owner who invoked this function.
	//
	// Note: The calling thread must own the lock to freeze
	// it.

	if (head->is_frozen == 1) {
		ARC_DEBUG(ERR, "Lock is already frozen!\n");
		return -1;
	}

	head->is_frozen = 1;

	// TODO: Implement yielding

	return 0;
}
int Arc_QThaw(struct ARC_QLock *head) {
	// Thaw a frozen lock
	if (head->is_frozen == 0) {
		return 0;
	}

	if (((struct internal_qlock_node *)head->next)->tid != Arc_GetCurrentTID()) {
		return -1;
	}

	head->is_frozen = 0;

	return 0;
}

int Arc_MutexInit(ARC_GenericMutex **mutex) {
	if (mutex == NULL) {
		return 1;
	}

	*mutex = (ARC_GenericMutex *)Arc_SlabAlloc(sizeof(ARC_GenericMutex));

	if (*mutex == NULL) {
		return 1;
	}

	memset(*mutex, 0, sizeof(ARC_GenericMutex));

	return 0;
}

int Arc_MutexUninit(ARC_GenericMutex *mutex) {
	Arc_SlabFree(mutex);

	return 0;
}

int Arc_MutexStaticInit(ARC_GenericMutex *mutex) {
	if (mutex == NULL) {
		return 1;
	}

	memset(mutex, 0, sizeof(ARC_GenericMutex));

	return 0;
}

int Arc_MutexLock(ARC_GenericMutex *mutex) {
	// Atomically lock and yield if it is locked
	(void)mutex;
	return 0;
}

int Arc_MutexUnlock(ARC_GenericMutex *mutex) {
	// Atomically unlock
	(void)mutex;
	return 0;
}
