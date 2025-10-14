/*
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
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

#include <common/ep_info.h>
#include <common/runtime_svc.h>
#include <drivers/arm/gicv3.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <plat_private.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_ipi.h"
#include "pm_svc_main.h"

#define MODE				0x80000000U

#define INVALID_SGI    0xFFU
#define PM_INIT_SUSPEND_CB	(30U)
#define PM_NOTIFY_CB		(32U)
#define EVENT_CPU_PWRDWN	(4U)
#define MBOX_SGI_SHARED_IPI	(7U)

/**
 * upper_32_bits - return bits 32-63 of a number
 * @n: the number we're accessing
 */
#define upper_32_bits(n)	((uint32_t)((n) >> 32U))

/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n)	((uint32_t)((n) & 0xffffffffU))

/**
 * EXTRACT_SMC_ARGS - extracts 32-bit payloads from 64-bit SMC arguments
 * @pm_arg: array of 32-bit payloads
 * @x: array of 64-bit SMC arguments
 */
#define EXTRACT_ARGS(pm_arg, x)						\
	for (uint32_t i = 0U; i < (PAYLOAD_ARG_CNT - 1U); i++) {	\
		if ((i % 2U) != 0U) {					\
			pm_arg[i] = lower_32_bits(x[(i / 2U) + 1U]);	\
		} else {						\
			pm_arg[i] = upper_32_bits(x[i / 2U]);		\
		}							\
	}

/* 1 sec of wait timeout for secondary core down */
#define PWRDWN_WAIT_TIMEOUT	(1000U)

/* pm_up = true - UP, pm_up = false - DOWN */
static bool pm_up;
static uint32_t sgi = (uint32_t)INVALID_SGI;
static bool pwrdwn_req_received;

bool pm_pwrdwn_req_status(void)
{
	return pwrdwn_req_received;
}

static void notify_os(void)
{
	plat_ic_raise_ns_sgi((int)sgi, read_mpidr_el1());
}

static uint64_t cpu_pwrdwn_req_handler(uint32_t id, uint32_t flags,
				       void *handle, void *cookie)
{
	(void)id;
	(void)flags;
	(void)handle;
	(void)cookie;
	uint32_t cpu_id = plat_my_core_pos();

	VERBOSE("Powering down CPU %d\n", cpu_id);

	/* Deactivate CPU power down SGI */
	plat_ic_end_of_interrupt(CPU_PWR_DOWN_REQ_INTR);

	return (uint64_t)psci_cpu_off();
}

/**
 * raise_pwr_down_interrupt() - Callback function to raise SGI.
 * @mpidr: MPIDR for the target CPU.
 *
 * Raise SGI interrupt to trigger the CPU power down sequence on all the
 * online secondary cores.
 */
static void raise_pwr_down_interrupt(u_register_t mpidr)
{
	plat_ic_raise_el3_sgi((int)CPU_PWR_DOWN_REQ_INTR, mpidr);
}

void request_cpu_pwrdwn(void)
{
	int ret;

	VERBOSE("CPU power down request received\n");

	/* Send powerdown request to online secondary core(s) */
	ret = psci_stop_other_cores(plat_my_core_pos(), PWRDWN_WAIT_TIMEOUT,
				    raise_pwr_down_interrupt);
	if (ret != PSCI_E_SUCCESS) {
		ERROR("Failed to powerdown secondary core(s)\n");
	}

	/* Clear IPI IRQ */
	pm_ipi_irq_clear(primary_proc);

	/* Deactivate IPI IRQ */
	plat_ic_end_of_interrupt(PLAT_VERSAL_IPI_IRQ);
}

