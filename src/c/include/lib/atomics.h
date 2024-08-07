/**
 * @file atomics.h
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
#ifndef ARC_LIB_ATOMICS_H
#define ARC_LIB_ATOMICS_H

#include <stdatomic.h>
#include <stdbool.h>

#define ARC_MEM_BARRIER __asm__("" ::: "memory");
// TODO: Fences


/// Generic spinlock
typedef _Atomic int ARC_GenericSpinlock;

/// Generic mutex
typedef _Atomic int ARC_GenericMutex;

/**
 * Queue lock structure
 * */
struct ARC_QLock {
	/// Synchronization lock for the queue
	ARC_GenericMutex lock;
	bool is_frozen;
	/// Pointer to the current owner of the lock
	void *next;
	/// Pointer to the last element in the queue
	void *last;
};

#define ARC_GENERIC_LOCK(__lock__) \
	while (atomic_flag_test_and_set_explicit(__lock__, memory_order_acquire)) __builtin_ia32_pause();
#define ARC_GENERIC_UNLOCK(__lock__) \
	atomic_flag_clear_explicit(__lock__, memory_order_release)

/**
 * Initialize dynamic qlock
 *
 * Allocate and zero a lock, return it in the
 * given doubly pointer.
 * */
int init_qlock(struct ARC_QLock **lock);

/**
 * Uninitialize dynamic qlock
 *
 * Deallocate the given lock.
 * */
int uninit_qlock(struct ARC_QLock *lock);

/**
 * Initialize static qlock.
 * */
int init_static_qlock(struct ARC_QLock *head);

/**
 * Enqueue calling thread.
 *
 * Enqueue the calling thread into the provided lock.
 *
 * @struct ARC_QLock *lock - The lock into which the calling thread should be enqueued.
 * @return tid of the current thread upon success (>= 0), -1: thread owns lock, -2: failed
 * to enqueue thread.
 * */
int qlock(struct ARC_QLock *lock);

/**
 * Yield current thread to lock owner thread.
 *
 * If the provided tid is not equivalent to
 * lock->next->tid, then the scheduler will be
 * asked to yield the calling thread to the thread
 * which owns the lock.
 * */
void qlock_yield(struct ARC_QLock *lock);

/**
 * Dequeue current lock owner.
 *
 * Dequeues the current lock owner, which should
 * be the caller.
 *
 * @param struct ARC_QLock *lock - Lock from which to dequeue current thread.
 * @return 0: upon succes, 1: upon current lock owner tid and calling thread tid
 * mismatch.
 * */
int qunlock(struct ARC_QLock *lock);

int qlock_freeze(struct ARC_QLock *head);
int qlock_thaw(struct ARC_QLock *head);

int init_mutex(ARC_GenericMutex **mutex);
int uninit_mutex(ARC_GenericMutex *mutex);
int init_static_mutex(ARC_GenericMutex *mutex);
int mutex_lock(ARC_GenericMutex *mutex);
int mutex_unlock(ARC_GenericMutex *mutex);

int init_static_spinlock(ARC_GenericSpinlock *spinlock);
int spinlock_lock(ARC_GenericSpinlock *spinlock);
int spinlock_unlock(ARC_GenericSpinlock *spinlock);

#endif
