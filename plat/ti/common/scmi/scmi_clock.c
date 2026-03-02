#include <ti_sci.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <plat_scmi_def.h>
#include <scmi_clock.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <drivers/clk.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include <ti_clk_handler.h>
#include <ti_device.h>
#include <ti_devices.h>
#include <ti_clocks.h>
#include <ti_device_clk.h>

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
	unsigned long orate;
	int32_t ret;

	if (scmi_id >= ARRAY_SIZE(clock_table))
		return SCMI_NOT_FOUND;

	ret = clk_set_rate(scmi_id, rate, &orate);
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
	if (scmi_id >= ARRAY_SIZE(clock_table))
		return 0;

	return clk_get_rate(scmi_id);
}

int32_t plat_scmi_clock_get_state(unsigned int agent_id,
				  unsigned int scmi_id)
{
	if (scmi_id >= ARRAY_SIZE(clock_table))
		return SCMI_NOT_SUPPORTED;

	/*
	 * This function is broken, send a fix upstream, create a return value
	 * for status and put the state in a separate variable pointer
	 */

	/*
	 * Also, PM framework can return unreq and auto state, linux expect req and auto state,
	 * for auto and req state, return value is always enabled
	 */
	return clk_is_enabled(scmi_id) ? 1 : 0;
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id,
				  unsigned int scmi_id,
				  bool enable)
{
	int ret;

	if (scmi_id >= ARRAY_SIZE(clock_table))
		return SCMI_NOT_SUPPORTED;

	VERBOSE("%s: agent_id = %d, scmi_id = %d, enable: %d\n",
		__func__, agent_id, scmi_id, enable);

	if (enable) {
		ret = clk_enable(scmi_id);
	} else {
		clk_disable(scmi_id);
		ret = 0;
	}

	return ret;
}

int32_t plat_scmi_clock_get_possible_parents(unsigned int agent_id,
					     unsigned int scmi_id,
					     unsigned int *plat_possible_parents,
					     size_t *nb_elts,
					     uint32_t skip_parents)
{
	VERBOSE("scmi_clock_get_possible_parents agent_id = %d, scmi_id = %d\n", agent_id, scmi_id);
	int encoded_parents;
	uint32_t mux_dev_id;
	size_t total_parents, reserved_parents, non_reserved_parents;
	size_t parent_idx = 0;
	size_t reserved_found = 0;

	if (scmi_id >= ARRAY_SIZE(clock_table))
		return SCMI_NOT_FOUND;

	/* Get encoded parent info: bits 0-7=total, bits 8-15=reserved */
	encoded_parents = clk_get_possible_parents_num(scmi_id);
	total_parents = (size_t)(encoded_parents & 0xFF);
	reserved_parents = (size_t)((encoded_parents >> 8) & 0xFF);
	non_reserved_parents = total_parents - reserved_parents;

	*nb_elts = (total_parents > 1) ? total_parents : 0;

	if (plat_possible_parents) {
		/*
		 * It doesn't make sense for a mux to have just 1 parent,
		 * so check for minimum 2
		 */
		if (*nb_elts > 1) {
			mux_dev_id = clock_table[scmi_id].dev_id;

			/*
			 * Get non-reserved parents from contiguous entries
			 * before the MUX clock.
			 */
			for (size_t i = 0; i < non_reserved_parents; i++) {
				plat_possible_parents[parent_idx] =
					scmi_id - non_reserved_parents + i;
				VERBOSE("%s: non-reserved parent[%zu] = %u\n",
					__func__, parent_idx, plat_possible_parents[parent_idx]);
				parent_idx++;
			}

			/*
			 * Get reserved parents from bottom of clock_table.
			 */
			if (reserved_parents > 0) {
				for (int i = ARRAY_SIZE(clock_table) - 1;
				     i >= scmi_id && reserved_found < reserved_parents; i--) {
					if (clock_table[i].dev_id == mux_dev_id &&
					    (unsigned int)i != scmi_id) {
						plat_possible_parents[parent_idx] = i;
						VERBOSE("%s: reserved parent[%zu] = %d (dev=%u)\n",
							__func__, parent_idx, i,
							clock_table[i].dev_id);
						parent_idx++;
						reserved_found++;
					}
				}
			}
		} else {
			*nb_elts = 0;	// make it 0 for even a single parent
			VERBOSE("%s: 0 possible parents", __func__);
		}
	} else {
		*nb_elts = (*nb_elts > 1) ? *nb_elts : 0; // make it 0 for just a single parent
		VERBOSE("%s: plat_possible_parents is NULL, *nb_elts = %zu\n", __func__, *nb_elts);
	}
	VERBOSE("num_parents %zu (non-reserved=%zu, reserved=%zu)\n",
		*nb_elts, non_reserved_parents, reserved_parents);
	return SCMI_SUCCESS;
}

int32_t plat_scmi_clock_get_parent(unsigned int agent_id,
				   unsigned int scmi_id,
				   unsigned int *parent_id)
{
	int parent_scmi_id;

	VERBOSE("scmi_clock_get_parent agent_id = %d, scmi_id = %d\n", agent_id, scmi_id);

	if (scmi_id >= ARRAY_SIZE(clock_table))
		return SCMI_NOT_FOUND;

	parent_scmi_id = clk_get_parent(scmi_id);
	if (parent_scmi_id < 0) {
		ERROR("Failed to get clock parent for scmi_id %d\n", scmi_id);
		return SCMI_GENERIC_ERROR;
	}

	VERBOSE("scmi_clock_get_parent parent_id = %d\n", parent_scmi_id);
	return parent_scmi_id;
}

int32_t plat_scmi_clock_set_parent(unsigned int agent_id,
				   unsigned int scmi_id,
				   unsigned int parent_id)
{
	int status;

	VERBOSE("%s: agent_id = %d, scmi_id = %d parent_id = %d\n", __func__,
		agent_id, scmi_id, parent_id);

	if (scmi_id >= ARRAY_SIZE(clock_table) || parent_id >= ARRAY_SIZE(clock_table))
		return SCMI_NOT_FOUND;

	status = clk_set_parent(scmi_id, parent_id);
	if (status)
		return SCMI_GENERIC_ERROR;

	return SCMI_SUCCESS;
}
