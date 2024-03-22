/*
 * Copyright (c) 2019-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPI3_MBOX_H
#define RPI3_MBOX_H

#include <stdint.h>

/* This struct must be aligned to 16 bytes */
typedef struct __packed __aligned(16) rpi3_mbox_request {
	uint32_t	size; /* Buffer size in bytes */
	uint32_t	code; /* Request/response code */
	uint32_t	tags[0];
} rpi3_mbox_request_t;

/* VideoCore -> ARM */
#define RPI3_MBOX0_READ_OFFSET		ULL(0x00000000)
#define RPI3_MBOX0_PEEK_OFFSET		ULL(0x00000010)
#define RPI3_MBOX0_SENDER_OFFSET	ULL(0x00000014)
#define RPI3_MBOX0_STATUS_OFFSET	ULL(0x00000018)
#define RPI3_MBOX0_CONFIG_OFFSET	ULL(0x0000001C)
/* ARM -> VideoCore */
#define RPI3_MBOX1_WRITE_OFFSET		ULL(0x00000020)
#define RPI3_MBOX1_PEEK_OFFSET		ULL(0x00000030)
#define RPI3_MBOX1_SENDER_OFFSET	ULL(0x00000034)
#define RPI3_MBOX1_STATUS_OFFSET	ULL(0x00000038)
#define RPI3_MBOX1_CONFIG_OFFSET	ULL(0x0000003C)
/* Mailbox status constants */
#define RPI3_MBOX_STATUS_FULL_MASK	U(0x80000000) /* Set if full */
#define RPI3_MBOX_STATUS_EMPTY_MASK	U(0x40000000) /* Set if empty */

#define RPI3_MBOX_BUFFER_SIZE		U(256)

/* Constants to perform a request/check the status of a request. */
#define RPI3_MBOX_PROCESS_REQUEST	U(0x00000000)
#define RPI3_MBOX_REQUEST_SUCCESSFUL	U(0x80000000)
#define RPI3_MBOX_REQUEST_ERROR		U(0x80000001)

/* Command constants */
#define RPI3_TAG_HARDWARE_GET_BOARD_REVISION	U(0x00010002)
#define RPI3_TAG_END				U(0x00000000)

#define RPI3_TAG_REQUEST		U(0x00000000)
#define RPI3_TAG_IS_RESPONSE		U(0x80000000) /* Set if response */
#define RPI3_TAG_RESPONSE_LENGTH_MASK	U(0x7FFFFFFF)

#define RPI3_CHANNEL_ARM_TO_VC		U(0x8)
#define RPI3_CHANNEL_MASK		U(0xF)

void rpi3_vc_mailbox_request_send(rpi3_mbox_request_t *req, int req_size);

#endif
