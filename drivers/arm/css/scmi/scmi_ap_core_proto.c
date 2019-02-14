/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>

#include "scmi_private.h"

/*
 * API to set the SCMI AP core reset address and attributes
 */
int scmi_ap_core_set_reset_addr(void *p, uint64_t reset_addr, uint32_t attr)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_AP_CORE_PROTO_ID,
			SCMI_AP_CORE_RESET_ADDR_SET_MSG, token);
	mbx_mem->len = SCMI_AP_CORE_RESET_ADDR_SET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG3(mbx_mem->payload, reset_addr & 0xffffffff,
		reset_addr >> 32, attr);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == SCMI_AP_CORE_RESET_ADDR_SET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to get the SCMI AP core reset address and attributes
 */
int scmi_ap_core_get_reset_addr(void *p, uint64_t *reset_addr, uint32_t *attr)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;
	uint32_t lo_addr, hi_addr;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_AP_CORE_PROTO_ID,
			SCMI_AP_CORE_RESET_ADDR_GET_MSG, token);
	mbx_mem->len = SCMI_AP_CORE_RESET_ADDR_GET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL4(mbx_mem->payload, ret, lo_addr, hi_addr, *attr);
	*reset_addr = lo_addr | (uint64_t)hi_addr << 32;
	assert(mbx_mem->len == SCMI_AP_CORE_RESET_ADDR_GET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}
