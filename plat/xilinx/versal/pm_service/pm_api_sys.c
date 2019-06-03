/*
 * Copyright (c) 2019, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Versal system level PM-API functions and communication with PMC via
 * IPI interrupts
 */

#include <pm_common.h>
#include <pm_ipi.h>
#include <plat/common/platform.h>
#include "pm_api_sys.h"
#include "pm_client.h"

/*********************************************************************
 * Target module IDs macros
 ********************************************************************/
#define LIBPM_MODULE_ID		0x2
#define LOADER_MODULE_ID	0x7

/* default shutdown/reboot scope is system(2) */
static unsigned int pm_shutdown_scope = XPM_SHUTDOWN_SUBTYPE_RST_SYSTEM;

/**
 * pm_get_shutdown_scope() - Get the currently set shutdown scope
 *
 * @return	Shutdown scope value
 */
unsigned int pm_get_shutdown_scope(void)
{
	return pm_shutdown_scope;
}

/**
 * Assigning of argument values into array elements.
 */
#define PM_PACK_PAYLOAD1(pl, mid, arg0) {	\
	pl[0] = (uint32_t)((uint32_t)((arg0) & 0xFF) | (mid << 8)); \
}

#define PM_PACK_PAYLOAD2(pl, mid, arg0, arg1) {		\
	pl[1] = (uint32_t)(arg1);			\
	PM_PACK_PAYLOAD1(pl, mid, arg0);		\
}

#define PM_PACK_PAYLOAD3(pl, mid, arg0, arg1, arg2) {	\
	pl[2] = (uint32_t)(arg2);			\
	PM_PACK_PAYLOAD2(pl, mid, arg0, arg1);		\
}

#define PM_PACK_PAYLOAD4(pl, mid, arg0, arg1, arg2, arg3) {	\
	pl[3] = (uint32_t)(arg3);				\
	PM_PACK_PAYLOAD3(pl, mid, arg0, arg1, arg2);		\
}

#define PM_PACK_PAYLOAD5(pl, mid, arg0, arg1, arg2, arg3, arg4) {	\
	pl[4] = (uint32_t)(arg4);					\
	PM_PACK_PAYLOAD4(pl, mid, arg0, arg1, arg2, arg3);		\
}

#define PM_PACK_PAYLOAD6(pl, mid, arg0, arg1, arg2, arg3, arg4, arg5) {	\
	pl[5] = (uint32_t)(arg5);					\
	PM_PACK_PAYLOAD5(pl, mid, arg0, arg1, arg2, arg3, arg4);	\
}

/* PM API functions */

/**
 * pm_get_api_version() - Get version number of PMC PM firmware
 * @version	Returns 32-bit version number of PMC Power Management Firmware
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_api_version(unsigned int *version)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD1(payload, LIBPM_MODULE_ID, PM_GET_API_VERSION);
	return pm_ipi_send_sync(primary_proc, payload, version, 1);
}

/**
 * pm_self_suspend() - PM call for processor to suspend itself
 * @nid		Node id of the processor or subsystem
 * @latency	Requested maximum wakeup latency (not supported)
 * @state	Requested state
 * @address	Resume address
 *
 * This is a blocking call, it will return only once PMU has responded.
 * On a wakeup, resume address will be automatically set by PMU.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_self_suspend(uint32_t nid,
				   unsigned int latency,
				   unsigned int state,
				   uintptr_t address)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	unsigned int cpuid = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpuid);

	if (!proc) {
		WARN("Failed to get proc %d\n", cpuid);
		return PM_RET_ERROR_INTERNAL;
	}

	/*
	 * Do client specific suspend operations
	 * (e.g. set powerdown request bit)
	 */
	pm_client_suspend(proc, state);

	/* Send request to the PLM */
	PM_PACK_PAYLOAD6(payload, LIBPM_MODULE_ID, PM_SELF_SUSPEND,
			 proc->node_id, latency, state, address,
			 (address >> 32));
	return pm_ipi_send_sync(proc, payload, NULL, 0);
}

