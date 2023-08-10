/*
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Top-level SMC handler for Versal power management calls and
 * IPI setup functions for communication with PMC.
 */

#include <errno.h>
#include <stdbool.h>

#include "../drivers/arm/gic/v3/gicv3_private.h"

#include <common/runtime_svc.h>
#include <drivers/arm/gicv3.h>
#include <plat/common/platform.h>

#include <plat_private.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_ipi.h"
#include "pm_svc_main.h"

#define MODE				0x80000000U

#define XSCUGIC_SGIR_EL1_INITID_SHIFT    24U
#define INVALID_SGI    0xFFU
#define PM_INIT_SUSPEND_CB	(30U)
#define PM_NOTIFY_CB		(32U)
DEFINE_RENAME_SYSREG_RW_FUNCS(icc_asgi1r_el1, S3_0_C12_C11_6)

/* pm_up = true - UP, pm_up = false - DOWN */
static bool pm_up;
static uint32_t sgi = (uint32_t)INVALID_SGI;

static void notify_os(void)
{
	int32_t cpu;
	uint32_t reg;

	cpu = plat_my_core_pos() + 1U;

	reg = (cpu | (sgi << XSCUGIC_SGIR_EL1_INITID_SHIFT));
	write_icc_asgi1r_el1(reg);
}

static uint64_t ipi_fiq_handler(uint32_t id, uint32_t flags, void *handle,
				void *cookie)
{
	uint32_t payload[4] = {0};
	enum pm_ret_status ret;

	VERBOSE("Received IPI FIQ from firmware\n");

	(void)plat_ic_acknowledge_interrupt();

	ret = pm_get_callbackdata(payload, ARRAY_SIZE(payload), 0, 0);
	if (ret != PM_RET_SUCCESS) {
		payload[0] = ret;
	}

	switch (payload[0]) {
	case PM_INIT_SUSPEND_CB:
	case PM_NOTIFY_CB:
		if (sgi != INVALID_SGI) {
			notify_os();
		}
		break;
	case PM_RET_ERROR_INVALID_CRC:
		pm_ipi_irq_clear(primary_proc);
		WARN("Invalid CRC in the payload\n");
		break;

	default:
		pm_ipi_irq_clear(primary_proc);
		WARN("Invalid IPI payload\n");
		break;
	}

	/* Clear FIQ */
	plat_ic_end_of_interrupt(id);

	return 0;
}

/**
 * pm_register_sgi() - PM register the IPI interrupt.
 * @sgi_num: SGI number to be used for communication.
 * @reset: Reset to invalid SGI when reset=1.
 *
 * Return: On success, the initialization function must return 0.
 *         Any other return value will cause the framework to ignore
 *         the service.
 *
 * Update the SGI number to be used.
 *
 */
int32_t pm_register_sgi(uint32_t sgi_num, uint32_t reset)
{
	if (reset == 1U) {
		sgi = INVALID_SGI;
		return 0;
	}

	if (sgi != INVALID_SGI) {
		return -EBUSY;
	}

	if (sgi_num >= GICV3_MAX_SGI_TARGETS) {
		return -EINVAL;
	}

	sgi = (uint32_t)sgi_num;
	return 0;
}

/**
 * pm_setup() - PM service setup.
 *
 * Return: On success, the initialization function must return 0.
 *         Any other return value will cause the framework to ignore
 *         the service.
 *
 * Initialization functions for Versal power management for
 * communicaton with PMC.
 *
 * Called from sip_svc_setup initialization function with the
 * rt_svc_init signature.
 *
 */
int32_t pm_setup(void)
{
	int32_t ret = 0;

	pm_ipi_init(primary_proc);
	pm_up = true;

	/*
	 * Enable IPI IRQ
	 * assume the rich OS is OK to handle callback IRQs now.
	 * Even if we were wrong, it would not enable the IRQ in
	 * the GIC.
	 */
	pm_ipi_irq_enable(primary_proc);

	ret = request_intr_type_el3(PLAT_VERSAL_IPI_IRQ, ipi_fiq_handler);
	if (ret != 0) {
		WARN("BL31: registering IPI interrupt failed\n");
	}

	gicd_write_irouter(gicv3_driver_data->gicd_base, PLAT_VERSAL_IPI_IRQ, MODE);
	return ret;
}

/**
 * eemi_for_compatibility() - EEMI calls handler for deprecated calls.
 * @api_id: identifier for the API being called.
 * @pm_arg: pointer to the argument data for the API call.
 * @handle: Pointer to caller's context structure.
 * @security_flag: SECURE_FLAG or NON_SECURE_FLAG.
 *
 * Return: If EEMI API found then, uintptr_t type address, else 0.
 *
 * Some EEMI API's use case needs to be changed in Linux driver, so they
 * can take advantage of common EEMI handler in TF-A. As of now the old
 * implementation of these APIs are required to maintain backward compatibility
 * until their use case in linux driver changes.
 *
 */