static uint64_t ipi_fiq_handler(uint32_t id, uint32_t flags, void *handle,
				void *cookie)
{
	(void)flags;
	(void)handle;
	(void)cookie;
	uint32_t payload[4] = {0};
	enum pm_ret_status ret;
	uint32_t ipi_status, i;

	VERBOSE("Received IPI FIQ from firmware\n");

	console_flush();
	(void)plat_ic_acknowledge_interrupt();

	/* Check status register for each IPI except PMC */
	for (i = IPI_ID_APU; i <= IPI_ID_5; i++) {
		ipi_status = ipi_mb_enquire_status(IPI_ID_APU, i);

		/* If any agent other than PMC has generated IPI FIQ then send SGI to mbox driver */
		if ((ipi_status & IPI_MB_STATUS_RECV_PENDING) != 0U) {
			plat_ic_raise_ns_sgi((int)MBOX_SGI_SHARED_IPI, read_mpidr_el1());
			break;
		}
	}

	/* If PMC has not generated interrupt then end ISR */
	ipi_status = ipi_mb_enquire_status(IPI_ID_APU, IPI_ID_PMC);
	if ((ipi_status & IPI_MB_STATUS_RECV_PENDING) == 0U) {
		plat_ic_end_of_interrupt(id);
		goto exit_label;
	}

	/* Handle PMC case */
	ret = pm_get_callbackdata(payload, ARRAY_SIZE(payload), 0, 0);
	if (ret != PM_RET_SUCCESS) {
		payload[0] = (uint32_t)ret;
	}

	switch (payload[0]) {
	case PM_INIT_SUSPEND_CB:
		if (sgi != INVALID_SGI) {
			notify_os();
		}
		break;
	case PM_NOTIFY_CB:
		if (sgi != INVALID_SGI) {
			if ((payload[2] == EVENT_CPU_PWRDWN) &&
			    (NODECLASS(payload[1]) == (uint32_t)XPM_NODECLASS_DEVICE)) {
				if (pwrdwn_req_received) {
					pwrdwn_req_received = false;
					request_cpu_pwrdwn();
					(void)psci_cpu_off();
					break;
				} else {
					pwrdwn_req_received = true;
				}
			}
			notify_os();
		} else {
			if ((payload[2] == EVENT_CPU_PWRDWN) &&
			    (NODECLASS(payload[1]) == (uint32_t)XPM_NODECLASS_DEVICE)) {
				request_cpu_pwrdwn();
				(void)psci_cpu_off();
			}
		}
		break;
	case (uint32_t)PM_RET_ERROR_INVALID_CRC:
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

exit_label:
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
	int32_t ret = 0;

	if (reset == 1U) {
		sgi = INVALID_SGI;
	} else if (sgi != INVALID_SGI) {
		ret = -EBUSY;
	} else if (sgi_num >= GICV3_MAX_SGI_TARGETS) {
		ret = -EINVAL;
	} else {
		sgi = (uint32_t)sgi_num;
	}

	return ret;
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
	pwrdwn_req_received = false;

	/* register SGI handler for CPU power down request */
	ret = request_intr_type_el3(CPU_PWR_DOWN_REQ_INTR, cpu_pwrdwn_req_handler);
	if (ret != 0) {
		WARN("BL31: registering SGI interrupt failed\n");
	}

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

	/* Register for idle callback during force power down/restart */
	ret = (int32_t)pm_register_notifier(primary_proc->node_id, EVENT_CPU_PWRDWN,
					    0x0U, 0x1U, SECURE);
	if (ret != 0) {
		WARN("BL31: registering idle callback for restart/force power down failed\n");
	}

	return ret;
}

/**
 * eemi_for_compatibility() - EEMI calls handler for deprecated calls.
 * @api_id: identifier for the API being called.
 * @pm_arg: pointer to the argument data for the API call.
 * @handle: Pointer to caller's context structure.
 * @security_flag: SECURE or NON_SECURE.
 *
 * Return: If EEMI API found then, uintptr_t type address, else 0.
 *
 * Some EEMI API's use case needs to be changed in Linux driver, so they
 * can take advantage of common EEMI handler in TF-A. As of now the old
 * implementation of these APIs are required to maintain backward compatibility
 * until their use case in linux driver changes.
 *
 */
static uintptr_t eemi_for_compatibility(uint32_t api_id, const uint32_t *pm_arg,
					void *handle, uint32_t security_flag)
{
	enum pm_ret_status ret;

	switch (api_id) {

	case (uint32_t)PM_FEATURE_CHECK:
	{
		uint32_t result[RET_PAYLOAD_ARG_CNT] = {0U};

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
 * @security_flag: SECURE or NON_SECURE.
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
static uintptr_t eemi_psci_debugfs_handler(uint32_t api_id, const uint32_t *pm_arg,
					   void *handle, uint32_t security_flag)
{
	enum pm_ret_status ret;

	switch (api_id) {

	case (uint32_t)PM_SELF_SUSPEND:
		ret = pm_self_suspend(pm_arg[0], pm_arg[1], pm_arg[2],
				      pm_arg[3], security_flag);
		SMC_RET1(handle, (u_register_t)ret);

	case (uint32_t)PM_FORCE_POWERDOWN:
		ret = pm_force_powerdown(pm_arg[0], (uint8_t)pm_arg[1], security_flag);
		SMC_RET1(handle, (u_register_t)ret);

	case (uint32_t)PM_SYSTEM_SHUTDOWN:
		ret = pm_system_shutdown(pm_arg[0], pm_arg[1], security_flag);
		SMC_RET1(handle, (u_register_t)ret);

	default:
		return (uintptr_t)0;
	}
}

/**
 * tfa_clear_pm_state() - Reset TF-A-specific PM state.
 *
 * This function resets TF-A-specific state that may have been modified,
 * such as during a kexec-based kernel reload. It resets the SGI number
 * and the shutdown scope to its default value.
 */
static enum pm_ret_status tfa_clear_pm_state(void)
{
	/* Reset SGI number to default value(-1). */
	sgi = (uint32_t)INVALID_SGI;

	/* Reset the shutdown scope to its default value(system). */
	return pm_system_shutdown(XPM_SHUTDOWN_TYPE_SETSCOPE_ONLY, XPM_SHUTDOWN_SUBTYPE_RST_SYSTEM,
				  0U);
}

/**
 * TF_A_specific_handler() - SMC handler for TF-A specific functionality.
 * @api_id: identifier for the API being called.
 * @pm_arg: pointer to the argument data for the API call.
 * @handle: Pointer to caller's context structure.
 * @security_flag: SECURE or NON_SECURE.
 *
 * These EEMI calls performs functionality that does not require
 * IPI transaction. The handler ends in TF-A and returns requested data to
 * kernel from TF-A.
 *
 * Return: If TF-A specific API found then, uintptr_t type address, else 0
 *
 */
static uintptr_t TF_A_specific_handler(uint32_t api_id, const uint32_t *pm_arg,
				       void *handle, uint32_t security_flag)
{
	switch (api_id) {

	case TF_A_FEATURE_CHECK:
	{
		enum pm_ret_status ret;
		uint32_t result[PAYLOAD_ARG_CNT] = {0U};

		ret = tfa_api_feature_check(pm_arg[0], result);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)result[0] << 32U));
	}

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
		if (ret != PM_RET_SUCCESS) {
			result[0] = (uint32_t)ret;
		}

		SMC_RET2(handle,
			(uint64_t)result[0] | ((uint64_t)result[1] << 32U),
			(uint64_t)result[2] | ((uint64_t)result[3] << 32U));
	}

	case PM_GET_TRUSTZONE_VERSION:
		SMC_RET1(handle, (uint64_t)PM_RET_SUCCESS |
			 ((uint64_t)TZ_VERSION << 32U));

	case TF_A_CLEAR_PM_STATE:
	{
		enum pm_ret_status ret;

		ret = tfa_clear_pm_state();

		SMC_RET1(handle, (uint64_t)ret);
	}

	default:
		return (uintptr_t)0;
	}
}

