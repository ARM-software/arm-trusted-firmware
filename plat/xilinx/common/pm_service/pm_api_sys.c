/*
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Versal system level PM-API functions and communication with PMC via
 * IPI interrupts
 */

#include <drivers/arm/gic_common.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <pm_api_sys.h>
#include <pm_client.h>
#include <pm_common.h>
#include <pm_defs.h>
#include <pm_ipi.h>
#include "pm_svc_main.h"

#define NUM_GICD_ISENABLER	((IRQ_MAX >> 5U) + 1U)

/* default shutdown/reboot scope is system(2) */
static uint32_t pm_shutdown_scope = XPM_SHUTDOWN_SUBTYPE_RST_SYSTEM;

/**
 * pm_get_shutdown_scope() - Get the currently set shutdown scope.
 *
 * Return: Shutdown scope value.
 *
 */
uint32_t pm_get_shutdown_scope(void)
{
	return pm_shutdown_scope;
}

/* PM API functions */

/**
 * pm_client_set_wakeup_sources - Set all devices with enabled interrupts as
 *                                wake sources in the XilPM.
 * @node_id: Node id of processor.
 *
 */
void pm_client_set_wakeup_sources(uint32_t node_id)
{
	uint32_t reg_num, device_id;
	uint8_t pm_wakeup_nodes_set[XPM_NODEIDX_DEV_MAX] = {0U};
	uint32_t isenabler1 = PLAT_GICD_BASE_VALUE + GICD_ISENABLER + 4U;

	zeromem(&pm_wakeup_nodes_set, (u_register_t)sizeof(pm_wakeup_nodes_set));

	for (reg_num = 0U; reg_num < NUM_GICD_ISENABLER; reg_num++) {
		uint32_t base_irq = reg_num << ISENABLER_SHIFT;
		uint32_t reg = mmio_read_32(isenabler1 + (reg_num << 2));

		if (reg == 0U) {
			continue;
		}

		while (reg != 0U) {
			enum pm_device_node_idx node_idx;
			uint32_t idx, irq, lowest_set = reg & (-reg);
			enum pm_ret_status ret;

			idx = (uint32_t)__builtin_ctz(lowest_set);
			irq = base_irq + idx;

			if (irq > IRQ_MAX) {
				break;
			}

			node_idx = irq_to_pm_node_idx(irq);
			reg &= ~lowest_set;

			if (node_idx > XPM_NODEIDX_DEV_MIN) {
				if (pm_wakeup_nodes_set[node_idx] == 0U) {
					/* Get device ID from node index */
					device_id = PERIPH_DEVID((uint32_t)node_idx);
					ret = pm_set_wakeup_source(node_id,
								   device_id, 1U,
								   SECURE_FLAG);
					pm_wakeup_nodes_set[node_idx] = (ret == PM_RET_SUCCESS) ?
										 1U : 0U;
				}
			}
		}
	}
}

/**
 * pm_handle_eemi_call() - PM call for processor to send eemi payload.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 * @x0: Arguments received per SMC64 standard.
 * @x1: Arguments received per SMC64 standard.
 * @x2: Arguments received per SMC64 standard.
 * @x3: Arguments received per SMC64 standard.
 * @x4: Arguments received per SMC64 standard.
 * @x5: Arguments received per SMC64 standard.
 * @result: Payload received from firmware.
 *
 * Return: PM_RET_SUCCESS on success or error code.
 *
 */
enum pm_ret_status pm_handle_eemi_call(uint32_t flag, uint32_t x0, uint32_t x1,
				       uint32_t x2, uint32_t x3, uint32_t x4,
				       uint32_t x5, uint64_t *result)
{
	uint32_t payload[PAYLOAD_ARG_CNT] = {0};
	uint32_t module_id;

	module_id = (x0 & MODULE_ID_MASK) >> 8U;

	//default module id is for LIBPM
	if (module_id == 0) {
		module_id = LIBPM_MODULE_ID;
	}

	PM_PACK_PAYLOAD6(payload, module_id, flag, x0, x1, x2, x3, x4, x5);
	return pm_ipi_send_sync(primary_proc, payload, (uint32_t *)result, PAYLOAD_ARG_CNT);
}

