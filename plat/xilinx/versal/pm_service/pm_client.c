/*
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * APU specific definition of processors in the subsystem as well as functions
 * for getting information about and changing state of the APU.
 */

#include <assert.h>

#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#include <plat_ipi.h>
#include <platform_def.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_defs.h"
#include <versal_def.h>

#define UNDEFINED_CPUID		(~0)

DEFINE_BAKERY_LOCK(pm_client_secure_lock);

static const struct pm_ipi apu_ipi = {
	.local_ipi_id = IPI_LOCAL_ID,
	.remote_ipi_id = IPI_REMOTE_ID,
	.buffer_base = IPI_BUFFER_LOCAL_BASE,
};

/* Order in pm_procs_all array must match cpu ids */
static const struct pm_proc pm_procs_all[] = {
	{
		.node_id = XPM_DEVID_ACPU_0,
		.ipi = &apu_ipi,
		.pwrdn_mask = APU_0_PWRCTL_CPUPWRDWNREQ_MASK,
	},
	{
		.node_id = XPM_DEVID_ACPU_1,
		.ipi = &apu_ipi,
		.pwrdn_mask = APU_1_PWRCTL_CPUPWRDWNREQ_MASK,
	}
};

const struct pm_proc *primary_proc = &pm_procs_all[0];

/**
 * irq_to_pm_node_idx - Get PM node index corresponding to the interrupt number.
 * @irq: Interrupt number
 *
 * Return: PM node index corresponding to the specified interrupt.
 *
 */
enum pm_device_node_idx irq_to_pm_node_idx(uint32_t irq)
{
	enum pm_device_node_idx dev_idx = XPM_NODEIDX_DEV_MIN;

	assert(irq <= IRQ_MAX);

	switch (irq) {
	case 13:
		dev_idx = XPM_NODEIDX_DEV_GPIO;
		break;
	case 14:
		dev_idx = XPM_NODEIDX_DEV_I2C_0;
		break;
	case 15:
		dev_idx = XPM_NODEIDX_DEV_I2C_1;
		break;
	case 16:
		dev_idx = XPM_NODEIDX_DEV_SPI_0;
		break;
	case 17:
		dev_idx = XPM_NODEIDX_DEV_SPI_1;
		break;
	case 18:
		dev_idx = XPM_NODEIDX_DEV_UART_0;
		break;
	case 19:
		dev_idx = XPM_NODEIDX_DEV_UART_1;
		break;
	case 20:
		dev_idx = XPM_NODEIDX_DEV_CAN_FD_0;
		break;
	case 21:
		dev_idx = XPM_NODEIDX_DEV_CAN_FD_1;
		break;
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
		dev_idx = XPM_NODEIDX_DEV_USB_0;
		break;
	case 37:
	case 38:
	case 39:
		dev_idx = XPM_NODEIDX_DEV_TTC_0;
		break;
	case 40:
	case 41:
	case 42:
		dev_idx = XPM_NODEIDX_DEV_TTC_1;
		break;
	case 43:
	case 44:
	case 45:
		dev_idx = XPM_NODEIDX_DEV_TTC_2;
		break;
	case 46:
	case 47:
	case 48:
		dev_idx = XPM_NODEIDX_DEV_TTC_3;
		break;
	case 56:
	case 57:
		dev_idx = XPM_NODEIDX_DEV_GEM_0;
		break;
	case 58:
	case 59:
		dev_idx = XPM_NODEIDX_DEV_GEM_1;
		break;
	case 60:
		dev_idx = XPM_NODEIDX_DEV_ADMA_0;
		break;
	case 61:
		dev_idx = XPM_NODEIDX_DEV_ADMA_1;
		break;
	case 62:
		dev_idx = XPM_NODEIDX_DEV_ADMA_2;
		break;
	case 63:
		dev_idx = XPM_NODEIDX_DEV_ADMA_3;
		break;
	case 64:
		dev_idx = XPM_NODEIDX_DEV_ADMA_4;
		break;
	case 65:
		dev_idx = XPM_NODEIDX_DEV_ADMA_5;
		break;
	case 66:
		dev_idx = XPM_NODEIDX_DEV_ADMA_6;
		break;
	case 67:
		dev_idx = XPM_NODEIDX_DEV_ADMA_7;
		break;
	case 74:
		dev_idx = XPM_NODEIDX_DEV_USB_0;
		break;
	case 126:
	case 127:
		dev_idx = XPM_NODEIDX_DEV_SDIO_0;
		break;
	case 128:
	case 129:
		dev_idx = XPM_NODEIDX_DEV_SDIO_1;
		break;
	case 142:
		dev_idx = XPM_NODEIDX_DEV_RTC;
		break;
	default:
		dev_idx = XPM_NODEIDX_DEV_MIN;
		break;
	}