static uintptr_t eemi_for_compatibility(uint32_t api_id, uint32_t *pm_arg,
					void *handle, uint32_t security_flag)
{
	enum pm_ret_status ret;

	switch (api_id) {

	case (uint32_t)PM_IOCTL:
	{
		uint32_t value = 0U;

		ret = pm_api_ioctl(pm_arg[0], pm_arg[1], pm_arg[2],
				   pm_arg[3], pm_arg[4],
				   &value, security_flag);
		if (ret == PM_RET_ERROR_NOTSUPPORTED)
			return (uintptr_t)0;

		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32U);
	}

	case (uint32_t)PM_QUERY_DATA:
	{
		uint32_t data[PAYLOAD_ARG_CNT] = { 0 };

		ret = pm_query_data(pm_arg[0], pm_arg[1], pm_arg[2],
				    pm_arg[3], data, security_flag);

		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)data[0] << 32U),
			 (uint64_t)data[1] | ((uint64_t)data[2] << 32U));
	}

	case (uint32_t)PM_FEATURE_CHECK:
	{
		uint32_t result[PAYLOAD_ARG_CNT] = {0U};

		ret = pm_feature_check(pm_arg[0], result, security_flag);
		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)result[0] << 32U),
			 (uint64_t)result[1] | ((uint64_t)result[2] << 32U));
	}

	case PM_LOAD_PDI:
	{
		ret = pm_load_pdi(pm_arg[0], pm_arg[1], pm_arg[2],
				  security_flag);
		SMC_RET1(handle, (uint64_t)ret);
	}

	default:
		return (uintptr_t)0;
	}
}

/**
 * eemi_psci_debugfs_handler() - EEMI API invoked from PSCI.
 * @api_id: identifier for the API being called.
 * @pm_arg: pointer to the argument data for the API call.
 * @handle: Pointer to caller's context structure.
 * @security_flag: SECURE_FLAG or NON_SECURE_FLAG.
 *
 * These EEMI APIs performs CPU specific power management tasks.
 * These EEMI APIs are invoked either from PSCI or from debugfs in kernel.
 * These calls require CPU specific processing before sending IPI request to
 * Platform Management Controller. For example enable/disable CPU specific
 * interrupts. This requires separate handler for these calls and may not be
 * handled using common eemi handler.
 *
 * Return: If EEMI API found then, uintptr_t type address, else 0.
 *
 */
static uintptr_t eemi_psci_debugfs_handler(uint32_t api_id, uint32_t *pm_arg,
					   void *handle, uint32_t security_flag)
{
	enum pm_ret_status ret;

	switch (api_id) {

	case (uint32_t)PM_SELF_SUSPEND:
		ret = pm_self_suspend(pm_arg[0], pm_arg[1], pm_arg[2],
				      pm_arg[3], security_flag);
		SMC_RET1(handle, (u_register_t)ret);

	case (uint32_t)PM_FORCE_POWERDOWN:
		ret = pm_force_powerdown(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (u_register_t)ret);

	case (uint32_t)PM_REQ_SUSPEND:
		ret = pm_req_suspend(pm_arg[0], pm_arg[1], pm_arg[2],
				     pm_arg[3], security_flag);
		SMC_RET1(handle, (u_register_t)ret);

	case (uint32_t)PM_ABORT_SUSPEND:
		ret = pm_abort_suspend(pm_arg[0], security_flag);
		SMC_RET1(handle, (u_register_t)ret);

	case (uint32_t)PM_SYSTEM_SHUTDOWN:
		ret = pm_system_shutdown(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (u_register_t)ret);

	default:
		return (uintptr_t)0;
	}
}

/**
 * TF_A_specific_handler() - SMC handler for TF-A specific functionality.
 * @api_id: identifier for the API being called.
 * @pm_arg: pointer to the argument data for the API call.
 * @handle: Pointer to caller's context structure.
 * @security_flag: SECURE_FLAG or NON_SECURE_FLAG.
 *
 * These EEMI calls performs functionality that does not require
 * IPI transaction. The handler ends in TF-A and returns requested data to
 * kernel from TF-A.
 *
 * Return: If TF-A specific API found then, uintptr_t type address, else 0
 *
 */
static uintptr_t TF_A_specific_handler(uint32_t api_id, uint32_t *pm_arg,
				       void *handle, uint32_t security_flag)
{
	switch (api_id) {

	case TF_A_PM_REGISTER_SGI:
	{
		int32_t ret;

		ret = pm_register_sgi(pm_arg[0], pm_arg[1]);
		if (ret != 0) {
			SMC_RET1(handle, (uint32_t)PM_RET_ERROR_ARGS);
		}

		SMC_RET1(handle, (uint32_t)PM_RET_SUCCESS);
	}

	case PM_GET_CALLBACK_DATA:
	{
		uint32_t result[4] = {0};
		enum pm_ret_status ret;

		ret = pm_get_callbackdata(result, ARRAY_SIZE(result), security_flag, 1U);
		if (ret != 0) {
			result[0] = ret;
		}

		SMC_RET2(handle,
			(uint64_t)result[0] | ((uint64_t)result[1] << 32U),
			(uint64_t)result[2] | ((uint64_t)result[3] << 32U));
	}

	case PM_GET_TRUSTZONE_VERSION:
		SMC_RET1(handle, (uint64_t)PM_RET_SUCCESS |
			 ((uint64_t)TZ_VERSION << 32U));

	default:
		return (uintptr_t)0;
	}
}