/**
 * pm_self_suspend() - PM call for processor to suspend itself
 * @nid: Node id of the processor or subsystem.
 * @latency: Requested maximum wakeup latency (not supported).
 * @state: Requested state.
 * @address: Resume address.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * This is a blocking call, it will return only once PMU has responded.
 * On a wakeup, resume address will be automatically set by PMU.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_self_suspend(uint32_t nid,
				   uint32_t latency,
				   uint32_t state,
				   uintptr_t address, uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	uint32_t cpuid = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpuid);

	if (proc == NULL) {
		WARN("Failed to get proc %d\n", cpuid);
		return PM_RET_ERROR_INTERNAL;
	}

	/*
	 * Do client specific suspend operations
	 * (e.g. set powerdown request bit)
	 */
	pm_client_suspend(proc, state);

	/* Send request to the PLM */
	PM_PACK_PAYLOAD6(payload, LIBPM_MODULE_ID, flag, PM_SELF_SUSPEND,
			 proc->node_id, latency, state, address,
			 (address >> 32));
	return pm_ipi_send_sync(proc, payload, NULL, 0);
}

/**
 * pm_abort_suspend() - PM call to announce that a prior suspend request
 *                      is to be aborted.
 * @reason: Reason for the abort.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * Calling PU expects the PMU to abort the initiated suspend procedure.
 * This is a non-blocking call without any acknowledge.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_abort_suspend(enum pm_abort_reason reason, uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/*
	 * Do client specific abort suspend operations
	 * (e.g. enable interrupts and clear powerdown request bit)
	 */
	pm_client_abort_suspend();

	/* Send request to the PLM */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, flag, PM_ABORT_SUSPEND,
			 reason, primary_proc->node_id);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_req_suspend() - PM call to request for another PU or subsystem to
 *                    be suspended gracefully.
 * @target: Node id of the targeted PU or subsystem.
 * @ack: Flag to specify whether acknowledge is requested.
 * @latency: Requested wakeup latency (not supported)
 * @state: Requested state (not supported).
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_req_suspend(uint32_t target, uint8_t ack,
				  uint32_t latency, uint32_t state,
				  uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD4(payload, LIBPM_MODULE_ID, flag, PM_REQ_SUSPEND, target,
			 latency, state);
	if (ack == IPI_BLOCKING) {
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	} else {
		return pm_ipi_send(primary_proc, payload);
	}
}

/**
 * pm_req_wakeup() - PM call for processor to wake up selected processor
 *                   or subsystem.
 * @target: Device ID of the processor or subsystem to wake up.
 * @set_address: Resume address presence indicator.
 *               1 - resume address specified, 0 - otherwise.
 * @address: Resume address.
 * @ack: Flag to specify whether acknowledge requested.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * This API function is either used to power up another APU core for SMP
 * (by PSCI) or to power up an entirely different PU or subsystem, such
 * as RPU0, RPU, or PL_CORE_xx. Resume address for the target PU will be
 * automatically set by PMC.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_req_wakeup(uint32_t target, uint32_t set_address,
				 uintptr_t address, uint8_t ack, uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC to perform the wake of the PU */
	PM_PACK_PAYLOAD5(payload, LIBPM_MODULE_ID, flag, PM_REQ_WAKEUP, target,
			 set_address, address, ack);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_get_callbackdata() - Read from IPI response buffer.
 * @data: array of PAYLOAD_ARG_CNT elements.
 * @count: Number of values to return.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 * @ack: 0 - Do not ack IPI after reading payload.
 *       1 - Ack IPI after reading payload.
 *
 * Read value from ipi buffer response buffer.
 * Return: Returns status, either success or error.
 *
 */
