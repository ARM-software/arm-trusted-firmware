/*
 * Copyright (c) 2019-2020, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Top-level SMC handler for Versal power management calls and
 * IPI setup functions for communication with PMC.
 */

#include <errno.h>
#include <plat_private.h>
#include <stdbool.h>
#include <common/runtime_svc.h>
#include <plat/common/platform.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_ipi.h"
#include <drivers/arm/gicv3.h>

#define XSCUGIC_SGIR_EL1_INITID_SHIFT    24U
#define INVALID_SGI    0xFF
DEFINE_RENAME_SYSREG_RW_FUNCS(icc_asgi1r_el1, S3_0_C12_C11_6)

/* pm_up = true - UP, pm_up = false - DOWN */
static bool pm_up;
static unsigned int sgi = INVALID_SGI;

static uint64_t ipi_fiq_handler(uint32_t id, uint32_t flags, void *handle,
				void *cookie)
{
	int cpu;
	unsigned int reg;

	(void)plat_ic_acknowledge_interrupt();
	cpu = plat_my_core_pos() + 1;

	if (sgi != INVALID_SGI) {
		reg = (cpu | (sgi << XSCUGIC_SGIR_EL1_INITID_SHIFT));
		write_icc_asgi1r_el1(reg);
	}

	/* Clear FIQ */
	plat_ic_end_of_interrupt(id);

	return 0;
}

/**
 * pm_register_sgi() - PM register the IPI interrupt
 *
 * @sgi -  SGI number to be used for communication.
 * @return	On success, the initialization function must return 0.
 *		Any other return value will cause the framework to ignore
 *		the service
 *
 * Update the SGI number to be used.
 *
 */
int pm_register_sgi(unsigned int sgi_num)
{
	if (sgi != INVALID_SGI) {
		return -EBUSY;
	}

	if (sgi_num >= GICV3_MAX_SGI_TARGETS) {
		return -EINVAL;
	}

	sgi = sgi_num;
	return 0;
}

/**
 * pm_setup() - PM service setup
 *
 * @return	On success, the initialization function must return 0.
 *		Any other return value will cause the framework to ignore
 *		the service
 *
 * Initialization functions for Versal power management for
 * communicaton with PMC.
 *
 * Called from sip_svc_setup initialization function with the
 * rt_svc_init signature.
 */
