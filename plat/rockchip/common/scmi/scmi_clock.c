/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>

#include "scmi_clock.h"

#pragma weak rockchip_scmi_clock_count
#pragma weak rockchip_scmi_get_clock

size_t rockchip_scmi_clock_count(unsigned int agent_id __unused)
{
	return 0;
}

rk_scmi_clock_t *rockchip_scmi_get_clock(uint32_t agent_id __unused,
					 uint32_t scmi_id __unused)
{
	return NULL;
}

size_t plat_scmi_clock_count(unsigned int agent_id)
{
	return rockchip_scmi_clock_count(agent_id);
}

const char *plat_scmi_clock_get_name(unsigned int agent_id,
				     unsigned int scmi_id)
{
	rk_scmi_clock_t *clock;

	clock = rockchip_scmi_get_clock(agent_id, scmi_id);
	if (clock == NULL)
		return NULL;

	return clock->name;
}

int32_t plat_scmi_clock_rates_array(unsigned int agent_id,
				    unsigned int scmi_id,
				    unsigned long *rates,
				    size_t *nb_elts,
				    uint32_t start_idx)
{
	uint32_t i;
	unsigned long *rate_table;
	rk_scmi_clock_t *clock;

	clock = rockchip_scmi_get_clock(agent_id, scmi_id);
	if (clock == NULL)
		return SCMI_NOT_FOUND;

	rate_table = clock->rate_table;
	if (rate_table == NULL)
		return SCMI_NOT_SUPPORTED;

	if (rates == 0) {
		*nb_elts = clock->rate_cnt;
		goto out;
	}

	if (start_idx + *nb_elts > clock->rate_cnt)
		return SCMI_OUT_OF_RANGE;

	for (i = 0; i < *nb_elts; i++)
		rates[i] = rate_table[start_idx + i];

out:
	return SCMI_SUCCESS;
}

int32_t plat_scmi_clock_rates_by_step(unsigned int agent_id __unused,
				      unsigned int scmi_id __unused,
				      unsigned long *steps __unused)
{
	return SCMI_NOT_SUPPORTED;
}

unsigned long plat_scmi_clock_get_rate(unsigned int agent_id,
				       unsigned int scmi_id)
{
	rk_scmi_clock_t *clock;
	unsigned long rate = 0;

	clock = rockchip_scmi_get_clock(agent_id, scmi_id);
	if (clock == NULL)
		return 0;

	if (clock->clk_ops && clock->clk_ops->get_rate)
		rate = clock->clk_ops->get_rate(clock);

	/* return cur_rate if no get_rate ops or get_rate return 0 */
	if (rate == 0)
		rate = clock->cur_rate;

	return rate;
}

int32_t plat_scmi_clock_set_rate(unsigned int agent_id,
				 unsigned int scmi_id,
				 unsigned long rate)
{
	rk_scmi_clock_t *clock;
	int32_t status = 0;

	clock = rockchip_scmi_get_clock(agent_id, scmi_id);
	if (clock == NULL)
		return SCMI_NOT_FOUND;

	if (clock->clk_ops && clock->clk_ops->set_rate) {
		status = clock->clk_ops->set_rate(clock, rate);
		if (status == SCMI_SUCCESS)
			clock->cur_rate = rate;
	} else {
		status = SCMI_NOT_SUPPORTED;
	}

	return status;
}

int32_t plat_scmi_clock_get_state(unsigned int agent_id,
				  unsigned int scmi_id)
{
	rk_scmi_clock_t *clock;

	clock = rockchip_scmi_get_clock(agent_id, scmi_id);
	if (clock == NULL)
		return 0;

	return clock->enable;
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id,
				  unsigned int scmi_id,
				  bool enable_not_disable)
{
	rk_scmi_clock_t *clock;
	int32_t status = 0;

	clock = rockchip_scmi_get_clock(agent_id, scmi_id);
	if (clock == NULL)
		return SCMI_NOT_FOUND;

	if (clock->clk_ops && clock->clk_ops->set_status) {
		status = clock->clk_ops->set_status(clock, enable_not_disable);
		if (status == SCMI_SUCCESS)
			clock->enable = enable_not_disable;
	} else {
		status = SCMI_NOT_SUPPORTED;
	}

	return status;
}
