/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for clock control.
 */

#include <arch_helpers.h>
#include <mmio.h>
#include <platform.h>
#include <string.h>
#include "pm_api_clock.h"
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_common.h"
#include "pm_ipi.h"

#define CLK_NODE_MAX			6

/**
 * struct pm_clock_node - Clock topology node information
 * @type:	Topology type (mux/div1/div2/gate/pll/fixed factor)
 * @offset:	Offset in control register
 * @width:	Width of the specific type in control register
 * @clkflags:	Clk specific flags
 * @typeflags:	Type specific flags
 * @mult:	Multiplier for fixed factor
 * @div:	Divisor for fixed factor
 */
struct pm_clock_node {
	uint16_t clkflags;
	uint16_t typeflags;
	uint8_t type;
	uint8_t offset;
	uint8_t width;
	uint8_t mult:4;
	uint8_t div:4;
};

/**
 * struct pm_clock - Clock structure
 * @name:	Clock name
 * @control_reg:	Control register address
 * @status_reg:	Status register address
 * @parents:	Parents for first clock node. Lower byte indicates parent
 *		clock id and upper byte indicate flags for that id.
 * pm_clock_node:	Clock nodes
 */
struct pm_clock {
	char name[CLK_NAME_LEN];
	uint8_t num_nodes;
	unsigned int control_reg;
	unsigned int status_reg;
	int32_t (*parents)[];
	struct pm_clock_node(*nodes)[];
};

/* Clock array containing clock informaton */
struct pm_clock clocks[] = {0};

/**
 * pm_api_clock_get_name() - PM call to request a clock's name
 * @clock_id	Clock ID
 * @name	Name of clock (max 16 bytes)
 *
 * This function is used by master to get nmae of clock specified
 * by given clock ID.
 *
 * @return	Returns success. In case of error, name data is 0.
 */
enum pm_ret_status pm_api_clock_get_name(unsigned int clock_id, char *name)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_get_topology() - PM call to request a clock's topology
 * @clock_id	Clock ID
 * @index	Topology index for next toplogy node
 * @topology	Buffer to store nodes in topology and flags
 *
 * This function is used by master to get topology information for the
 * clock specified by given clock ID. Each response would return 3
 * topology nodes. To get next nodes, caller needs to call this API with
 * index of next node. Index starts from 0.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clock_get_topology(unsigned int clock_id,
					     unsigned int index,
					     uint32_t *topology)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_get_fixedfactor_params() - PM call to request a clock's fixed
 *					   factor parameters for fixed clock
 * @clock_id	Clock ID
 * @mul		Multiplication value
 * @div		Divisor value
 *
 * This function is used by master to get fixed factor parameers for the
 * fixed clock. This API is application only for the fixed clock.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clock_get_fixedfactor_params(unsigned int clock_id,
						       uint32_t *mul,
						       uint32_t *div)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_get_parents() - PM call to request a clock's first 3 parents
 * @clock_id	Clock ID
 * @index	Index of next parent
 * @parents	Parents of the given clock
 *
 * This function is used by master to get clock's parents information.
 * This API will return 3 parents with a single response. To get other
 * parents, master should call same API in loop with new parent index
 * till error is returned.
 *
 * E.g First call should have index 0 which will return parents 0, 1 and
 * 2. Next call, index should be 3 which will return parent 3,4 and 5 and
 * so on.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clock_get_parents(unsigned int clock_id,
					    unsigned int index,
					    uint32_t *parents)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_get_attributes() - PM call to request a clock's attributes
 * @clock_id	Clock ID
 * @attr	Clock attributes
 *
 * This function is used by master to get clock's attributes
 * (e.g. valid, clock type, etc).
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clock_get_attributes(unsigned int clock_id,
					       uint32_t *attr)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_enable() - Enable the clock for given id
 * @clock_id: Id of the clock to be enabled
 *
 * This function is used by master to enable the clock
 * including peripherals and PLL clocks.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_enable(unsigned int clock_id)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_disable - Disable the clock for given id
 * @clock_id	Id of the clock to be disable
 *
 * This function is used by master to disable the clock
 * including peripherals and PLL clocks.
 *
 * Return: Returns status, either success or error+reason.
 */

enum pm_ret_status pm_api_clock_disable(unsigned int clock_id)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_getstate - Get the clock state for given id
 * @clock_id	Id of the clock to be queried
 * @state	1/0 (Enabled/Disabled)
 *
 * This function is used by master to get the state of clock
 * including peripherals and PLL clocks.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_getstate(unsigned int clock_id,
					 unsigned int *state)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_setdivider - Set the clock divider for given id
 * @clock_id	Id of the clock
 * @divider	Divider value
 *
 * This function is used by master to set divider for any clock
 * to achieve desired rate.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_setdivider(unsigned int clock_id,
					   unsigned int divider)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_getdivider - Get the clock divider for given id
 * @clock_id	Id of the clock
 * @divider	Divider value
 *
 * This function is used by master to get divider values
 * for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_getdivider(unsigned int clock_id,
					   unsigned int *divider)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_setrate - Set the clock rate for given id
 * @clock_id	Id of the clock
 * @rate	Rate value in hz
 *
 * This function is used by master to set rate for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_setrate(unsigned int clock_id,
					unsigned int rate)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_getrate - Get the clock rate for given id
 * @clock_id	Id of the clock
 * @rate	rate value in hz
 *
 * This function is used by master to get rate
 * for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_getrate(unsigned int clock_id,
					unsigned int *rate)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_setparent - Set the clock parent for given id
 * @clock_id	Id of the clock
 * @parent_id	parent id
 *
 * This function is used by master to set parent for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_setparent(unsigned int clock_id,
					  unsigned int parent_idx)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_getparent - Get the clock parent for given id
 * @clock_id	Id of the clock
 * @parent_id	parent id
 *
 * This function is used by master to get parent index
 * for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_getparent(unsigned int clock_id,
					  unsigned int *parent_idx)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clk_set_pll_mode() -  Set PLL mode
 * @pll     PLL id
 * @mode    Mode fraction/integar
 *
 * This function sets PLL mode.
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clk_set_pll_mode(unsigned int pll,
					   unsigned int mode)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_ioctl_get_pll_mode() -  Get PLL mode
 * @pll     PLL id
 * @mode    Mode fraction/integar
 *
 * This function returns current PLL mode.
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clk_get_pll_mode(unsigned int pll,
					   unsigned int *mode)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clk_set_pll_frac_data() -  Set PLL fraction data
 * @pll     PLL id
 * @data    fraction data
 *
 * This function sets fraction data. It is valid for fraction
 * mode only.
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clk_set_pll_frac_data(unsigned int pll,
						unsigned int data)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clk_get_pll_frac_data() - Get PLL fraction data
 * @pll     PLL id
 * @data    fraction data
 *
 * This function returns fraction data value.
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clk_get_pll_frac_data(unsigned int pll,
						unsigned int *data)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}