/**
 * pm_abort_suspend() - PM call to announce that a prior suspend request
 *			is to be aborted.
 * @reason	Reason for the abort
 *
 * Calling PU expects the PMU to abort the initiated suspend procedure.
 * This is a non-blocking call without any acknowledge.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_abort_suspend(enum pm_abort_reason reason)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/*
	 * Do client specific abort suspend operations
	 * (e.g. enable interrupts and clear powerdown request bit)
	 */
	pm_client_abort_suspend();

	/* Send request to the PLM */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_ABORT_SUSPEND, reason,
			 primary_proc->node_id);
	return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_req_suspend() - PM call to request for another PU or subsystem to
 *		      be suspended gracefully.
 * @target	Node id of the targeted PU or subsystem
 * @ack		Flag to specify whether acknowledge is requested
 * @latency	Requested wakeup latency (not supported)
 * @state	Requested state (not supported)
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_req_suspend(uint32_t target, uint8_t ack,
				  unsigned int latency, unsigned int state)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD4(payload, LIBPM_MODULE_ID, PM_REQ_SUSPEND, target,
			 latency, state);
	if (ack == IPI_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_req_wakeup() - PM call for processor to wake up selected processor
 *		     or subsystem
 * @target	Device ID of the processor or subsystem to wake up
 * @set_address	Resume address presence indicator
 *		1 - resume address specified, 0 - otherwise
 * @address	Resume address
 * @ack		Flag to specify whether acknowledge requested
 *
 * This API function is either used to power up another APU core for SMP
 * (by PSCI) or to power up an entirely different PU or subsystem, such
 * as RPU0, RPU, or PL_CORE_xx. Resume address for the target PU will be
 * automatically set by PMC.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_req_wakeup(uint32_t target, uint32_t set_address,
				 uintptr_t address, uint8_t ack)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC to perform the wake of the PU */
	PM_PACK_PAYLOAD5(payload, LIBPM_MODULE_ID, PM_REQ_WAKEUP, target,
			 set_address, address, ack);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_request_device() - Request a device
 * @device_id		Device ID
 * @capabilities	Requested capabilities for the device
 * @qos			Required Quality of Service
 * @ack			Flag to specify whether acknowledge requested
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_request_device(uint32_t device_id, uint32_t capabilities,
				     uint32_t qos, uint32_t ack)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD5(payload, LIBPM_MODULE_ID, PM_REQUEST_DEVICE,
			 device_id, capabilities, qos, ack);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_release_device() - Release a device
 * @device_id		Device ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_release_device(uint32_t device_id)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_RELEASE_DEVICE,
			 device_id);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_set_requirement() - Set requirement for the device
 * @device_id		Device ID
 * @capabilities	Requested capabilities for the device
 * @latency		Requested maximum latency
 * @qos			Required Quality of Service
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_set_requirement(uint32_t device_id, uint32_t capabilities,
				      uint32_t latency, uint32_t qos)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD5(payload, LIBPM_MODULE_ID, PM_SET_REQUIREMENT,
			 device_id, capabilities, latency, qos);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_get_device_status() - Get device's status
 * @device_id		Device ID
 * @response		Buffer to store device status response
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_device_status(uint32_t device_id, uint32_t *response)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_GET_DEVICE_STATUS,
			 device_id);

	return pm_ipi_send_sync(primary_proc, payload, response, 3);
}

/**
 * pm_reset_assert() - Assert/De-assert reset
 * @reset	Reset ID
 * @assert	Assert (1) or de-assert (0)
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_reset_assert(uint32_t reset, bool assert)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_RESET_ASSERT, reset,
			 assert);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_reset_get_status() - Get current status of a reset line
 * @reset	Reset ID
 * @status	Returns current status of selected reset ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_reset_get_status(uint32_t reset, uint32_t *status)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_RESET_ASSERT, reset);

	return pm_ipi_send_sync(primary_proc, payload, status, 1);
}

/**
 * pm_get_callbackdata() - Read from IPI response buffer
 * @data - array of PAYLOAD_ARG_CNT elements
 *
 * Read value from ipi buffer response buffer.
 */
void pm_get_callbackdata(uint32_t *data, size_t count)
{
	/* Return if interrupt is not from PMU */
	if (!pm_ipi_irq_status(primary_proc))
		return;

	pm_ipi_buff_read_callb(data, count);
	pm_ipi_irq_clear(primary_proc);
}

