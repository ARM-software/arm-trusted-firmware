/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/libc/errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../../../drivers/scmi-msg/sensor.h"

#include <common/debug.h>
#include <drivers/scmi.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <scmi.h>

#include <upower_api.h>

/* Only Temperature now */
static uint16_t imx_scmi_sensor_count(unsigned int agent_id __unused)
{
	return 1U;
}

uint8_t imx_scmi_sensor_max_requests(unsigned int agent_id __unused)
{
	return 1U;
}

extern int upower_read_temperature(uint32_t sensor_id, int32_t *temperature);
int imx_scmi_sensor_reading_get(uint32_t agent_id __unused, uint16_t sensor_id __unused,
				 uint32_t *val)
{
	int32_t temperature;
	int ret;

	ret = upower_read_temperature(1, &temperature);
	if (ret != 0U) {
		val[0] = 0xFFFFFFFF;
	} else {
		val[0] = temperature;
	}

	val[1] = 0;
	val[2] = 0;
	val[3] = 0;

	return ret;
}

#define SCMI_SENSOR_NAME_LENGTH_MAX	16U

uint32_t imx_scmi_sensor_state(uint32_t agent_id __unused, uint16_t sensor_id __unused)
{
	return 1U;
}

uint32_t imx_scmi_sensor_description_get(uint32_t agent_id __unused, uint16_t desc_index __unused,
					  struct scmi_sensor_desc *desc __unused)
{
	desc->id = 0;
	desc->attr_low = 0;
	desc->attr_high = 2;
	strlcpy((char *)desc->name, "UPOWER-TEMP", 12);
	desc->power = 0;
	desc->resolution = 0;
	desc->min_range_low = 0;
	desc->min_range_high = 0x80000000;
	desc->max_range_low = 0xffffffff;
	desc->max_range_high = 0x7fffffff;

	return 1U;
}

REGISTER_SCMI_SENSOR_OPS(imx_scmi_sensor_count,
			 imx_scmi_sensor_max_requests,
			 NULL,
			 imx_scmi_sensor_reading_get,
			 imx_scmi_sensor_description_get,
			 NULL,
			 imx_scmi_sensor_state,
			 NULL);
