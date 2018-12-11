/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPRT_COMMON_H
#define SPRT_COMMON_H

#define SPRT_MAX_MSG_ARGS	6

/*
 * Message types supported.
 */
#define SPRT_MSG_TYPE_SERVICE_HANDLE_OPEN		1
#define SPRT_MSG_TYPE_SERVICE_HANDLE_CLOSE		2
/* TODO: Add other types of SPRT messages. */
#define SPRT_MSG_TYPE_SERVICE_TUN_REQUEST		10

/*
 * Struct that defines the layout of the fields corresponding to a request in
 * shared memory.
 */
struct __attribute__((__packed__)) sprt_queue_entry_message {
	uint32_t type;		/* Type of message (result of an SPCI call). */
	uint16_t client_id;	/* SPCI client ID */
	uint16_t service_handle;/* SPCI service handle */
	uint32_t session_id;	/* Optional SPCI session ID */
	uint32_t token;		/* SPCI request token */
	uint64_t args[SPRT_MAX_MSG_ARGS];
};

#define SPRT_QUEUE_ENTRY_MSG_SIZE	(sizeof(struct sprt_queue_entry_message))

#define SPRT_QUEUE_NUM_BLOCKING		0
#define SPRT_QUEUE_NUM_NON_BLOCKING	1

#endif /* SPRT_COMMON_H */
