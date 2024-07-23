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
#include <mm/pmm.h>
#include <mm/allocator.h>
#include <global.h>
#include <stdbool.h>
#include <lib/util.h>

struct buddy_node {
	struct buddy_node *left;
	struct buddy_node *right;
	struct buddy_node *parent;
	void *base;
	size_t size;
	int exponent;
	uint8_t in_use : 1;    // Set if a sub-tree is allocated, but this node
	uint8_t allocated : 1; // Set if this node is allocated, and cannot be
};

static int quick_log2(size_t number, size_t *estimate, bool round_up) {
	size_t temp = 0;

	if (estimate == NULL) {
		estimate = &temp;
	}

	int exp = 0;
	*estimate = 1;
	size_t original = number;

	while (number > 1) {
		exp++;
		*estimate <<= 1;
		number >>= 1;
	}

	if (round_up && *estimate < original) {
		// 2^exp < number, caller wants 2^exp
		// to fit number
		exp++;
	} else if (!round_up && *estimate > original) {
		// 2^exp > number, caller wants 2^exp to
		// fit into number
		exp--;
	}

	return exp;
}

static int split(struct ARC_BuddyMeta *meta, struct buddy_node *node) {
	if (node == NULL || node->exponent < 0) {
		return -1;
	}

	if (meta->lowest_exponent == node->exponent) {
		return -2;
	}

	int level = abs(node->exponent) - 1;

	if (level < meta->lowest_exponent) {
		return -3;
	}

	struct buddy_node *new = (struct buddy_node *)alloc(sizeof(struct buddy_node));

	if (new == NULL) {
		return -4;
	}

	memset(new, 0, sizeof(struct buddy_node));

	new->exponent = level;
	new->base = node->base;
	new->size = node->size / 2;
	new->parent = node;
	node->left = new;

	new = (struct buddy_node *)alloc(sizeof(struct buddy_node));

	if (new == NULL) {
		return -4;
	}

	memset(new, 0, sizeof(struct buddy_node));

	new->exponent = level;
	new->base = node->base + (node->size / 2);
	new->size = node->size / 2;
	new->parent = node;
	node->right = new;

	return 0;
}

void *buddy_alloc(struct ARC_BuddyMeta *meta, size_t size) {
	if (meta == NULL || size == 0 || meta->tree == NULL) {
		return NULL;
	}

	int exponent = quick_log2(size, NULL, 1);

	struct buddy_node *current = (struct buddy_node *)meta->tree;
	struct buddy_node *stack[abs(current->exponent)];

	stack[0] = current->right;

	int stack_pointer = 1;

	mutex_lock(&meta->mutex);

	while (current != NULL) {
		if (current->allocated) {
			current = stack[--stack_pointer];
			continue;
		}

		// NOTE: If current->left is NULL, it can be assumed that current->right is NULL
		if (current->left == NULL && current->exponent != exponent && split(meta, current) != 0) {
			// Node does not have sub-nodes, is not on the target exponent, and failed to split
			mutex_unlock(&meta->mutex);
			return NULL;
		}

		// Not on the parent level, push right node
		// and continue to the left
		if (current->exponent != exponent + 1) {
			stack[stack_pointer++] = current->right;
			current = current->left;
			continue;
		}

		bool left = current->left->allocated | current->left->in_use;
		bool right = current->right->allocated | current->right->in_use;

		// No available nodes here, go back to the right
		if (left == 1 && right == 1) {
			current = stack[--stack_pointer];
			continue;
		}

		// Allocate
		if (left == 1) {
			current = current->right;
			break;
		} else {
			current = current->left;
			break;
		}
	}

	if (current == NULL) {
		mutex_unlock(&meta->mutex);
		return NULL;
	}

	void *address = current->base;

	current->allocated = 1;

	do {
		current = current->parent;

		if (current != NULL) {
			current->in_use = 1;
		}
	} while (current != NULL);

	mutex_unlock(&meta->mutex);

        return address;
}

void *buddy_free(struct ARC_BuddyMeta *meta, void *address) {
	if (meta == NULL || meta->tree == NULL) {
		return NULL;
	}

	mutex_lock(&meta->mutex);

	struct buddy_node *current = (struct buddy_node *)meta->tree;

	while (current != NULL && !current->allocated) {
		if (address >= (void *)(current->base + current->size / 2)) {
			current = current->right;
			continue;
		}

		current = current->left;
	}

	if (current == NULL) {
		mutex_unlock(&meta->mutex);
		return NULL;
	}

	current->allocated = 0;

	do {
		current = current->parent;

		if (current == NULL) {
			break;
		}

		bool left = current->left == NULL ? 0 : (current->left->allocated | current->left->in_use);
		bool right = current->right == NULL ? 0 : (current->right->allocated | current->right->in_use);

		current->in_use = left | right;
	} while (current != NULL);

	mutex_unlock(&meta->mutex);

        return address;
}

int init_buddy(struct ARC_BuddyMeta *meta, void *base, size_t size, int lowest_exponent) {
        ARC_DEBUG(INFO, "Initializing new buddy allocator (%lu bytes, lowest 2^%d bytes) at %p\n", size, lowest_exponent, base);

	// Setup meta
	meta->base = base;
	meta->ceil = base + size;
	meta->lowest_exponent = lowest_exponent;
	meta->next = NULL;
	init_static_mutex(&meta->mutex);

	// Approximate log2(size)
	int max_exponent = quick_log2(size, &size, 0);
	ARC_DEBUG(INFO, "\tExponent range: [%d, %d]\n", lowest_exponent, max_exponent);
	ARC_DEBUG(INFO, "\tNew size: %lu\n", size);

	// Setup top node
	struct buddy_node *head = (struct buddy_node *)alloc(sizeof(struct buddy_node));

	if (head == NULL) {
		return -1;
	}

	memset(head, 0, sizeof(struct buddy_node));

	head->exponent = max_exponent;
	head->base = base;
	head->size = size;
	meta->tree = (void *)head;

        return 0;
}
