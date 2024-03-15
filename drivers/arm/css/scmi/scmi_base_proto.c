/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>

#include "scmi_private.h"

int scmi_base_protocol_attributes(void *p, uint32_t *num_protocols, uint32_t *num_agents)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0U;
	int ret;
	uint32_t attr;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_BASE_PROTO_ID,
			SCMI_PROTO_ATTR_MSG, token);
	mbx_mem->len = SCMI_PROTO_ATTR_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL2(mbx_mem->payload, ret, attr);
	assert(mbx_mem->len == SCMI_BASE_RESET_AGENT_CONFIGURATION_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	if (ret == SCMI_E_SUCCESS) {
		if (num_protocols) {
			*num_protocols = attr & 0xFF;
		}

		if (num_agents) {
			*num_agents = (attr >> 8) & 0xFF;
		}
	}

	return ret;
}

int scmi_base_discover_agent(void *p, uint32_t agent_id, uint32_t *agent_id_resp, char *name)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0U;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_BASE_PROTO_ID,
			SCMI_BASE_DISCOVER_AGENT, token);
	mbx_mem->len = SCMI_BASE_DISCOVER_AGENT_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG1(mbx_mem->payload, agent_id);

	scmi_send_sync_command(ch);

	/* Get the return values */
	if (agent_id_resp) {
		SCMI_PAYLOAD_RET_VAL2(mbx_mem->payload, ret, *agent_id_resp);
	} else {
		SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	}
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	if (name != NULL) {
		memcpy(name, (void *)&mbx_mem->payload[2],
		       SCMI_BASE_DISCOVER_AGENT_RESP_LEN - 8);
	}

	scmi_put_channel(ch);

	return ret;
}

int scmi_base_reset_agent_config(void *p, uint32_t agent_id, uint32_t flags)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0U;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_BASE_PROTO_ID,
			SCMI_BASE_RESET_AGENT_CONFIGURATION, token);
	mbx_mem->len = SCMI_BASE_RESET_AGENT_CONFIGURATION_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG2(mbx_mem->payload, agent_id, flags);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == SCMI_BASE_RESET_AGENT_CONFIGURATION_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}
