/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ti_sci.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <plat_scmi_def.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include <ti_device_handler.h>
#include <ti_device.h>
#include <ti_devices.h>
#include <ti_clocks.h>
#include <ti_device_clk.h>
#include <ti_device_pm.h>
#include <ti_clk.h>

#include <scmi_pd_data.h>

#define POWER_STATE_ON  (0 << 30)
#define POWER_STATE_OFF BIT(30)

size_t plat_scmi_pd_count(unsigned int agent_id __unused)
{
	return ARRAY_SIZE(scmi_power_domains);
}

const char *plat_scmi_pd_get_name(unsigned int agent_id __unused,
				  unsigned int pd_id)
{
	return scmi_power_domains[pd_id].name;
}

unsigned int plat_scmi_pd_get_state(unsigned int agent_id __unused,
				    unsigned int pd_id __unused)
{
	bool is_on = get_device_handler(scmi_power_domains[pd_id].id);
	return is_on ? POWER_STATE_ON : POWER_STATE_OFF;
}

int32_t plat_scmi_pd_set_state(unsigned int agent_id __unused,
			       unsigned int flags,
			       unsigned int pd_id,
			       unsigned int state)
{
	int ret = SCMI_SUCCESS;

	bool current_state = get_device_handler(scmi_power_domains[pd_id].id);
	unsigned int current_power_state = current_state ? POWER_STATE_ON : POWER_STATE_OFF;
	/*
	 * First, check if the device state even needs to be changed, otherwise do nothing and
	 * return SCMI_SUCCESS
	 */
	if (current_power_state == POWER_STATE_ON && state == POWER_STATE_OFF) {
		VERBOSE("\n%s: Disabling PD: agent_id = %d, pd = %d, state to set = 0x%x\n",
			__func__, agent_id, pd_id, state);
		ret = set_device_handler(scmi_power_domains[pd_id].id, false);
	} else if (current_power_state == POWER_STATE_OFF && state == POWER_STATE_ON) {
		VERBOSE("\n%s: Enabling PD: agent_id = %d, pd = %d, state to set = 0x%x\n",
			__func__, agent_id, pd_id, state);
		ret = set_device_handler(scmi_power_domains[pd_id].id, true);
	} else {
		ret = SCMI_SUCCESS;
	}

	return ret;
}

