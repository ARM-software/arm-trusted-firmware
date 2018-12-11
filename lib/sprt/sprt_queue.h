/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPRT_QUEUE_H
#define SPRT_QUEUE_H

#include <stdint.h>

/* Struct that defines a queue. Not to be used directly. */
struct __attribute__((__packed__)) sprt_queue {
	uint32_t entry_num;	/* Number of entries */
	uint32_t entry_size;	/* Size of an entry */
	uint32_t idx_write;	/* Index of first empty entry */
	uint32_t idx_read;	/* Index of first entry to read */
	uint8_t  data[0];	/* Start of data */
};

#define SPRT_QUEUE_HEADER_SIZE	(sizeof(struct sprt_queue))

/*
 * Initializes a memory region to be used as a queue of the given number of
 * entries with the specified size.
 */
void sprt_queue_init(void *queue_base, uint32_t entry_num, uint32_t entry_size);

/* Returns 1 if the queue is empty, 0 otherwise */
int sprt_queue_is_empty(void *queue_base);

/* Returns 1 if the queue is full, 0 otherwise */
int sprt_queue_is_full(void *queue_base);

/*
 * Pushes a new entry intro the queue. Returns 0 on success, -ENOMEM if the
 * queue is full.
 */
int sprt_queue_push(void *queue_base, const void *entry);

/*
 * Pops an entry from the queue. Returns 0 on success, -ENOENT if the queue is
 * empty.
 */
int sprt_queue_pop(void *queue_base, void *entry);

#endif /* SPRT_QUEUE_H */
