/*
 * Copyright (c) 2013-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Top-level SMC handler for ZynqMP power management calls and
 * IPI setup functions for communication with PMU.
 */

#include <errno.h>

#include <common/runtime_svc.h>
#if ZYNQMP_WDT_RESTART
#include <arch_helpers.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#endif

#include <plat_private.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_defs.h"
#include "pm_ipi.h"

/* pm_up = !0 - UP, pm_up = 0 - DOWN */
static int32_t pm_up, ipi_irq_flag;

#if ZYNQMP_WDT_RESTART
static spinlock_t inc_lock;
static int active_cores = 0;
#endif

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

#if ZYNQMP_WDT_RESTART
/**
 * trigger_wdt_restart() - Trigger warm restart event to APU cores
 *
 * This function triggers SGI for all active APU CPUs. SGI handler then
 * power down CPU and call system reset.
 */
static void trigger_wdt_restart(void)
{
	uint32_t core_count = 0;
	uint32_t core_status[3];
	uint32_t target_cpu_list = 0;
	int i;

	for (i = 0; i < 4; i++) {
		pm_get_node_status(NODE_APU_0 + i, core_status);
		if (core_status[0] == 1) {
			core_count++;
			target_cpu_list |= (1 << i);
		}
	}

	spin_lock(&inc_lock);
	active_cores = core_count;
	spin_unlock(&inc_lock);

	INFO("Active Cores: %d\n", active_cores);

	for (i = PLATFORM_CORE_COUNT - 1; i >= 0; i--) {
		if (target_cpu_list & (1 << i)) {
			/* trigger SGI to active cores */
			plat_ic_raise_el3_sgi(ARM_IRQ_SEC_SGI_7, i);
		}
	}
}

/**
 * ttc_fiq_handler() - TTC Handler for timer event
 * @id         number of the highest priority pending interrupt of the type
 *             that this handler was registered for
 * @flags      security state, bit[0]
 * @handler    pointer to 'cpu_context' structure of the current CPU for the
 *             security state specified in the 'flags' parameter
 * @cookie     unused
 *
 * Function registered as INTR_TYPE_EL3 interrupt handler
 *
 * When WDT event is received in PMU, PMU needs to notify master to do cleanup
 * if required. PMU sets up timer and starts timer to overflow in zero time upon
 * WDT event. ATF handles this timer event and takes necessary action required
 * for warm restart.
 *
 * In presence of non-secure software layers (EL1/2) sets the interrupt
 * at registered entrance in GIC and informs that PMU responsed or demands
 * action.
 */
static uint64_t ttc_fiq_handler(uint32_t id, uint32_t flags, void *handle,
				void *cookie)
{
	INFO("BL31: Got TTC FIQ\n");

	plat_ic_end_of_interrupt(id);

	/* Clear TTC interrupt by reading interrupt register */
	mmio_read_32(TTC3_INTR_REGISTER_1);

	/* Disable the timer interrupts */
	mmio_write_32(TTC3_INTR_ENABLE_1, 0);

	trigger_wdt_restart();

	return 0;
}

/**
 * zynqmp_sgi7_irq() - Handler for SGI7 IRQ
 * @id         number of the highest priority pending interrupt of the type
 *             that this handler was registered for
 * @flags      security state, bit[0]
 * @handler    pointer to 'cpu_context' structure of the current CPU for the
 *             security state specified in the 'flags' parameter
 * @cookie     unused
 *
 * Function registered as INTR_TYPE_EL3 interrupt handler
 *
 * On receiving WDT event from PMU, ATF generates SGI7 to all running CPUs.
 * In response to SGI7 interrupt, each CPUs do clean up if required and last
 * running CPU calls system restart.
 */
static uint64_t __unused __dead2 zynqmp_sgi7_irq(uint32_t id, uint32_t flags,
						 void *handle, void *cookie)
{
	int i;
	uint32_t value;

	/* enter wfi and stay there */
	INFO("Entering wfi\n");

	spin_lock(&inc_lock);
	active_cores--;

	for (i = 0; i < 4; i++) {
		mmio_write_32(BASE_GICD_BASE + GICD_CPENDSGIR + 4 * i,
				0xffffffff);
	}

	spin_unlock(&inc_lock);

	if (active_cores == 0) {
		pm_mmio_read(PMU_GLOBAL_GEN_STORAGE4, &value);
		value = (value & RESTART_SCOPE_MASK) >> RESTART_SCOPE_SHIFT;
		pm_system_shutdown(PMF_SHUTDOWN_TYPE_RESET, value);
	}

	/* enter wfi and stay there */
	while (1)
		wfi();
}