int pm_setup(void)
{
	int status, ret = 0;

	status = pm_ipi_init(primary_proc);

	if (status < 0) {
		INFO("BL31: PM Service Init Failed, Error Code %d!\n", status);
		ret = status;
	} else {
		pm_up = true;
	}

	/*
	 * Enable IPI IRQ
	 * assume the rich OS is OK to handle callback IRQs now.
	 * Even if we were wrong, it would not enable the IRQ in
	 * the GIC.
	 */
	pm_ipi_irq_enable(primary_proc);

	ret = request_intr_type_el3(PLAT_VERSAL_IPI_IRQ, ipi_fiq_handler);
	if (ret) {
		WARN("BL31: registering IPI interrupt failed\n");
	}
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
	uint32_t security_flag = SECURE_FLAG;

	/* Handle case where PM wasn't initialized properly */
	if (!pm_up)
		SMC_RET1(handle, SMC_UNK);

	pm_arg[0] = (uint32_t)x1;
	pm_arg[1] = (uint32_t)(x1 >> 32);
	pm_arg[2] = (uint32_t)x2;
	pm_arg[3] = (uint32_t)(x2 >> 32);

	/*
	 * Mark BIT24 payload (i.e 1st bit of pm_arg[3] ) as non-secure (1)
	 * if smc called is non secure
	 */
	if (is_caller_non_secure(flags)) {
		security_flag = NON_SECURE_FLAG;
	}

	switch (smc_fid & FUNCID_NUM_MASK) {
	/* PM API Functions */
	case PM_SELF_SUSPEND:
		ret = pm_self_suspend(pm_arg[0], pm_arg[1], pm_arg[2],
				      pm_arg[3], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_FORCE_POWERDOWN:
		ret = pm_force_powerdown(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_REQ_SUSPEND:
		ret = pm_req_suspend(pm_arg[0], pm_arg[1], pm_arg[2],
				     pm_arg[3], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_ABORT_SUSPEND:
		ret = pm_abort_suspend(pm_arg[0], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_SYSTEM_SHUTDOWN:
		ret = pm_system_shutdown(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_REQ_WAKEUP:
		ret = pm_req_wakeup(pm_arg[0], pm_arg[1], pm_arg[2], pm_arg[3],
				    security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_SET_WAKEUP_SOURCE:
		ret = pm_set_wakeup_source(pm_arg[0], pm_arg[1], pm_arg[2],
					   security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_REQUEST_DEVICE:
		ret = pm_request_device(pm_arg[0], pm_arg[1], pm_arg[2],
					pm_arg[3], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_RELEASE_DEVICE:
		ret = pm_release_device(pm_arg[0], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_SET_REQUIREMENT:
		ret = pm_set_requirement(pm_arg[0], pm_arg[1], pm_arg[2],
					 pm_arg[3], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_GET_API_VERSION:
	{
		uint32_t api_version;

		ret = pm_get_api_version(&api_version, security_flag);
		SMC_RET1(handle, (uint64_t)PM_RET_SUCCESS |
				 ((uint64_t)api_version << 32));
	}

	case PM_GET_DEVICE_STATUS:
	{
		uint32_t buff[3];

		ret = pm_get_device_status(pm_arg[0], buff, security_flag);
		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)buff[0] << 32),
			 (uint64_t)buff[1] | ((uint64_t)buff[2] << 32));
	}

	case PM_RESET_ASSERT:
		ret = pm_reset_assert(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_RESET_GET_STATUS:
	{
		uint32_t reset_status;

		ret = pm_reset_get_status(pm_arg[0], &reset_status,
					  security_flag);
		SMC_RET1(handle, (uint64_t)ret |
			 ((uint64_t)reset_status << 32));
	}

	case PM_INIT_FINALIZE:
		ret = pm_init_finalize(security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_GET_CALLBACK_DATA:
	{
		uint32_t result[4] = {0};

		pm_get_callbackdata(result, ARRAY_SIZE(result), security_flag);
		SMC_RET2(handle,
			 (uint64_t)result[0] | ((uint64_t)result[1] << 32),
			 (uint64_t)result[2] | ((uint64_t)result[3] << 32));
	}

	case PM_PINCTRL_REQUEST:
		ret = pm_pinctrl_request(pm_arg[0], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PINCTRL_RELEASE:
		ret = pm_pinctrl_release(pm_arg[0], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PINCTRL_GET_FUNCTION:
	{
		uint32_t value = 0;

		ret = pm_pinctrl_get_function(pm_arg[0], &value, security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_PINCTRL_SET_FUNCTION:
		ret = pm_pinctrl_set_function(pm_arg[0], pm_arg[1],
					      security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PINCTRL_CONFIG_PARAM_GET:
	{
		uint32_t value;

		ret = pm_pinctrl_get_pin_param(pm_arg[0], pm_arg[1], &value,
					       security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_PINCTRL_CONFIG_PARAM_SET:
		ret = pm_pinctrl_set_pin_param(pm_arg[0], pm_arg[1], pm_arg[2],
					       security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_IOCTL:
	{
		uint32_t value;

		ret = pm_api_ioctl(pm_arg[0], pm_arg[1], pm_arg[2],
				   pm_arg[3], &value, security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_QUERY_DATA:
	{
		uint32_t data[8] = { 0 };

		ret = pm_query_data(pm_arg[0], pm_arg[1], pm_arg[2],
				      pm_arg[3], data, security_flag);

		SMC_RET2(handle, (uint64_t)ret  | ((uint64_t)data[0] << 32),
				 (uint64_t)data[1] | ((uint64_t)data[2] << 32));

	}
	case PM_CLOCK_ENABLE:
		ret = pm_clock_enable(pm_arg[0], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_CLOCK_DISABLE:
		ret = pm_clock_disable(pm_arg[0], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_CLOCK_GETSTATE:
	{
		uint32_t value;

		ret = pm_clock_get_state(pm_arg[0], &value, security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_CLOCK_SETDIVIDER:
		ret = pm_clock_set_divider(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_CLOCK_GETDIVIDER:
	{
		uint32_t value;

		ret = pm_clock_get_divider(pm_arg[0], &value, security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_CLOCK_SETPARENT:
		ret = pm_clock_set_parent(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_CLOCK_GETPARENT:
	{
		uint32_t value;

		ret = pm_clock_get_parent(pm_arg[0], &value, security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_CLOCK_GETRATE:
	{
		uint32_t rate[2] = { 0 };

		ret = pm_clock_get_rate(pm_arg[0], rate, security_flag);
		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)rate[0] << 32),
			 rate[1]);
	}

	case PM_PLL_SET_PARAMETER:
		ret = pm_pll_set_param(pm_arg[0], pm_arg[1], pm_arg[2],
				       security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PLL_GET_PARAMETER:
	{
		uint32_t value;

		ret = pm_pll_get_param(pm_arg[0], pm_arg[1], &value,
				       security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value << 32));
	}

	case PM_PLL_SET_MODE:
		ret = pm_pll_set_mode(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PLL_GET_MODE:
	{
		uint32_t mode;

		ret = pm_pll_get_mode(pm_arg[0], &mode, security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)mode << 32));
	}

	case PM_GET_TRUSTZONE_VERSION:
		SMC_RET1(handle, (uint64_t)PM_RET_SUCCESS |
			 ((uint64_t)VERSAL_TZ_VERSION << 32));

	case PM_GET_CHIPID:
	{
		uint32_t result[2];

		ret = pm_get_chipid(result, security_flag);
		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)result[0] << 32),
			 result[1]);
	}

	case PM_FEATURE_CHECK:
	{
		uint32_t version;

		ret = pm_feature_check(pm_arg[0], &version, security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)version << 32));
	}

	case PM_LOAD_PDI:
	{
		ret = pm_load_pdi(pm_arg[0], pm_arg[1], pm_arg[2],
				  security_flag);
		SMC_RET1(handle, (uint64_t)ret);
	}

	case PM_GET_OP_CHARACTERISTIC:
	{
		uint32_t result;

		ret = pm_get_op_characteristic(pm_arg[0], pm_arg[1], &result,
					       security_flag);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)result << 32));
	}

	case PM_SET_MAX_LATENCY:
	{
		ret = pm_set_max_latency(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (uint64_t)ret);
	}

	case PM_REGISTER_NOTIFIER:
	{
		ret = pm_register_notifier(pm_arg[0], pm_arg[1], pm_arg[2],
					   pm_arg[3], security_flag);
		SMC_RET1(handle, (uint64_t)ret);
	}

	default:
		WARN("Unimplemented PM Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}
