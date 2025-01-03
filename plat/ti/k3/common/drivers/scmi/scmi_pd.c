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

#include <clk.h>
#include <device_wrapper.h>
#include <device.h>
#include <devices.h>
#include <clocks.h>
#include <device_clk.h>
#include <device_pm.h>

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
	return scmi_handler_device_state_get(scmi_power_domains[pd_id].id);
}

int32_t plat_scmi_pd_set_state(unsigned int agent_id __unused,
			       unsigned int flags,
			       unsigned int pd_id,
			       unsigned int state)
{
	int ret = SCMI_SUCCESS;

	ret = scmi_handler_device_state_get(scmi_power_domains[pd_id].id);
	/*
	 * First, check if the device state even needs to be changed, otherwise do nothing and
	 * return SCMI_SUCCESS
	 */
	if (ret == POWER_STATE_ON && state == POWER_STATE_OFF) {
		VERBOSE("\n%s: Disabling PD: agent_id = %d, pd = %d, state to set = 0x%x\n",
			__func__, agent_id, pd_id, state);
		ret = scmi_handler_device_state_set_off(scmi_power_domains[pd_id].id);
	} else if (ret == POWER_STATE_OFF && state == POWER_STATE_ON) {
		VERBOSE("\n%s: Enabling PD: agent_id = %d, pd = %d, state to set = 0x%x\n",
			__func__, agent_id, pd_id, state);
		ret = scmi_handler_device_state_set_on(scmi_power_domains[pd_id].id);
	} else {
		ret = SCMI_SUCCESS;
	}

	return ret;
}

