/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <stdint.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>

#include <platform_def.h>

#define SMT_BUFFER_BASE		0x2201f000
#define SMT_BUFFER0_BASE	SMT_BUFFER_BASE
#define SMT_BUFFER1_BASE	(SMT_BUFFER_BASE + 0x200)

static struct scmi_msg_channel scmi_channel[] = {
	[0] = {
		.shm_addr = SMT_BUFFER0_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},
};

struct scmi_msg_channel *plat_scmi_get_channel(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(scmi_channel));

	return &scmi_channel[agent_id];
}

static const char vendor[] = "NXP";
static const char sub_vendor[] = "";

const char *plat_scmi_vendor_name(void)
{
	return vendor;
}

const char *plat_scmi_sub_vendor_name(void)
{
	return sub_vendor;
}

/* Currently supporting Clocks and Reset Domains */
static const uint8_t plat_protocol_list[] = {
	SCMI_PROTOCOL_ID_POWER_DOMAIN,
	SCMI_PROTOCOL_ID_SENSOR,
	0U /* Null termination */
};

size_t plat_scmi_protocol_count(void)
{
	return ARRAY_SIZE(plat_protocol_list) - 1U;
}

const uint8_t *plat_scmi_protocol_list(unsigned int agent_id __unused)
{
	return plat_protocol_list;
}

void imx8ulp_init_scmi_server(void)
{
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(scmi_channel); i++) {
		scmi_smt_init_agent_channel(&scmi_channel[i]);
	}
}
