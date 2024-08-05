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
#include <lib/util.h>
#include <mp/sched/abstract.h>
#include <global.h>
struct internal_qlock_node {
	int64_t tid;
	struct internal_qlock_node *next;
};

int init_qlock(struct ARC_QLock **lock) {
	*lock = alloc(sizeof(struct ARC_QLock));

	if (*lock == NULL) {
		return 1;
	}

	memset(*lock, 0, sizeof(struct ARC_QLock));

	// Queue lock header is now created, Arc_QLock
	// Arc_QUnlock, and Arc_QYield can now be called

	return 0;
}

int init_static_qlock(struct ARC_QLock *head) {
	memset(head, 0, sizeof(struct ARC_QLock));

	return 0;
}

int qlock(struct ARC_QLock *head) {
	if (head == NULL) {
		ARC_DEBUG(ERR, "Head is NULL\n");
		return -4;
	}

	if (head->is_frozen) {
		ARC_DEBUG(ERR, "Head %p is frozen!\n", head);
		return -3;
	}

	int64_t tid = get_current_tid();

	if (head->next != NULL && ((struct internal_qlock_node *)head->next)->tid == tid) {
		return 0;
	}

	register struct internal_qlock_node *next = (struct internal_qlock_node *)alloc(sizeof(struct internal_qlock_node));

	if (next == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate next link\n");
		return -2;
	}

	memset(next, 0, sizeof(struct internal_qlock_node));

	next->tid = tid;

	mutex_unlock(&head->lock);

	if (head->last == NULL) {
		head->next = next;
	} else {
		((struct internal_qlock_node *)head->last)->next = next;
	}

	head->last = next;

	mutex_unlock(&head->lock);

	return 0;
}

void qlock_yield(struct ARC_QLock *head) {
	if (head == NULL) {
		ARC_DEBUG(ERR, "Head is NULL\n");
		// TODO: What to do?
	}
	int64_t current_tid = ((struct internal_qlock_node *)head->next)->tid;

	if (current_tid == get_current_tid()) {
		return;
	}

	while (current_tid != get_current_tid()) {
		yield_cpu(current_tid);
	}
}

int qunlock(struct ARC_QLock *head) {
	if (head == NULL) {
		ARC_DEBUG(ERR, "Head is NULL\n");
		return -4;
	}

	struct internal_qlock_node *current = (struct internal_qlock_node *)head->next;

	if (current == NULL) {
		ARC_DEBUG(ERR, "No-one owns the lock\n");
		return -2;
	}

	struct internal_qlock_node *next = current->next;

	if (current->tid != get_current_tid()) {
		ARC_DEBUG(ERR, "Lock is not owned by 0x%"PRIx64" (!= 0x%"PRIx64")\n", get_current_tid(), current->tid);
		return -1;
	}

	mutex_lock(&head->lock);

	free(head->next);

	head->next = next;

	if (next == NULL) {
		head->last = NULL;
	}

	mutex_unlock(&head->lock);

	return 0;
}

int qlock_freeze(struct ARC_QLock *head) {
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
int qlock_thaw(struct ARC_QLock *head) {
	// Thaw a frozen lock
	if (head->is_frozen == 0) {
		return 0;
	}

	if (((struct internal_qlock_node *)head->next)->tid != get_current_tid()) {
		return -1;
	}

	head->is_frozen = 0;

	return 0;
}

int init_mutex(ARC_GenericMutex **mutex) {
	if (mutex == NULL) {
		return 1;
	}

	*mutex = (ARC_GenericMutex *)alloc(sizeof(ARC_GenericMutex));

	if (*mutex == NULL) {
		return 1;
	}

	memset(*mutex, 0, sizeof(ARC_GenericMutex));

	return 0;
}

int uninit_mutex(ARC_GenericMutex *mutex) {
	free(mutex);

	return 0;
}

int init_static_mutex(ARC_GenericMutex *mutex) {
	if (mutex == NULL) {
		return 1;
	}

	memset(mutex, 0, sizeof(ARC_GenericMutex));

	return 0;
}

int mutex_lock(ARC_GenericMutex *mutex) {
	if (mutex == NULL) {
		return 1;
	}

	// NOTE: In its current status, this functions identical to
	//       a spinlock
	while (__atomic_test_and_set(mutex, __ATOMIC_ACQUIRE)) {
		// TODO: Yield the CPU to another task, wake up
		//       once the mutex is unlocked
	}

	return 0;
}

int mutex_unlock(ARC_GenericMutex *mutex) {
	if (mutex == NULL) {
		return 1;
	}

	__atomic_clear(mutex, __ATOMIC_RELEASE);

	return 0;
}

int init_static_spinlock(ARC_GenericSpinlock *spinlock) {
	if (spinlock == NULL) {
		return 1;
	}

	memset(spinlock, 0, sizeof(ARC_GenericSpinlock));

	return 0;
}

int spinlock_lock(ARC_GenericSpinlock *spinlock) {
	if (spinlock == NULL) {
		return 1;
	}

	while (__atomic_test_and_set(spinlock, __ATOMIC_ACQUIRE));

	return 0;
}

int spinlock_unlock(ARC_GenericSpinlock *spinlock) {
	if (spinlock == NULL) {
		return 1;
	}

	__atomic_clear(spinlock, __ATOMIC_RELEASE);

	return 0;
}