/**
 * pm_wdt_restart_setup() - Setup warm restart interrupts
 *
 * This function sets up handler for SGI7 and TTC interrupts
 * used for warm restart.
 */
static int pm_wdt_restart_setup(void)
{
	int ret;

	/* register IRQ handler for SGI7 */
	ret = request_intr_type_el3(ARM_IRQ_SEC_SGI_7, zynqmp_sgi7_irq);
	if (ret) {
		WARN("BL31: registering SGI7 interrupt failed\n");
		goto err;
	}

	ret = request_intr_type_el3(IRQ_TTC3_1, ttc_fiq_handler);
	if (ret)
		WARN("BL31: registering TTC3 interrupt failed\n");

err:
	return ret;
}
#endif

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

	ret = pm_get_api_version(&pm_ctx.api_version);
	if (pm_ctx.api_version < PM_VERSION) {
		ERROR("BL31: Platform Management API version error. Expected: "
		      "v%d.%d - Found: v%d.%d\n", PM_VERSION_MAJOR,
		      PM_VERSION_MINOR, pm_ctx.api_version >> 16,
		      pm_ctx.api_version & 0xFFFF);
		return -EINVAL;
	}

#if ZYNQMP_WDT_RESTART
	status = pm_wdt_restart_setup();
	if (status)
		WARN("BL31: warm-restart setup failed\n");
