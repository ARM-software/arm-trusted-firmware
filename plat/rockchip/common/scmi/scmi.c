/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#define MAX_PROTOCOL_IN_LIST		8U

static const char vendor[] = "rockchip";
static const char sub_vendor[] = "";

#pragma weak rockchip_scmi_protocol_table

const uint8_t rockchip_scmi_protocol_table[1][MAX_PROTOCOL_IN_LIST] = {
	{
		SCMI_PROTOCOL_ID_CLOCK,
		SCMI_PROTOCOL_ID_RESET_DOMAIN,
		0
	}
};

const char *plat_scmi_vendor_name(void)
{
	return vendor;
}

const char *plat_scmi_sub_vendor_name(void)
{
	return sub_vendor;
}

size_t plat_scmi_protocol_count(void)
{
	unsigned int count = 0U;
	const uint8_t *protocol_list = rockchip_scmi_protocol_table[0];

	while (protocol_list[count])
		count++;

	return count;
}

const uint8_t *plat_scmi_protocol_list(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(rockchip_scmi_protocol_table));

	return rockchip_scmi_protocol_table[agent_id];
}

static struct scmi_msg_channel scmi_channel[] = {
	[0] = {
		.shm_addr = SMT_BUFFER0_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},

#ifdef SMT_BUFFER1_BASE
	[1] = {
		.shm_addr = SMT_BUFFER1_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},
#endif
};

struct scmi_msg_channel *plat_scmi_get_channel(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(scmi_channel));

	return &scmi_channel[agent_id];
}

#pragma weak rockchip_init_scmi_server

void rockchip_init_scmi_server(void)
{
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(scmi_channel); i++)
		scmi_smt_init_agent_channel(&scmi_channel[i]);
}