	return dev_idx;
}

/**
 * pm_client_suspend() - Client-specific suspend actions.
 * @proc: processor which need to suspend.
 * @state: desired suspend state.
 *
 * This function should contain any PU-specific actions
 * required prior to sending suspend request to PMU
 * Actions taken depend on the state system is suspending to.
 *
 */
void pm_client_suspend(const struct pm_proc *proc, uint32_t state)
{
	bakery_lock_get(&pm_client_secure_lock);

	if (state == PM_STATE_SUSPEND_TO_RAM) {
		pm_client_set_wakeup_sources((uint32_t)proc->node_id);
	}

	/* Set powerdown request */
	mmio_write_32(FPD_APU_PWRCTL, mmio_read_32(FPD_APU_PWRCTL) |
		      (uint32_t)proc->pwrdn_mask);

	bakery_lock_release(&pm_client_secure_lock);
}

/**
 * pm_client_abort_suspend() - Client-specific abort-suspend actions.
 *
 * This function should contain any PU-specific actions
 * required for aborting a prior suspend request.
 *
 */
void pm_client_abort_suspend(void)
{
	/* Enable interrupts at processor level (for current cpu) */
	gicv3_cpuif_enable(plat_my_core_pos());

	bakery_lock_get(&pm_client_secure_lock);

	/* Clear powerdown request */
	mmio_write_32(FPD_APU_PWRCTL, mmio_read_32(FPD_APU_PWRCTL) &
		      ~((uint32_t)primary_proc->pwrdn_mask));

	bakery_lock_release(&pm_client_secure_lock);
}

/**
 * pm_get_cpuid() - get the local cpu ID for a global node ID.
 * @nid: node id of the processor.
 *
 * Return: the cpu ID (starting from 0) for the subsystem.
 *
 */
static uint32_t pm_get_cpuid(uint32_t nid)
{
	for (size_t i = 0U; i < ARRAY_SIZE(pm_procs_all); i++) {
		if (pm_procs_all[i].node_id == nid) {
			return i;
		}
	}
	return UNDEFINED_CPUID;
}

/**
 * pm_client_wakeup() - Client-specific wakeup actions.
 * @proc: Processor which need to wakeup.
 *
 * This function should contain any PU-specific actions
 * required for waking up another APU core.
 *
 */
void pm_client_wakeup(const struct pm_proc *proc)
{
	uint32_t cpuid = pm_get_cpuid(proc->node_id);

	if (cpuid == UNDEFINED_CPUID) {
		return;
	}

	bakery_lock_get(&pm_client_secure_lock);

	/* clear powerdown bit for affected cpu */
	uint32_t val = mmio_read_32(FPD_APU_PWRCTL);
	val &= ~(proc->pwrdn_mask);
	mmio_write_32(FPD_APU_PWRCTL, val);

	bakery_lock_release(&pm_client_secure_lock);
}

/**
 * pm_get_proc() - returns pointer to the proc structure.
 * @cpuid: id of the cpu whose proc struct pointer should be returned.
 *
 * Return: pointer to a proc structure if proc is found, otherwise NULL.
 *
 */
const struct pm_proc *pm_get_proc(uint32_t cpuid)
{
	if (cpuid < ARRAY_SIZE(pm_procs_all)) {
		return &pm_procs_all[cpuid];
	}

	return NULL;
}
