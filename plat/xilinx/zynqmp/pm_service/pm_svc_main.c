/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Top-level SMC handler for ZynqMP power management calls and
 * IPI setup functions for communication with PMU.
 */

#include <errno.h>
#include <runtime_svc.h>
#include "../zynqmp_private.h"
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_ipi.h"

#define PM_GET_CALLBACK_DATA	0xa01
#define PM_SET_SUSPEND_MODE	0xa02
#define PM_GET_TRUSTZONE_VERSION	0xa03

/* !0 - UP, 0 - DOWN */
static int32_t pm_up = 0;

/**
 * pm_context - Structure which contains data for power management
 * @api_version		version of PM API, must match with one on PMU side
 * @payload		payload array used to store received
 *			data from ipi buffer registers
 */
static struct {
	uint32_t api_version;
	uint32_t payload[PAYLOAD_ARG_CNT];
} pm_ctx;

/**
 * pm_setup() - PM service setup
 *
 * @return	On success, the initialization function must return 0.
 *		Any other return value will cause the framework to ignore
 *		the service
 *
 * Initialization functions for ZynqMP power management for
 * communicaton with PMU.
 *
 * Called from sip_svc_setup initialization function with the
 * rt_svc_init signature.
 */
int pm_setup(void)
{
	int status, ret;

	status = pm_ipi_init(primary_proc);

	if (status >= 0) {
		INFO("BL31: PM Service Init Complete: API v%d.%d\n",
		     PM_VERSION_MAJOR, PM_VERSION_MINOR);
		ret = 0;
	} else {
		INFO("BL31: PM Service Init Failed, Error Code %d!\n", status);
		ret = status;
	}

	pm_up = !status;

	return ret;
}

/**
 * pm_smc_handler() - SMC handler for PM-API calls coming from EL1/EL2.
 * @smc_fid - Function Identifier
 * @x1 - x4 - Arguments
 * @cookie  - Unused
 * @handler - Pointer to caller's context structure
 *
 * @return  - Unused
 *
 * Determines that smc_fid is valid and supported PM SMC Function ID from the
 * list of pm_api_ids, otherwise completes the request with
 * the unknown SMC Function ID
 *
 * The SMC calls for PM service are forwarded from SIP Service SMC handler
 * function with rt_svc_handle signature
 */
