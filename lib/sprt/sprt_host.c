/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include "sprt_common.h"
#include "sprt_queue.h"

void sprt_initialize_queues(void *buffer_base, size_t buffer_size)
{
	/* Initialize queue for blocking messages */

	void *blocking_base = buffer_base;
	uint32_t blocking_num = 4U;
	size_t blocking_size = SPRT_QUEUE_HEADER_SIZE +
			       SPRT_QUEUE_ENTRY_MSG_SIZE * blocking_num;

	sprt_queue_init(blocking_base, blocking_num, SPRT_QUEUE_ENTRY_MSG_SIZE);

	/* Initialize queue for non-blocking messages */

	void *non_blocking_base = (void *)((uintptr_t)blocking_base + blocking_size);
	size_t non_blocking_size = buffer_size - blocking_size;
	uint32_t non_blocking_num = (non_blocking_size - SPRT_QUEUE_HEADER_SIZE) /
		SPRT_QUEUE_ENTRY_MSG_SIZE;

	sprt_queue_init(non_blocking_base, non_blocking_num, SPRT_QUEUE_ENTRY_MSG_SIZE);
}

int sprt_push_message(void *buffer_base,
		      const struct sprt_queue_entry_message *message,
		      int queue_num)
{
	struct sprt_queue *q = buffer_base;

	while (queue_num-- > 0) {
		uintptr_t next_addr = (uintptr_t)q + sizeof(struct sprt_queue) +
				      q->entry_num * q->entry_size;
		q = (struct sprt_queue *) next_addr;
	}

	return sprt_queue_push(q, message);
}
