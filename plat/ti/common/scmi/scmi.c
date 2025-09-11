/*
 * Copyright (c) 2025, Texas Instruments - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include <ti_device.h>
#include <ti_devices.h>
#include <ti_clocks.h>
#include <ti_device_clk.h>
#include <ti_device_pm.h>
#include <plat_scmi_def.h>
#include <platform_def.h>
#include <ti_clk.h>

#define MAX_PROTOCOL_IN_LIST            8U

const uint8_t ti_scmi_protocol_table[] = {
	SCMI_PROTOCOL_ID_POWER_DOMAIN,
	SCMI_PROTOCOL_ID_CLOCK,
	0,
};

static const char vendor_name[] = "Texas";
static const char sub_vendor_name[] = "Instruments";

static struct scmi_msg_channel ti_scmi_channel[TI_SCMI_CHANNELS] = {
	[0] = {
		.shm_addr = SCMI_SHMEM_ADDR,
		.shm_size = SCMI_SHMEM_SIZE,
	},
};

const char *plat_scmi_vendor_name(void)
{
	return vendor_name;
}

const char *plat_scmi_sub_vendor_name(void)
{
	return sub_vendor_name;
}

size_t plat_scmi_protocol_count(void)
{
	unsigned int count = 0U;
	const uint8_t *protocol_list = ti_scmi_protocol_table;

	while (protocol_list[count] != 0)
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
	assert(agent_id < TI_SCMI_CHANNELS);
	return &ti_scmi_channel[agent_id];
}

void ti_clk_and_dev_init(void)
{
	VERBOSE("%s started!\n", __func__);
	if (ti_clk_init()) {
		WARN("%s: Clock init failed!\n", __func__);
	}

	if (ti_devices_init()) {
		WARN("%s: Devices init failed!\n", __func__);
	}
}

void ti_init_scmi_server(void)
{
	size_t i;

	for (i = 0U; i < TI_SCMI_CHANNELS; i++)
		scmi_smt_init_agent_channel(&ti_scmi_channel[i]);
	ti_clk_and_dev_init();
}

