// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2020, Linaro Limited
 */

#include <assert.h>

#include <drivers/st/scmi-msg.h>
#include <drivers/st/scmi.h>

#include "common.h"

void scmi_status_response(struct scmi_msg *msg, int32_t status)
{
	assert(msg->out && msg->out_size >= sizeof(int32_t));

	memcpy(msg->out, &status, sizeof(int32_t));
	msg->out_size_out = sizeof(int32_t);
}

void scmi_write_response(struct scmi_msg *msg, void *payload, size_t size)
{
	/*
	 * Output payload shall be at least the size of the status
	 * Output buffer shall be at least be the size of the status
	 * Output paylaod shall fit in output buffer
	 */
	assert(payload && size >= sizeof(int32_t) && size <= msg->out_size &&
	       msg->out && msg->out_size >= sizeof(int32_t));

	memcpy(msg->out, payload, size);
	msg->out_size_out = size;
}

void scmi_process_message(struct scmi_msg *msg)
{
	scmi_msg_handler_t handler = NULL;

	switch (msg->protocol_id) {
	case SCMI_PROTOCOL_ID_BASE:
		handler = scmi_msg_get_base_handler(msg);
		break;
	case SCMI_PROTOCOL_ID_CLOCK:
		handler = scmi_msg_get_clock_handler(msg);
		break;
	case SCMI_PROTOCOL_ID_RESET_DOMAIN:
		handler = scmi_msg_get_rstd_handler(msg);
		break;
	default:
		break;
	}

	if (handler) {
		handler(msg);
		return;
	}

	ERROR("Agent %u Protocol 0x%x Message 0x%x: not supported",
	      msg->agent_id, msg->protocol_id, msg->message_id);

	scmi_status_response(msg, SCMI_NOT_SUPPORTED);
}
