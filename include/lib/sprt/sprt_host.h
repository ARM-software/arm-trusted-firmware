/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SPRT_HOST_H
#define SPRT_HOST_H

#include <stddef.h>

#include "sprt_common.h"

/*
 * Initialize the specified buffer to be used by SPM.
 */
void sprt_initialize_queues(void *buffer_base, size_t buffer_size);

/*
 * Push a message to the queue number `queue_num` in a buffer that has been
 * initialized by `sprt_initialize_queues`.
 */
int sprt_push_message(void *buffer_base,
		      const struct sprt_queue_entry_message *message,
		      int queue_num);

#endif /* SPRT_HOST_H */