uint64_t pm_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
			uint64_t x4, void *cookie, void *handle, uint64_t flags)
{
	enum pm_ret_status ret;

	uint32_t pm_arg[4];

	/* Handle case where PM wasn't initialized properly */
	if (!pm_up)
		SMC_RET1(handle, SMC_UNK);

	pm_arg[0] = (uint32_t)x1;
	pm_arg[1] = (uint32_t)(x1 >> 32);
	pm_arg[2] = (uint32_t)x2;
	pm_arg[3] = (uint32_t)(x2 >> 32);

	switch (smc_fid & FUNCID_NUM_MASK) {
	/* PM API Functions */
	case PM_SELF_SUSPEND:
		ret = pm_self_suspend(pm_arg[0], pm_arg[1], pm_arg[2],
				      pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_REQ_SUSPEND:
		ret = pm_req_suspend(pm_arg[0], pm_arg[1], pm_arg[2],
				     pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_REQ_WAKEUP:
	{
		/* Use address flag is encoded in the 1st bit of the low-word */
		unsigned int set_addr = pm_arg[1] & 0x1;
		uint64_t address = (uint64_t)pm_arg[2] << 32;

		address |= pm_arg[1] & (~0x1);
		ret = pm_req_wakeup(pm_arg[0], set_addr, address,
				    pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);
	}

	case PM_FORCE_POWERDOWN:
		ret = pm_force_powerdown(pm_arg[0], pm_arg[1]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_ABORT_SUSPEND:
		ret = pm_abort_suspend(pm_arg[0]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_SET_WAKEUP_SOURCE:
		ret = pm_set_wakeup_source(pm_arg[0], pm_arg[1], pm_arg[2]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_SYSTEM_SHUTDOWN:
		ret = pm_system_shutdown(pm_arg[0], pm_arg[1]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_REQ_NODE:
		ret = pm_req_node(pm_arg[0], pm_arg[1], pm_arg[2], pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_RELEASE_NODE:
		ret = pm_release_node(pm_arg[0]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_SET_REQUIREMENT:
		ret = pm_set_requirement(pm_arg[0], pm_arg[1], pm_arg[2],
					 pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_SET_MAX_LATENCY:
		ret = pm_set_max_latency(pm_arg[0], pm_arg[1]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_GET_API_VERSION:
		/* Check is PM API version already verified */
		if (pm_ctx.api_version == PM_VERSION) {
			SMC_RET1(handle, (uint64_t)PM_RET_SUCCESS |
				 ((uint64_t)PM_VERSION << 32));
		}

		ret = pm_get_api_version(&pm_ctx.api_version);
		/*
		 * Enable IPI IRQ
		 * assume the rich OS is OK to handle callback IRQs now.
		 * Even if we were wrong, it would not enable the IRQ in
		 * the GIC.
		 */
		pm_ipi_irq_enable(primary_proc);
		SMC_RET1(handle, (uint64_t)ret |
			 ((uint64_t)pm_ctx.api_version << 32));

	case PM_SET_CONFIGURATION:
		ret = pm_set_configuration(pm_arg[0]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_INIT_FINALIZE:
		ret = pm_init_finalize();
		SMC_RET1(handle, (uint64_t)ret);

	case PM_GET_NODE_STATUS:
	{
		uint32_t buff[3];

		ret = pm_get_node_status(pm_arg[0], buff);
		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)buff[0] << 32),
			 (uint64_t)buff[1] | ((uint64_t)buff[2] << 32));
	}

	case PM_GET_OP_CHARACTERISTIC:
	{
		uint32_t result;

		ret = pm_get_op_characteristic(pm_arg[0], pm_arg[1], &result);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)result << 32));
	}

	case PM_REGISTER_NOTIFIER:
		ret = pm_register_notifier(pm_arg[0], pm_arg[1], pm_arg[2],
					   pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_RESET_ASSERT:
		ret = pm_reset_assert(pm_arg[0], pm_arg[1]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_RESET_GET_STATUS:
	{
		uint32_t reset_status;

		ret = pm_reset_get_status(pm_arg[0], &reset_status);
		SMC_RET1(handle, (uint64_t)ret |
			 ((uint64_t)reset_status << 32));
	}

	/* PM memory access functions */
	case PM_MMIO_WRITE:
		ret = pm_mmio_write(pm_arg[0], pm_arg[1], pm_arg[2]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_MMIO_READ:
	{
		uint32_t value;

		ret = pm_mmio_read(pm_arg[0], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_FPGA_LOAD:
		ret = pm_fpga_load(pm_arg[0], pm_arg[1], pm_arg[2], pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_FPGA_GET_STATUS:
	{
		uint32_t value;

		ret = pm_fpga_get_status(&value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_GET_CHIPID:
	{
		uint32_t result[2];

		ret = pm_get_chipid(result);
		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)result[0] << 32),
			 result[1]);
	}

	case PM_SECURE_RSA_AES:
		ret = pm_secure_rsaaes(pm_arg[0], pm_arg[1], pm_arg[2],
				       pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_GET_CALLBACK_DATA:
	{
		uint32_t result[4];

		pm_get_callbackdata(result, sizeof(result));
		SMC_RET2(handle,
			 (uint64_t)result[0] | ((uint64_t)result[1] << 32),
			 (uint64_t)result[2] | ((uint64_t)result[3] << 32));
	}

	case PM_PINCTRL_REQUEST:
		ret = pm_pinctrl_request(pm_arg[0]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PINCTRL_RELEASE:
		ret = pm_pinctrl_release(pm_arg[0]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PINCTRL_GET_FUNCTION:
	{
		uint32_t value = 0;

		ret = pm_pinctrl_get_function(pm_arg[0], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_PINCTRL_SET_FUNCTION:
		ret = pm_pinctrl_set_function(pm_arg[0], pm_arg[1]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PINCTRL_CONFIG_PARAM_GET:
	{
		uint32_t value;

		ret = pm_pinctrl_get_config(pm_arg[0], pm_arg[1], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_PINCTRL_CONFIG_PARAM_SET:
		ret = pm_pinctrl_set_config(pm_arg[0], pm_arg[1], pm_arg[2]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_IOCTL:
	{
		uint32_t value;

		ret = pm_ioctl(pm_arg[0], pm_arg[1], pm_arg[2],
			       pm_arg[3], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_QUERY_DATA:
	{
		uint32_t data[4] = { 0 };

		ret = pm_query_data(pm_arg[0], pm_arg[1], pm_arg[2],
				    pm_arg[3], data);
		SMC_RET2(handle, (uint64_t)data[0]  | ((uint64_t)data[1] << 32),
			 (uint64_t)data[2] | ((uint64_t)data[3] << 32));
	}

	case PM_CLOCK_ENABLE:
		ret = pm_clock_enable(pm_arg[0]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_CLOCK_DISABLE:
		ret = pm_clock_disable(pm_arg[0]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_CLOCK_GETSTATE:
	{
		uint32_t value;

		ret = pm_clock_getstate(pm_arg[0], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_CLOCK_SETDIVIDER:
		ret = pm_clock_setdivider(pm_arg[0], pm_arg[1]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_CLOCK_GETDIVIDER:
	{
		uint32_t value;

		ret = pm_clock_getdivider(pm_arg[0], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_CLOCK_SETRATE:
		ret = pm_clock_setrate(pm_arg[0],
		       ((uint64_t)pm_arg[2]) << 32 | pm_arg[1]);

		SMC_RET1(handle, (uint64_t)ret);

	case PM_CLOCK_GETRATE:
	{
		uint64_t value;

		ret = pm_clock_getrate(pm_arg[0], &value);
		SMC_RET2(handle, (uint64_t)ret |
				  (((uint64_t)value & 0xFFFFFFFFU) << 32U),
			 (value >> 32U) & 0xFFFFFFFFU);

	}

	case PM_CLOCK_SETPARENT:
		ret = pm_clock_setparent(pm_arg[0], pm_arg[1]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_CLOCK_GETPARENT:
	{
		uint32_t value;

		ret = pm_clock_getparent(pm_arg[0], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_GET_TRUSTZONE_VERSION:
		SMC_RET1(handle, (uint64_t)PM_RET_SUCCESS |
			 ((uint64_t)ZYNQMP_TZ_VERSION << 32));

	case PM_SET_SUSPEND_MODE:
		ret = pm_set_suspend_mode(pm_arg[0]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_SECURE_SHA:
		ret = pm_sha_hash(pm_arg[0], pm_arg[1], pm_arg[2],
				pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_SECURE_RSA:
		ret = pm_rsa_core(pm_arg[0], pm_arg[1], pm_arg[2],
				       pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	default:
		WARN("Unimplemented PM Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}