/**
 * eemi_handler() - Prepare EEMI payload and perform IPI transaction.
 * @api_id: identifier for the API being called.
 * @pm_arg: pointer to the argument data for the API call.
 * @handle: Pointer to caller's context structure.
 * @security_flag: SECURE_FLAG or NON_SECURE_FLAG.
 *
 * EEMI - Embedded Energy Management Interface is Xilinx proprietary protocol
 * to allow communication between power management controller and different
 * processing clusters.
 *
 * This handler prepares EEMI protocol payload received from kernel and performs
 * IPI transaction.
 *
 * Return: If EEMI API found then, uintptr_t type address, else 0
 *
 */
static uintptr_t eemi_handler(uint32_t api_id, uint32_t *pm_arg,
			      void *handle, uint32_t security_flag)
{
	enum pm_ret_status ret;
	uint32_t buf[PAYLOAD_ARG_CNT] = {0};

	ret = pm_handle_eemi_call(security_flag, api_id, pm_arg[0], pm_arg[1],
				  pm_arg[2], pm_arg[3], pm_arg[4],
				  (uint64_t *)buf);
	/*
	 * Two IOCTLs, to get clock name and pinctrl name of pm_query_data API
	 * receives 5 words of respoonse from firmware. Currently linux driver can
	 * receive only 4 words from TF-A. So, this needs to be handled separately
	 * than other eemi calls.
	 */
	if (api_id == (uint32_t)PM_QUERY_DATA) {
		if ((pm_arg[0] == XPM_QID_CLOCK_GET_NAME ||
		    pm_arg[0] == XPM_QID_PINCTRL_GET_FUNCTION_NAME) &&
		    ret == PM_RET_SUCCESS) {
			SMC_RET2(handle, (uint64_t)buf[0] | ((uint64_t)buf[1] << 32U),
				(uint64_t)buf[2] | ((uint64_t)buf[3] << 32U));
		}
	}

	SMC_RET2(handle, (uint64_t)ret | ((uint64_t)buf[0] << 32U),
		 (uint64_t)buf[1] | ((uint64_t)buf[2] << 32U));
}

/**
 * pm_smc_handler() - SMC handler for PM-API calls coming from EL1/EL2.
 * @smc_fid: Function Identifier.
 * @x1: SMC64 Arguments from kernel.
 * @x2: SMC64 Arguments from kernel.
 * @x3: SMC64 Arguments from kernel (upper 32-bits).
 * @x4: Unused.
 * @cookie: Unused.
 * @handle: Pointer to caller's context structure.
 * @flags: SECURE_FLAG or NON_SECURE_FLAG.
 *
 * Return: Unused.
 *
 * Determines that smc_fid is valid and supported PM SMC Function ID from the
 * list of pm_api_ids, otherwise completes the request with
 * the unknown SMC Function ID.
 *
 * The SMC calls for PM service are forwarded from SIP Service SMC handler
 * function with rt_svc_handle signature.
 *
 */
uint64_t pm_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
			uint64_t x4, const void *cookie, void *handle, uint64_t flags)
{
	uintptr_t ret;
	uint32_t pm_arg[PAYLOAD_ARG_CNT] = {0};
	uint32_t security_flag = NON_SECURE_FLAG;
	uint32_t api_id;
	bool status = false, status_tmp = false;

	/* Handle case where PM wasn't initialized properly */
	if (pm_up == false) {
		SMC_RET1(handle, SMC_UNK);
	}

	/*
	 * Mark BIT24 payload (i.e 1st bit of pm_arg[3] ) as secure (0)
	 * if smc called is secure
	 *
	 * Add redundant macro call to immune the code from glitches
	 */
	SECURE_REDUNDANT_CALL(status, status_tmp, is_caller_secure, flags);
	if ((status != false) && (status_tmp != false)) {
		security_flag = SECURE_FLAG;
	}

	pm_arg[0] = (uint32_t)x1;
	pm_arg[1] = (uint32_t)(x1 >> 32U);
	pm_arg[2] = (uint32_t)x2;
	pm_arg[3] = (uint32_t)(x2 >> 32U);
	pm_arg[4] = (uint32_t)x3;
	(void)(x4);
	api_id = smc_fid & FUNCID_NUM_MASK;

	ret = eemi_for_compatibility(api_id, pm_arg, handle, security_flag);
	if (ret != (uintptr_t)0) {
		return ret;
	}

	ret = eemi_psci_debugfs_handler(api_id, pm_arg, handle, flags);
	if (ret !=  (uintptr_t)0) {
		return ret;
	}

	ret = TF_A_specific_handler(api_id, pm_arg, handle, security_flag);
	if (ret !=  (uintptr_t)0) {
		return ret;
	}

	ret = eemi_handler(api_id, pm_arg, handle, security_flag);

	return ret;
}