enum pm_ret_status pm_get_callbackdata(uint32_t *data, size_t count, uint32_t flag, uint32_t ack)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	/* Return if interrupt is not from PMU */
	if (pm_ipi_irq_status(primary_proc) == 0) {
		return ret;
	}

	ret = pm_ipi_buff_read_callb(data, count);

	if (ack != 0U) {
		pm_ipi_irq_clear(primary_proc);
	}

	return ret;
}

/**
 * pm_pll_set_param() - Set PLL parameter.
 * @clk_id: PLL clock ID.
 * @param: PLL parameter ID.
 * @value: Value to set for PLL parameter.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * This API is deprecated and maintained here for backward compatibility.
 * New use of this API should be avoided for versal platform.
 * This API and its use cases will be removed for versal platform.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_pll_set_param(uint32_t clk_id, uint32_t param,
				    uint32_t value, uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD4(payload, LIBPM_MODULE_ID, flag, PM_PLL_SET_PARAMETER,
			 clk_id, param, value);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pll_get_param() - Get PLL parameter value.
 * @clk_id: PLL clock ID.
 * @param: PLL parameter ID.
 * @value: Buffer to store PLL parameter value.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * This API is deprecated and maintained here for backward compatibility.
 * New use of this API should be avoided for versal platform.
 * This API and its use cases will be removed for versal platform.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_pll_get_param(uint32_t clk_id, uint32_t param,
				    uint32_t *value, uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, flag, PM_PLL_GET_PARAMETER,
			 clk_id, param);

	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/**
 * pm_pll_set_mode() - Set PLL mode.
 * @clk_id: PLL clock ID.
 * @mode: PLL mode.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * This API is deprecated and maintained here for backward compatibility.
 * New use of this API should be avoided for versal platform.
 * This API and its use cases will be removed for versal platform.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_pll_set_mode(uint32_t clk_id, uint32_t mode,
				   uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, flag, PM_PLL_SET_MODE,
			 clk_id, mode);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pll_get_mode() - Get PLL mode.
 * @clk_id: PLL clock ID.
 * @mode: Buffer to store PLL mode.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * This API is deprecated and maintained here for backward compatibility.
 * New use of this API should be avoided for versal platform.
 * This API and its use cases will be removed for versal platform.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_pll_get_mode(uint32_t clk_id, uint32_t *mode,
				   uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, flag, PM_PLL_GET_MODE,
			 clk_id);

	return pm_ipi_send_sync(primary_proc, payload, mode, 1);
}

/**
 * pm_force_powerdown() - PM call to request for another PU or subsystem to
 *                        be powered down forcefully.
 * @target: Device ID of the PU node to be forced powered down.
 * @ack: Flag to specify whether acknowledge is requested
 * @flag: 0 - Call from secure source
 *        1 - Call from non-secure source
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_force_powerdown(uint32_t target, uint8_t ack,
				      uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, flag, PM_FORCE_POWERDOWN,
			 target, ack);

	if (ack == IPI_BLOCKING) {
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	} else {
		return pm_ipi_send(primary_proc, payload);
	}
}

/**
 * pm_system_shutdown() - PM call to request a system shutdown or restart.
 * @type: Shutdown or restart? 0=shutdown, 1=restart, 2=setscope.
 * @subtype: Scope: 0=APU-subsystem, 1=PS, 2=system.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_system_shutdown(uint32_t type, uint32_t subtype,
				      uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	if (type == XPM_SHUTDOWN_TYPE_SETSCOPE_ONLY) {
		/* Setting scope for subsequent PSCI reboot or shutdown */
		pm_shutdown_scope = subtype;
		return PM_RET_SUCCESS;
	}

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, LIBPM_MODULE_ID, flag, PM_SYSTEM_SHUTDOWN,
			 type, subtype);

	return pm_ipi_send_non_blocking(primary_proc, payload);
}

