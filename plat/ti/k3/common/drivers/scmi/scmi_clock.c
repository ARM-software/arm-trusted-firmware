#include <ti_sci.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <plat_scmi_def.h>
#include "scmi_clock.h"

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include <clk_wrapper.h>
#include <device.h>
#include <devices.h>
#include <clocks.h>
#include <device_clk.h>

#include <scmi_clk_data.h>

ti_scmi_clock_t *ti_scmi_get_clock(uint32_t agent_id __unused,
				   uint32_t clock_id)
{
	ti_scmi_clock_t *table = NULL;

	if (clock_id < ARRAY_SIZE(clock_table))
		table = &clock_table[clock_id];

	return table;
}

size_t plat_scmi_clock_count(unsigned int agent_id)
{
	return ARRAY_SIZE(clock_table);
}

int32_t plat_scmi_clock_set_rate(unsigned int agent_id,
				 unsigned int scmi_id,
				 unsigned long rate)
{
	VERBOSE("scmi_clock_set_rate scmi_id = %d rate = %lu\n", scmi_id, rate);
	ti_scmi_clock_t *clock;
	int32_t ret;

	clock = ti_scmi_get_clock(agent_id, scmi_id);
	if (clock == 0)
		return 0;

	ret = scmi_handler_clock_set_rate(clock->dev_id, clock->clock_id, rate);
	if (ret) {
		WARN("%s: Failed to set freq with scmi_id = %d rate = %ld\n",
		     __func__, scmi_id, rate);
		return SCMI_DENIED;
	}

	return SCMI_SUCCESS;

}

const char *plat_scmi_clock_get_name(unsigned int agent_id,
				     unsigned int scmi_id)
{
	ti_scmi_clock_t *clock;

	clock = ti_scmi_get_clock(agent_id, scmi_id);
	if (clock)
		return clock->name;

	return NULL;
}

int32_t plat_scmi_clock_rates_by_step(unsigned int agent_id __unused,
				      unsigned int scmi_id __unused,
				      unsigned long *steps __unused)
{

	ti_scmi_clock_t *clock;

	clock = ti_scmi_get_clock(agent_id, scmi_id);
	if (!clock)
		return SCMI_NOT_SUPPORTED;

	steps[0] = clock->rates[0];
	steps[1] = clock->rates[1];
	steps[2] = clock->rates[2];

	return SCMI_SUCCESS;
}

unsigned long plat_scmi_clock_get_rate(unsigned int agent_id,
				       unsigned int scmi_id)
{
	ti_scmi_clock_t *clock;
	uint64_t rate;

	clock = ti_scmi_get_clock(agent_id, scmi_id);
	if (!clock)
		return SCMI_NOT_SUPPORTED;

	rate = scmi_handler_clock_get_rate(clock->dev_id, clock->clock_id);
	return rate;
}

int32_t plat_scmi_clock_get_state(unsigned int agent_id,
				  unsigned int scmi_id)
{
	ti_scmi_clock_t *clock;

	clock = ti_scmi_get_clock(agent_id, scmi_id);
	if (!clock)
		return SCMI_NOT_SUPPORTED;

	/*
	 * This function is broken, send a fix upstream, create a return value
	 * for status and put the state in a separate variable pointer
	 */

	/*
	 * Also, PM framework can return unreq and auto state, linux expect req and auto state,
	 * for auto and req state, return value is always enabled
	 */
	return !!scmi_handler_clock_get_state(clock->dev_id, clock->clock_id);
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id,
				  unsigned int scmi_id,
				  bool enable_not_disable)
{
	ti_scmi_clock_t *clock;

	clock = ti_scmi_get_clock(agent_id, scmi_id);
	if (!clock)
		return SCMI_NOT_SUPPORTED;

	VERBOSE("%s: agent_id = %d, scmi_id = %d, enable: %d\n",
		__func__, agent_id, scmi_id, enable_not_disable);
	if (enable_not_disable) {
		return scmi_handler_clock_prepare(clock->dev_id, clock->clock_id);
	} else {
		return scmi_handler_clock_unprepare(clock->dev_id, clock->clock_id);
	}

	return SCMI_INVALID_PARAMETERS;
}

int32_t plat_scmi_clock_get_possible_parents(unsigned int agent_id,
					     unsigned int scmi_id,
					     unsigned int *plat_possible_parents,
					     size_t *nb_elts,
					     uint32_t skip_parents)
{
	VERBOSE("scmi_clock_get_possible_parents agent_id = %d, scmi_id = %d\n", agent_id, scmi_id);
	ti_scmi_clock_t *clock;

	clock = ti_scmi_get_clock(agent_id, scmi_id);
	if (clock == 0)
		return SCMI_NOT_FOUND;

	*nb_elts = (uint64_t)scmi_handler_clock_get_num_clock_parents(clock->dev_id,
									clock->clock_id);
	if (plat_possible_parents) {
		for (uint32_t i = 0; i < (uint32_t)*nb_elts ; i++) {
			plat_possible_parents[i] = i;
		}
	}
	VERBOSE("num_parents %d\n", (uint32_t)*nb_elts);
	return SCMI_SUCCESS;
}

int32_t plat_scmi_clock_get_parent(unsigned int agent_id,
				   unsigned int scmi_id)
{
	ti_scmi_clock_t *clock;
	uint32_t parent_id = 0;
	int32_t status = 0;

	VERBOSE("scmi_clock_get_parent agent_id = %d, scmi_id = %d\n", agent_id, scmi_id);
	clock = ti_scmi_get_clock(agent_id, scmi_id);
	if (clock == 0)
		return SCMI_NOT_FOUND;

	status = scmi_handler_clock_get_clock_parent(clock->dev_id, clock->clock_id, &parent_id);
	if (status)
		return SCMI_GENERIC_ERROR;

	parent_id = parent_id - clock->clock_id - 1;

	VERBOSE("scmi_clock_get_parent parent_id = %d\n", parent_id);
	return parent_id;
}

int32_t plat_scmi_clock_set_parent(unsigned int agent_id,
				   unsigned int scmi_id,
				   unsigned int parent_id)
{
	ti_scmi_clock_t *clock;
	int32_t status = 0;

	VERBOSE("%s: agent_id = %d, scmi_id = %d parent_id = %d\n", __func__,
		agent_id, scmi_id, parent_id);
	clock = ti_scmi_get_clock(agent_id, scmi_id);
	if (clock == 0)
		return SCMI_NOT_FOUND;

	parent_id = parent_id + clock->clock_id + 1;
	status = scmi_handler_clock_set_clock_parent(clock->dev_id, clock->clock_id, parent_id);
	if (status)
		return SCMI_GENERIC_ERROR;

	return SCMI_SUCCESS;
}
