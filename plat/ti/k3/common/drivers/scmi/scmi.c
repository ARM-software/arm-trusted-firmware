/*
 * Copyright (c) 2024, Texas Instruments - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <plat_scmi_def.h>
#include <clk.h>
#include <clk_wrapper.h>
#include <device.h>
#include <devices.h>
#include <clocks.h>
#include <device_clk.h>
#include <device_pm.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#define MAX_PROTOCOL_IN_LIST            8U

const uint8_t ti_scmi_protocol_table[] = {
	SCMI_PROTOCOL_ID_POWER_DOMAIN,
	SCMI_PROTOCOL_ID_CLOCK,
	0,
};

static const char vendor[] = "Texas";
static const char sub_vendor[] = "Instruments";

static struct scmi_msg_channel scmi_channel[] = {
	[0] = {
		.shm_addr = 0x70800000,
		.shm_size = 0x100,
	},
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
	const uint8_t *protocol_list = ti_scmi_protocol_table;
	while (protocol_list[count])
		count++;

	return count;
}

const uint8_t *plat_scmi_protocol_list(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(ti_scmi_protocol_table));
	return ti_scmi_protocol_table;
}


struct scmi_msg_channel *plat_scmi_get_channel(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(scmi_channel));
	return &scmi_channel[agent_id];
}

void ti_clk_and_dev_init(void)
{
	VERBOSE("%s started!\n", __func__);
	if (clk_init()) {
		WARN("%s: Clock init failed!\n", __func__);
	}

	if (devices_init()) {
		WARN("%s: Devices init failed!\n", __func__);
	}
}

void ti_init_scmi_server(void)
{
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(scmi_channel); i++)
		scmi_smt_init_agent_channel(&scmi_channel[i]);

	ti_clk_and_dev_init();
}