/**
 * pm_query_data() -  PM API for querying firmware data.
 * @qid: The type of data to query.
 * @arg1: Argument 1 to requested query data call.
 * @arg2: Argument 2 to requested query data call.
 * @arg3: Argument 3 to requested query data call.
 * @data: Returned output data.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * This API is deprecated and maintained here for backward compatibility.
 * New use of this API should be avoided for versal platform.
 * This API and its use cases will be removed for versal platform.
 *
 * Return: 0 if success else non-zero error code of type
 *         enum pm_ret_status.
 *
 */
enum pm_ret_status pm_query_data(uint32_t qid, uint32_t arg1, uint32_t arg2,
				 uint32_t arg3, uint32_t *data, uint32_t flag)
{
	uint32_t ret;
	uint32_t version[PAYLOAD_ARG_CNT] = {0};
	uint32_t payload[PAYLOAD_ARG_CNT];
	uint32_t fw_api_version;

	/* Send request to the PMC */
	PM_PACK_PAYLOAD5(payload, LIBPM_MODULE_ID, flag, PM_QUERY_DATA, qid,
			 arg1, arg2, arg3);

	ret = pm_feature_check((uint32_t)PM_QUERY_DATA, &version[0], flag);
	if (ret == PM_RET_SUCCESS) {
		fw_api_version = version[0] & 0xFFFFU;
		if ((fw_api_version == 2U) &&
		    ((qid == XPM_QID_CLOCK_GET_NAME) ||
		     (qid == XPM_QID_PINCTRL_GET_FUNCTION_NAME))) {
			ret = pm_ipi_send_sync(primary_proc, payload, data, PAYLOAD_ARG_CNT);
			if (ret == PM_RET_SUCCESS) {
				ret = data[0];
				data[0] = data[1];
				data[1] = data[2];
				data[2] = data[3];
			}
		} else {
			ret = pm_ipi_send_sync(primary_proc, payload, data, PAYLOAD_ARG_CNT);
		}
	}
	return ret;
}
/**
 * pm_api_ioctl() -  PM IOCTL API for device control and configs.
 * @device_id: Device ID.
 * @ioctl_id: ID of the requested IOCTL.
 * @arg1: Argument 1 to requested IOCTL call.
 * @arg2: Argument 2 to requested IOCTL call.
 * @arg3: Argument 3 to requested IOCTL call.
 * @value: Returned output value.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * This API is deprecated and maintained here for backward compatibility.
 * New use of this API should be avoided for versal platform.
 * This API and its use cases will be removed for versal platform.
 *
 * This function calls IOCTL to firmware for device control and configuration.
 *
 * Return: Returns status, either 0 on success or non-zero error code
 *         of type enum pm_ret_status.
 *
 */
enum pm_ret_status pm_api_ioctl(uint32_t device_id, uint32_t ioctl_id,
				uint32_t arg1, uint32_t arg2, uint32_t arg3,
				uint32_t *value, uint32_t flag)
{
	enum pm_ret_status ret;

	switch (ioctl_id) {
	case IOCTL_SET_PLL_FRAC_MODE:
		ret =  pm_pll_set_mode(arg1, arg2, flag);
		break;
	case IOCTL_GET_PLL_FRAC_MODE:
		ret =  pm_pll_get_mode(arg1, value, flag);
		break;
	case IOCTL_SET_PLL_FRAC_DATA:
		ret =  pm_pll_set_param(arg1, (uint32_t)PM_PLL_PARAM_DATA, arg2, flag);
		break;
	case IOCTL_GET_PLL_FRAC_DATA:
		ret =  pm_pll_get_param(arg1, (uint32_t)PM_PLL_PARAM_DATA, value, flag);
		break;
	case IOCTL_SET_SGI:
		/* Get the sgi number */
		ret = pm_register_sgi(arg1, arg2);
		if (ret != 0) {
			return PM_RET_ERROR_ARGS;
		}
		ret = PM_RET_SUCCESS;
		break;
	default:
		return PM_RET_ERROR_NOTSUPPORTED;
	}

	return ret;
}

