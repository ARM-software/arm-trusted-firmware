/*
 * Copyright (c) 2017-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>

#include "scmi_private.h"

/*
 * Initialize the SCMI system power management protocol.
 */
int scmi_sys_pwr_init(scmi_channel_t *ch)
{
	uint32_t version;
	int ret;

	ret = scmi_proto_version(ch, SCMI_SYS_PWR_PROTO_ID, &version);
	if (ret != SCMI_E_SUCCESS) {
		WARN("SCMI system power protocol version message failed\n");

		return (ret == SCMI_E_NOT_SUPPORTED) ? -EPROTONOSUPPORT :
						       -EPROTO;
	}

	if (!is_scmi_version_compatible(SCMI_SYS_PWR_PROTO_VER, version)) {
		WARN("SCMI system power management protocol version 0x%x incompatible with driver version 0x%x\n",
		     version, SCMI_SYS_PWR_PROTO_VER);

		return -EPROTONOSUPPORT;
	}

	VERBOSE("SCMI system power management protocol version 0x%x detected\n",
		version);

	return 0;
}

/*
 * API to set the SCMI system power state
 */
int scmi_sys_pwr_state_set(void *p, uint32_t flags, uint32_t system_state)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_SYS_PWR_PROTO_ID,
			SCMI_SYS_PWR_STATE_SET_MSG, token);
	mbx_mem->len = SCMI_SYS_PWR_STATE_SET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG2(mbx_mem->payload, flags, system_state);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == SCMI_SYS_PWR_STATE_SET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to get the SCMI system power state
 */
int scmi_sys_pwr_state_get(void *p, uint32_t *system_state)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_SYS_PWR_PROTO_ID,
			SCMI_SYS_PWR_STATE_GET_MSG, token);
	mbx_mem->len = SCMI_SYS_PWR_STATE_GET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL2(mbx_mem->payload, ret, *system_state);
	assert(mbx_mem->len == SCMI_SYS_PWR_STATE_GET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}