/**
 * eemi_handler() - Prepare EEMI payload and perform IPI transaction.
 * @api_id: identifier for the API being called.
 * @pm_arg: pointer to the argument data for the API call.
 * @handle: Pointer to caller's context structure.
 * @security_flag: SECURE or NON_SECURE.
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
static uintptr_t eemi_handler(uint32_t api_id, const uint32_t *pm_arg,
			      void *handle, uint32_t security_flag)
{
	enum pm_ret_status ret;
	uint32_t buf[RET_PAYLOAD_ARG_CNT] = {0};

	ret = pm_handle_eemi_call(security_flag, api_id, pm_arg[0], pm_arg[1],
				  pm_arg[2], pm_arg[3], pm_arg[4], buf);
	/*
	 * Two IOCTLs, to get clock name and pinctrl name of pm_query_data API
	 * receives 5 words of respoonse from firmware. Currently linux driver can
	 * receive only 4 words from TF-A. So, this needs to be handled separately
	 * than other eemi calls.
	 */
	if (api_id == (uint32_t)PM_QUERY_DATA) {
		if (((pm_arg[0] == (uint32_t)XPM_QID_CLOCK_GET_NAME) ||
		    (pm_arg[0] == (uint32_t)XPM_QID_PINCTRL_GET_FUNCTION_NAME)) &&
		    (ret == PM_RET_SUCCESS)) {
			SMC_RET2(handle, (uint64_t)buf[0] | ((uint64_t)buf[1] << 32U),
				(uint64_t)buf[2] | ((uint64_t)buf[3] << 32U));
		}
	}

	SMC_RET2(handle, (uint64_t)ret | ((uint64_t)buf[0] << 32U),
		 (uint64_t)buf[1] | ((uint64_t)buf[2] << 32U));
}