#endif

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
	uint32_t payload[PAYLOAD_ARG_CNT];

	uint32_t pm_arg[4];
	uint32_t result[PAYLOAD_ARG_CNT];
	uint32_t api_id;

	/* Handle case where PM wasn't initialized properly */
	if (!pm_up)
		SMC_RET1(handle, SMC_UNK);

	pm_arg[0] = (uint32_t)x1;
	pm_arg[1] = (uint32_t)(x1 >> 32);
	pm_arg[2] = (uint32_t)x2;
	pm_arg[3] = (uint32_t)(x2 >> 32);
	pm_arg[4] = (uint32_t)x3;

	api_id = smc_fid & FUNCID_NUM_MASK;

	switch (api_id) {
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

	case PM_SET_REQUIREMENT:
		ret = pm_set_requirement(pm_arg[0], pm_arg[1], pm_arg[2],
					 pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_GET_API_VERSION:
		/* Check is PM API version already verified */
		if (pm_ctx.api_version >= PM_VERSION) {
			if (!ipi_irq_flag) {
				/*
				 * Enable IPI IRQ
				 * assume the rich OS is OK to handle callback IRQs now.
				 * Even if we were wrong, it would not enable the IRQ in
				 * the GIC.
				 */
				pm_ipi_irq_enable(primary_proc);
				ipi_irq_flag = 1;
			}
			SMC_RET1(handle, (uint64_t)PM_RET_SUCCESS |
				 ((uint64_t)pm_ctx.api_version << 32));
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

	case PM_SECURE_RSA_AES:
		ret = pm_secure_rsaaes(pm_arg[0], pm_arg[1], pm_arg[2],
				       pm_arg[3]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_GET_CALLBACK_DATA:
		pm_get_callbackdata(result, ARRAY_SIZE(result));
		SMC_RET2(handle,
			 (uint64_t)result[0] | ((uint64_t)result[1] << 32),
			 (uint64_t)result[2] | ((uint64_t)result[3] << 32));
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

		pm_query_data(pm_arg[0], pm_arg[1], pm_arg[2],
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

	case PM_SECURE_IMAGE:
	{
		ret = pm_secure_image(pm_arg[0], pm_arg[1], pm_arg[2],
				      pm_arg[3], &result[0]);
		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)result[0] << 32),
			 result[1]);
	}

	case PM_FPGA_READ:
	{
		uint32_t value;

		ret = pm_fpga_read(pm_arg[0], pm_arg[1], pm_arg[2], pm_arg[3],
				   &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_SECURE_AES:
	{
		uint32_t value;

		ret = pm_aes_engine(pm_arg[0], pm_arg[1], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_PLL_SET_PARAMETER:
		ret = pm_pll_set_parameter(pm_arg[0], pm_arg[1], pm_arg[2]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PLL_GET_PARAMETER:
	{
		uint32_t value;

		ret = pm_pll_get_parameter(pm_arg[0], pm_arg[1], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value << 32));
	}

	case PM_PLL_SET_MODE:
		ret = pm_pll_set_mode(pm_arg[0], pm_arg[1]);
		SMC_RET1(handle, (uint64_t)ret);

	case PM_PLL_GET_MODE:
	{
		uint32_t mode;

		ret = pm_pll_get_mode(pm_arg[0], &mode);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)mode << 32));
	}

	case PM_REGISTER_ACCESS:
	{
		uint32_t value;

		ret = pm_register_access(pm_arg[0], pm_arg[1], pm_arg[2],
					 pm_arg[3], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_EFUSE_ACCESS:
	{
		uint32_t value;

		ret = pm_efuse_access(pm_arg[0], pm_arg[1], &value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case PM_FPGA_GET_VERSION:
	case PM_FPGA_GET_FEATURE_LIST:
	{
		uint32_t ret_payload[PAYLOAD_ARG_CNT];

		PM_PACK_PAYLOAD5(payload, smc_fid & FUNCID_NUM_MASK,
				 pm_arg[0], pm_arg[1], pm_arg[2], pm_arg[3]);
		ret = pm_ipi_send_sync(primary_proc, payload, ret_payload, 3U);
		SMC_RET2(handle, (uint64_t)ret | (uint64_t)ret_payload[0] << 32,
			 (uint64_t)ret_payload[1] | (uint64_t)ret_payload[2] << 32);
	}

	case PM_FEATURE_CHECK:
	{
		uint32_t version;
		uint32_t bit_mask[2] = {0};

		ret = pm_feature_check(pm_arg[0], &version, bit_mask,
				       ARRAY_SIZE(bit_mask));
		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)version << 32),
			 (uint64_t)bit_mask[0] | ((uint64_t)bit_mask[1] << 32));
	}

	default:
		/* Send request to the PMU */
		PM_PACK_PAYLOAD6(payload, api_id, pm_arg[0], pm_arg[1],
				 pm_arg[2], pm_arg[3], pm_arg[4]);
		ret = pm_ipi_send_sync(primary_proc, payload, result,
				       PAYLOAD_ARG_CNT);
		SMC_RET2(handle, (uint64_t)ret | ((uint64_t)result[0] << 32),
			 (uint64_t)result[1] | ((uint64_t)result[2] << 32));
	}
}

/**
 * em_smc_handler() - SMC handler for EM-API calls coming from EL1/EL2.
 * @smc_fid - Function Identifier
 * @x1 - x4 - Arguments
 * @cookie  - Unused
 * @handler - Pointer to caller's context structure
 *
 * @return  - Unused
 *
 * Determines that smc_fid is valid and supported EM SMC Function ID from the
 * list of em_api_ids, otherwise completes the request with
 * the unknown SMC Function ID
 *
 * The SMC calls for EM service are forwarded from SIP Service SMC handler
 * function with rt_svc_handle signature
 */
uint64_t em_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
			uint64_t x4, void *cookie, void *handle, uint64_t flags)
{
	enum pm_ret_status ret;

	switch (smc_fid & FUNCID_NUM_MASK) {
	/* EM API Functions */
	case EM_SET_ACTION:
	{
		uint32_t value;

		ret = em_set_action(&value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case EM_REMOVE_ACTION:
	{
		uint32_t value;

		ret = em_remove_action(&value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	case EM_SEND_ERRORS:
	{
		uint32_t value;

		ret = em_send_errors(&value);
		SMC_RET1(handle, (uint64_t)ret | ((uint64_t)value) << 32);
	}

	default:
		WARN("Unimplemented EM Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}
