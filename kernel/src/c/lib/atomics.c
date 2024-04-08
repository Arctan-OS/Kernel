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
#include <mp/sched/abstract.h>

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

int64_t Arc_QLock(struct ARC_QLock *head) {
	// TODO: Replace
	int64_t tid = Arc_GetCurrentTID();

	if (head->next != NULL && ((struct internal_qlock_node *)head->next)->tid == tid) {
		return tid;
	}

	register struct internal_qlock_node *next = (struct internal_qlock_node *)Arc_SlabAlloc(sizeof(struct internal_qlock_node));

	if (next == NULL) {
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

	return tid;
}

void Arc_QYield(struct ARC_QLock *head, int64_t tid) {
	int64_t current_tid = ((struct internal_qlock_node *)head->next)->tid;

	if (current_tid == tid) {
		return;
	}

	while (current_tid != tid) {
		Arc_YieldCPU(current_tid);
	}
}

int Arc_QUnlock(struct ARC_QLock *head) {
	struct internal_qlock_node *next = ((struct internal_qlock_node *)head->next)->next;

	if (head->next != NULL && ((struct internal_qlock_node *)head->next)->tid != Arc_GetCurrentTID()){
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
	return 0;
}

int Arc_MutexUnlock(ARC_GenericMutex *mutex) {
	// Atomically unlock
	return 0;
}