/**
 * pm_pinctrl_request() - Request a pin
 * @pin		Pin ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_request(uint32_t pin)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_PINCTRL_REQUEST, pin);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pinctrl_release() - Release a pin
 * @pin		Pin ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_release(uint32_t pin)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_PINCTRL_RELEASE, pin);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pinctrl_set_function() - Set pin function
 * @pin		Pin ID
 * @function	Function ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_set_function(uint32_t pin, uint32_t function)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_PINCTRL_SET_FUNCTION, pin,
			 function)

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pinctrl_get_function() - Get function set on the pin
 * @pin		Pin ID
 * @function	Function set on the pin
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_get_function(uint32_t pin, uint32_t *function)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_PINCTRL_SET_FUNCTION,
			 pin);

	return pm_ipi_send_sync(primary_proc, payload, function, 1);
}

/**
 * pm_pinctrl_set_pin_param() - Set configuration parameter for the pin
 * @pin		Pin ID
 * @param	Parameter ID
 * @value	Parameter value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_set_pin_param(uint32_t pin, uint32_t param,
					    uint32_t value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD4(payload, LIBPM_MODULE_ID, PM_PINCTRL_CONFIG_PARAM_SET,
			 pin, param, value);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pinctrl_get_pin_param() - Get configuration parameter value for the pin
 * @pin		Pin ID
 * @param	Parameter ID
 * @value	Buffer to store parameter value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_get_pin_param(uint32_t pin, uint32_t param,
					    uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_PINCTRL_CONFIG_PARAM_GET,
			 pin, param);

	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/**
 * pm_clock_enable() - Enable the clock
 * @clk_id	Clock ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_enable(uint32_t clk_id)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_CLOCK_ENABLE, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_disable() - Disable the clock
 * @clk_id	Clock ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_disable(uint32_t clk_id)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_CLOCK_DISABLE, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_get_state() - Get clock status
 * @clk_id	Clock ID
 * @state:	Buffer to store clock status (1: Enabled, 0:Disabled)
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_get_state(uint32_t clk_id, uint32_t *state)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_CLOCK_GETSTATE, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, state, 1);
}

/**
 * pm_clock_set_divider() - Set divider for the clock
 * @clk_id	Clock ID
 * @divider	Divider value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_set_divider(uint32_t clk_id, uint32_t divider)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_CLOCK_SETDIVIDER, clk_id,
			 divider);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_get_divider() - Get divider value for the clock
 * @clk_id	Clock ID
 * @divider:	Buffer to store clock divider value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_get_divider(uint32_t clk_id, uint32_t *divider)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_CLOCK_GETDIVIDER, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, divider, 1);
}

/**
 * pm_clock_set_parent() - Set parent for the clock
 * @clk_id	Clock ID
 * @parent	Parent ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_set_parent(uint32_t clk_id, uint32_t parent)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_CLOCK_SETPARENT, clk_id,
			 parent);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_get_parent() - Get parent value for the clock
 * @clk_id	Clock ID
 * @parent:	Buffer to store clock parent value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_get_parent(uint32_t clk_id, uint32_t *parent)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_CLOCK_GETPARENT, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, parent, 1);
}

/**
 * pm_pll_set_param() - Set PLL parameter
 * @clk_id	PLL clock ID
 * @param	PLL parameter ID
 * @value	Value to set for PLL parameter
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pll_set_param(uint32_t clk_id, uint32_t param,
				    uint32_t value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD4(payload, LIBPM_MODULE_ID, PM_PLL_SET_PARAMETER, clk_id,
			 param, value);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pll_get_param() - Get PLL parameter value
 * @clk_id	PLL clock ID
 * @param	PLL parameter ID
 * @value:	Buffer to store PLL parameter value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pll_get_param(uint32_t clk_id, uint32_t param,
				    uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_PLL_GET_PARAMETER, clk_id,
			 param);

	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/**
 * pm_pll_set_mode() - Set PLL mode
 * @clk_id	PLL clock ID
 * @mode	PLL mode
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pll_set_mode(uint32_t clk_id, uint32_t mode)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_PLL_SET_MODE, clk_id,
			 mode);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pll_get_mode() - Get PLL mode
 * @clk_id	PLL clock ID
 * @mode:	Buffer to store PLL mode
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pll_get_mode(uint32_t clk_id, uint32_t *mode)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_PLL_GET_MODE, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, mode, 1);
}

/**
 * pm_force_powerdown() - PM call to request for another PU or subsystem to
 *			  be powered down forcefully
 * @target	Device ID of the PU node to be forced powered down.
 * @ack		Flag to specify whether acknowledge is requested
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_force_powerdown(uint32_t target, uint8_t ack)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_FORCE_POWERDOWN, target,
			 ack);

	if (ack == IPI_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_system_shutdown() - PM call to request a system shutdown or restart
 * @type	Shutdown or restart? 0=shutdown, 1=restart, 2=setscope
 * @subtype	Scope: 0=APU-subsystem, 1=PS, 2=system
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_system_shutdown(uint32_t type, uint32_t subtype)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	if (type == XPM_SHUTDOWN_TYPE_SETSCOPE_ONLY) {
		/* Setting scope for subsequent PSCI reboot or shutdown */
		pm_shutdown_scope = subtype;
		return PM_RET_SUCCESS;
	}

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_SYSTEM_SHUTDOWN, type,
			 subtype);

	return pm_ipi_send_non_blocking(primary_proc, payload);
}

