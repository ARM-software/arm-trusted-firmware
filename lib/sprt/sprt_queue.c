/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "sprt_queue.h"

void sprt_queue_init(void *queue_base, uint32_t entry_num, uint32_t entry_size)
{
	assert(queue_base != NULL);
	assert(entry_size > 0U);
	assert(entry_num > 0U);

	struct sprt_queue *queue = (struct sprt_queue *)queue_base;

	queue->entry_num = entry_num;
	queue->entry_size = entry_size;
	queue->idx_write = 0U;
	queue->idx_read = 0U;

	memset(queue->data, 0, entry_num * entry_size);
}

int sprt_queue_is_empty(void *queue_base)
{
	assert(queue_base != NULL);

	struct sprt_queue *queue = (struct sprt_queue *)queue_base;

	return (queue->idx_write == queue->idx_read);
}

int sprt_queue_is_full(void *queue_base)
{
	assert(queue_base != NULL);

	struct sprt_queue *queue = (struct sprt_queue *)queue_base;

	uint32_t idx_next_write = (queue->idx_write + 1) % queue->entry_num;

	return (idx_next_write == queue->idx_read);
}

int sprt_queue_push(void *queue_base, const void *entry)
{
	assert(entry != NULL);
	assert(queue_base != NULL);

	if (sprt_queue_is_full(queue_base) != 0) {
		return -ENOMEM;
	}

	struct sprt_queue *queue = (struct sprt_queue *)queue_base;

	uint8_t *dst_entry = &queue->data[queue->entry_size * queue->idx_write];

	memcpy(dst_entry, entry, queue->entry_size);

	/*
	 * Make sure that the message data is visible before increasing the
	 * counter of available messages.
	 */
	__asm__ volatile("dmb st" ::: "memory");

	queue->idx_write = (queue->idx_write + 1) % queue->entry_num;

	__asm__ volatile("dmb st" ::: "memory");

	return 0;
}

int sprt_queue_pop(void *queue_base, void *entry)
{
	assert(entry != NULL);
	assert(queue_base != NULL);

	if (sprt_queue_is_empty(queue_base) != 0) {
		return -ENOENT;
	}

	struct sprt_queue *queue = (struct sprt_queue *)queue_base;

	uint8_t *src_entry = &queue->data[queue->entry_size * queue->idx_read];

	memcpy(entry, src_entry, queue->entry_size);

	/*
	 * Make sure that the message data is visible before increasing the
	 * counter of read messages.
	 */
	__asm__ volatile("dmb st" ::: "memory");

	queue->idx_read = (queue->idx_read + 1) % queue->entry_num;

	__asm__ volatile("dmb st" ::: "memory");

	return 0;
}
