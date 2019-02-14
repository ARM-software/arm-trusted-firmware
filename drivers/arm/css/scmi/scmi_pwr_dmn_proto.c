/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>

#include "scmi_private.h"

/*
 * API to set the SCMI power domain power state.
 */
int scmi_pwr_state_set(void *p, uint32_t domain_id,
					uint32_t scmi_pwr_state)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;

	/*
	 * Only asynchronous mode of `set power state` command is allowed on
	 * application processors.
	 */
	uint32_t pwr_state_set_msg_flag = SCMI_PWR_STATE_SET_FLAG_ASYNC;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_PWR_DMN_PROTO_ID,
			SCMI_PWR_STATE_SET_MSG, token);
	mbx_mem->len = SCMI_PWR_STATE_SET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG3(mbx_mem->payload, pwr_state_set_msg_flag,
						domain_id, scmi_pwr_state);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == SCMI_PWR_STATE_SET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to get the SCMI power domain power state.
 */
int scmi_pwr_state_get(void *p, uint32_t domain_id,
					uint32_t *scmi_pwr_state)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_PWR_DMN_PROTO_ID,
			SCMI_PWR_STATE_GET_MSG, token);
	mbx_mem->len = SCMI_PWR_STATE_GET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG1(mbx_mem->payload, domain_id);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL2(mbx_mem->payload, ret, *scmi_pwr_state);
	assert(mbx_mem->len == SCMI_PWR_STATE_GET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}