/**
* pm_query_data() -  PM API for querying firmware data
* @qid	The type of data to query
* @arg1	Argument 1 to requested query data call
* @arg2	Argument 2 to requested query data call
* @arg3	Argument 3 to requested query data call
* @data	Returned output data
*
* This function returns requested data.
*/
enum pm_ret_status pm_query_data(uint32_t qid, uint32_t arg1, uint32_t arg2,
				 uint32_t arg3, uint32_t *data)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD5(payload, LIBPM_MODULE_ID, PM_QUERY_DATA, qid, arg1,
			 arg2, arg3);
	return pm_ipi_send_sync(primary_proc, payload, data, 4);
}
/**
 * pm_api_ioctl() -  PM IOCTL API for device control and configs
 * @device_id	Device ID
 * @ioctl_id	ID of the requested IOCTL
 * @arg1	Argument 1 to requested IOCTL call
 * @arg2	Argument 2 to requested IOCTL call
 * @value	Returned output value
 *
 * This function calls IOCTL to firmware for device control and configuration.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_ioctl(uint32_t device_id, uint32_t ioctl_id,
				uint32_t arg1, uint32_t arg2, uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	switch (ioctl_id) {
	case IOCTL_SET_PLL_FRAC_MODE:
		return pm_pll_set_mode(arg1, arg2);
	case IOCTL_GET_PLL_FRAC_MODE:
		return pm_pll_get_mode(arg1, value);
	case IOCTL_SET_PLL_FRAC_DATA:
		return pm_pll_set_param(arg1, PM_PLL_PARAM_DATA, arg2);
	case IOCTL_GET_PLL_FRAC_DATA:
		return pm_pll_get_param(arg1, PM_PLL_PARAM_DATA, value);
	default:
		/* Send request to the PMC */
		PM_PACK_PAYLOAD5(payload, LIBPM_MODULE_ID, PM_IOCTL, device_id,
				 ioctl_id, arg1, arg2);
		return pm_ipi_send_sync(primary_proc, payload, value, 1);
	}
}

/**
 * pm_set_wakeup_source() - PM call to specify the wakeup source while suspended
 * @target	Device id of the targeted PU or subsystem
 * @wkup_node	Device id of the wakeup peripheral
 * @enable	Enable or disable the specified peripheral as wake source
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_set_wakeup_source(uint32_t target, uint32_t wkup_device,
					uint8_t enable)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD4(payload, LIBPM_MODULE_ID, PM_SET_WAKEUP_SOURCE, target,
			 wkup_device, enable);
	return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_get_chipid() - Read silicon ID registers
 * @value       Buffer for return values. Must be large enough
 *		to hold 8 bytes.
 *
 * @return      Returns silicon ID registers
 */