/**
 * eemi_api_handler() - Prepare EEMI payload and perform IPI transaction.
 * @api_id: identifier for the API being called.
 * @pm_arg: pointer to the argument data for the API call.
 * @handle: Pointer to caller's context structure.
 * @security_flag: SECURE or NON_SECURE.
 *
 * EEMI - Embedded Energy Management Interface is AMD-Xilinx proprietary
 * protocol to allow communication between power management controller and
 * different processing clusters.
 *
 * This handler prepares EEMI protocol payload received from kernel and performs
 * IPI transaction.
 *
 * Return: If EEMI API found then, uintptr_t type address, else 0
 */
static uintptr_t eemi_api_handler(uint32_t api_id, const uint32_t *pm_arg,
				  void *handle, uint32_t security_flag)
{
	enum pm_ret_status ret;
	uint32_t buf[RET_PAYLOAD_ARG_CNT] = {0U};
	uint32_t payload[PAYLOAD_ARG_CNT] = {0U};
	uint32_t module_id;

	module_id = (api_id & MODULE_ID_MASK) >> 8U;

	PM_PACK_PAYLOAD7(payload, module_id, security_flag, api_id,
			 pm_arg[0], pm_arg[1], pm_arg[2], pm_arg[3],
			 pm_arg[4], pm_arg[5]);

	ret = pm_ipi_send_sync(primary_proc, payload, (uint32_t *)buf,
			       RET_PAYLOAD_ARG_CNT);

	SMC_RET4(handle, (uint64_t)ret | ((uint64_t)buf[0] << 32U),
		 (uint64_t)buf[1] | ((uint64_t)buf[2] << 32U),
		 (uint64_t)buf[3] | ((uint64_t)buf[4] << 32U),
		 (uint64_t)buf[5]);
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
 * @flags: SECURE or NON_SECURE.
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
	(void)cookie;
	uintptr_t ret;
	uint32_t pm_arg[PAYLOAD_ARG_CNT] = {0};
	uint32_t security_flag = NON_SECURE;
	uint32_t api_id;
	bool status = false, status_tmp = false;
	const uint64_t x[4] = {x1, x2, x3, x4};

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
		security_flag = SECURE;
	}

	if ((smc_fid & FUNCID_NUM_MASK) == PASS_THROUGH_FW_CMD_ID) {
		api_id = lower_32_bits(x[0]);

		EXTRACT_ARGS(pm_arg, x);

		return eemi_api_handler(api_id, pm_arg, handle, security_flag);
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

	ret = eemi_psci_debugfs_handler(api_id, pm_arg, handle,
					(uint32_t)flags);
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