/**
 * pm_set_wakeup_source() - PM call to specify the wakeup source while
 *                          suspended.
 * @target: Device id of the targeted PU or subsystem
 * @wkup_device: Device id of the wakeup peripheral
 * @enable: Enable or disable the specified peripheral as wake source
 * @flag: 0 - Call from secure source
 *        1 - Call from non-secure source
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_set_wakeup_source(uint32_t target, uint32_t wkup_device,
					uint8_t enable, uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD4(payload, LIBPM_MODULE_ID, flag, PM_SET_WAKEUP_SOURCE,
			 target, wkup_device, enable);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_feature_check() - Returns the supported API version if supported.
 * @api_id: API ID to check.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 * @ret_payload: pointer to array of PAYLOAD_ARG_CNT number of
 *               words Returned supported API version and bitmasks
 *               for IOCTL and QUERY ID
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_feature_check(uint32_t api_id, uint32_t *ret_payload,
				    uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	uint32_t module_id;

	/* Return version of API which are implemented in TF-A only */
	switch (api_id) {
	case PM_GET_CALLBACK_DATA:
	case PM_GET_TRUSTZONE_VERSION:
		ret_payload[0] = PM_API_VERSION_2;
		return PM_RET_SUCCESS;
	case TF_A_PM_REGISTER_SGI:
		ret_payload[0] = PM_API_BASE_VERSION;
		return PM_RET_SUCCESS;
	default:
		break;
	}

	module_id = (api_id & MODULE_ID_MASK) >> 8U;

	/*
	 * feature check should be done only for LIBPM module
	 * If module_id is 0, then we consider it LIBPM module as default id
	 */
	if ((module_id > 0) && (module_id != LIBPM_MODULE_ID)) {
		return PM_RET_SUCCESS;
	}

	PM_PACK_PAYLOAD2(payload, LIBPM_MODULE_ID, flag,
			 PM_FEATURE_CHECK, api_id);
	return pm_ipi_send_sync(primary_proc, payload, ret_payload, PAYLOAD_ARG_CNT);
}

/**
 * pm_load_pdi() - Load the PDI. This function provides support to load
 *                 PDI from linux.
 *
 * @src: Source device of pdi(DDR, OCM, SD etc).
 * @address_low: lower 32-bit Linear memory space address.
 * @address_high: higher 32-bit Linear memory space address.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_load_pdi(uint32_t src, uint32_t address_low,
			       uint32_t address_high, uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD4(payload, LOADER_MODULE_ID, flag, PM_LOAD_PDI, src,
			 address_high, address_low);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_register_notifier() - PM call to register a subsystem to be notified
 *                          about the device event.
 * @device_id: Device ID for the Node to which the event is related.
 * @event: Event in question.
 * @wake: Wake subsystem upon capturing the event if value 1.
 * @enable: Enable the registration for value 1, disable for value 0.
 * @flag: 0 - Call from secure source.
 *        1 - Call from non-secure source.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_register_notifier(uint32_t device_id, uint32_t event,
					uint32_t wake, uint32_t enable,
					uint32_t flag)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD5(payload, LIBPM_MODULE_ID, flag, PM_REGISTER_NOTIFIER,
			 device_id, event, wake, enable);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_get_chipid() - Read silicon ID registers.
 * @value: Buffer for two 32bit words.
 *
 * Return: Returns status, either success or error+reason and,
 *         optionally, @value.
 */
enum pm_ret_status pm_get_chipid(uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD1(payload, LIBPM_MODULE_ID, SECURE_FLAG, PM_GET_CHIPID);

	return pm_ipi_send_sync(primary_proc, payload, value, 2);
}