enum pm_ret_status pm_get_chipid(uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD1(payload, LIBPM_MODULE_ID, PM_GET_CHIPID);

	return pm_ipi_send_sync(primary_proc, payload, value, 2);
}

/**
 * pm_feature_check() - Returns the supported API version if supported
 * @api_id	API ID to check
 * @value	Returned supported API version
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_feature_check(uint32_t api_id, unsigned int *version)
{
	uint32_t payload[PAYLOAD_ARG_CNT], fw_api_version;
	uint32_t status;

	switch (api_id) {
	case PM_GET_CALLBACK_DATA:
	case PM_GET_TRUSTZONE_VERSION:
	case PM_INIT_FINALIZE:
		*version = (PM_API_BASE_VERSION << 16);
		return PM_RET_SUCCESS;
	case PM_GET_API_VERSION:
	case PM_GET_DEVICE_STATUS:
	case PM_GET_OP_CHARACTERISTIC:
	case PM_REQ_SUSPEND:
	case PM_SELF_SUSPEND:
	case PM_FORCE_POWERDOWN:
	case PM_ABORT_SUSPEND:
	case PM_REQ_WAKEUP:
	case PM_SET_WAKEUP_SOURCE:
	case PM_SYSTEM_SHUTDOWN:
	case PM_REQUEST_DEVICE:
	case PM_RELEASE_DEVICE:
	case PM_SET_REQUIREMENT:
	case PM_RESET_ASSERT:
	case PM_RESET_GET_STATUS:
	case PM_PINCTRL_REQUEST:
	case PM_PINCTRL_RELEASE:
	case PM_PINCTRL_GET_FUNCTION:
	case PM_PINCTRL_SET_FUNCTION:
	case PM_PINCTRL_CONFIG_PARAM_GET:
	case PM_PINCTRL_CONFIG_PARAM_SET:
	case PM_IOCTL:
	case PM_QUERY_DATA:
	case PM_CLOCK_ENABLE:
	case PM_CLOCK_DISABLE:
	case PM_CLOCK_GETSTATE:
	case PM_CLOCK_SETDIVIDER:
	case PM_CLOCK_GETDIVIDER:
	case PM_CLOCK_SETPARENT:
	case PM_CLOCK_GETPARENT:
	case PM_PLL_SET_PARAMETER:
	case PM_PLL_GET_PARAMETER:
	case PM_PLL_SET_MODE:
	case PM_PLL_GET_MODE:
	case PM_FEATURE_CHECK:
		*version = (PM_API_BASE_VERSION << 16);
		break;
	case PM_LOAD_PDI:
		*version = (PM_API_BASE_VERSION << 16);
		return PM_RET_SUCCESS;
	default:
		*version = 0U;
		return PM_RET_ERROR_NOFEATURE;
	}

	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, PM_FEATURE_CHECK, api_id);

	status = pm_ipi_send_sync(primary_proc, payload, &fw_api_version, 1);
	if (status != PM_RET_SUCCESS)
		return status;

	*version |= fw_api_version;

	return PM_RET_SUCCESS;
}

/**
 * pm_load_pdi() - Load the PDI
 *
 * This function provides support to load PDI from linux
 *
 * src:        Source device of pdi(DDR, OCM, SD etc)
 * address_low: lower 32-bit Linear memory space address
 * address_high: higher 32-bit Linear memory space address
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_load_pdi(uint32_t src,
			       uint32_t address_low, uint32_t address_high)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD4(payload, LOADER_MODULE_ID, PM_LOAD_PDI, src,
			 address_high, address_low);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_get_op_characteristic() - PM call to request operating characteristics
 *                              of a device
 * @device_id   Device id
 * @type        Type of the operating characteristic
 *              (power, temperature and latency)
 * @result      Returns the operating characteristic for the requested device,
 *              specified by the type
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_op_characteristic(uint32_t device_id,
					    enum pm_opchar_type type,
					    uint32_t *result)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, PM_GET_OP_CHARACTERISTIC,
			 device_id, type);
	return pm_ipi_send_sync(primary_proc, payload, result, 1);
}
