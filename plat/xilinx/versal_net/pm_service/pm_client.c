/*
 * Copyright (c) 2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * APU specific definition of processors in the subsystem as well as functions
 * for getting information about and changing state of the APU.
 */

#include <assert.h>

#include <drivers/arm/gic.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#include <plat_ipi.h>
#include <platform_def.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include <versal_net_def.h>

#define UNDEFINED_CPUID		(~0U)

DEFINE_RENAME_SYSREG_RW_FUNCS(cpu_pwrctrl_val, S3_0_C15_C2_7)

spinlock_t pm_client_secure_lock;
static inline void pm_client_lock_get(void)
{
	spin_lock(&pm_client_secure_lock);
}

static inline void pm_client_lock_release(void)
{
	spin_unlock(&pm_client_secure_lock);
}

static const struct pm_ipi apu_ipi = {
	.local_ipi_id = IPI_LOCAL_ID,
	.remote_ipi_id = IPI_REMOTE_ID,
	.buffer_base = IPI_BUFFER_LOCAL_BASE,
};

/* Order in pm_procs_all array must match cpu ids */
static const struct pm_proc pm_procs_all[] = {
	{
		.node_id = PM_DEV_CLUSTER0_ACPU_0,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER0_ACPU_1,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER0_ACPU_2,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER0_ACPU_3,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER1_ACPU_0,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER1_ACPU_1,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER1_ACPU_2,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER1_ACPU_3,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER2_ACPU_0,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER2_ACPU_1,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER2_ACPU_2,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER2_ACPU_3,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER3_ACPU_0,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER3_ACPU_1,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER3_ACPU_2,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	},
	{
		.node_id = PM_DEV_CLUSTER3_ACPU_3,
		.ipi = &apu_ipi,
		.pwrdn_mask = 0,
	}
};

const struct pm_proc *primary_proc = &pm_procs_all[0];

/**
 * pm_get_proc() - returns pointer to the proc structure.
 * @cpuid: id of the cpu whose proc struct pointer should be returned.
 *
 * Return: Pointer to a proc structure if proc is found, otherwise NULL.
 *
 */
const struct pm_proc *pm_get_proc(uint32_t cpuid)
{
	if (cpuid < ARRAY_SIZE(pm_procs_all)) {
		return &pm_procs_all[cpuid];
	}

	NOTICE("ERROR: cpuid: %d proc NULL\n", cpuid);
	return NULL;
}

/**
 * irq_to_pm_node_idx - Get PM node index corresponding to the interrupt number.
 * @irq: Interrupt number.
 *
 * Return: PM node index corresponding to the specified interrupt.
 *
 */
enum pm_device_node_idx irq_to_pm_node_idx(uint32_t irq)
{
	enum pm_device_node_idx dev_idx = XPM_NODEIDX_DEV_MIN;

	assert(irq <= IRQ_MAX);

	switch (irq) {
	case 20:
		dev_idx = XPM_NODEIDX_DEV_GPIO;
		break;
	case 21:
		dev_idx = XPM_NODEIDX_DEV_I2C_0;
		break;
	case 22:
		dev_idx = XPM_NODEIDX_DEV_I2C_1;
		break;
	case 23:
		dev_idx = XPM_NODEIDX_DEV_SPI_0;
		break;
	case 24:
		dev_idx = XPM_NODEIDX_DEV_SPI_1;
		break;
	case 25:
		dev_idx = XPM_NODEIDX_DEV_UART_0;
		break;
	case 26:
		dev_idx = XPM_NODEIDX_DEV_UART_1;
		break;
	case 27:
		dev_idx = XPM_NODEIDX_DEV_CAN_FD_0;
		break;
	case 28:
		dev_idx = XPM_NODEIDX_DEV_CAN_FD_1;
		break;
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 98:
		dev_idx = XPM_NODEIDX_DEV_USB_0;
		break;
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 99:
		dev_idx = XPM_NODEIDX_DEV_USB_1;
		break;
	case 39:
	case 40:
		dev_idx = XPM_NODEIDX_DEV_GEM_0;
		break;
	case 41:
	case 42:
		dev_idx = XPM_NODEIDX_DEV_GEM_1;
		break;
	case 43:
	case 44:
	case 45:
		dev_idx = XPM_NODEIDX_DEV_TTC_0;
		break;
	case 46:
	case 47:
	case 48:
		dev_idx = XPM_NODEIDX_DEV_TTC_1;
		break;
	case 49:
	case 50:
	case 51:
		dev_idx = XPM_NODEIDX_DEV_TTC_2;
		break;
	case 52:
	case 53:
	case 54:
		dev_idx = XPM_NODEIDX_DEV_TTC_3;
		break;
	case 72:
		dev_idx = XPM_NODEIDX_DEV_ADMA_0;
		break;
	case 73:
		dev_idx = XPM_NODEIDX_DEV_ADMA_1;
		break;
	case 74:
		dev_idx = XPM_NODEIDX_DEV_ADMA_2;
		break;
	case 75:
		dev_idx = XPM_NODEIDX_DEV_ADMA_3;
		break;
	case 76:
		dev_idx = XPM_NODEIDX_DEV_ADMA_4;
		break;
	case 77:
		dev_idx = XPM_NODEIDX_DEV_ADMA_5;
		break;
	case 78:
		dev_idx = XPM_NODEIDX_DEV_ADMA_6;
		break;
	case 79:
		dev_idx = XPM_NODEIDX_DEV_ADMA_7;
		break;
	case 184:
	case 185:
		dev_idx = XPM_NODEIDX_DEV_SDIO_0;
		break;
	case 186:
	case 187:
		dev_idx = XPM_NODEIDX_DEV_SDIO_1;
		break;
	case 200:
		dev_idx = XPM_NODEIDX_DEV_RTC;
		break;
	default:
		dev_idx = XPM_NODEIDX_DEV_MIN;
		break;
	}

	return dev_idx;
}

/**
 * pm_client_suspend() - Client-specific suspend actions. This function
 *                       should contain any PU-specific actions required
 *                       prior to sending suspend request to PMU. Actions
 *                       taken depend on the state system is suspending to.
 * @proc: processor which need to suspend.
 * @state: desired suspend state.
 * @flag: 0 - Call from secure source.
 *	  1 - Call from non-secure source.
 *
 */
void pm_client_suspend(const struct pm_proc *proc, uint32_t state, uint32_t flag)
{
	uint32_t cpu_id = plat_my_core_pos();
	uintptr_t val;

	pm_client_lock_get();

	if (state == PM_STATE_SUSPEND_TO_RAM) {
		pm_client_set_wakeup_sources((uint32_t)proc->node_id, flag);
	}

	val = read_cpu_pwrctrl_val();
	val |= CORE_PWRDN_EN_BIT_MASK;
	write_cpu_pwrctrl_val(val);

	isb();

	/* Enable power down interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IEN_POWER_REG(cpu_id),
		      APU_PCIL_CORE_X_IEN_POWER_MASK);
	/* Enable wake interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IEN_WAKE_REG(cpu_id),
		      APU_PCIL_CORE_X_IEN_WAKE_MASK);

	pm_client_lock_release();
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
	uint32_t ret = UNDEFINED_CPUID;
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(pm_procs_all); i++) {
		if (pm_procs_all[i].node_id == nid) {
			ret = i;
			break;
		}
	}
	return ret;
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
	uintptr_t val;

	if (cpuid == UNDEFINED_CPUID) {
		return;
	}

	pm_client_lock_get();

	/* Clear powerdown request */
	val = read_cpu_pwrctrl_val();
	val &= ~CORE_PWRDN_EN_BIT_MASK;
	write_cpu_pwrctrl_val(val);

	isb();

	/* Disabled power down interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IDS_POWER_REG(cpuid),
			APU_PCIL_CORE_X_IDS_POWER_MASK);
	/* Disable wake interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IDS_WAKE_REG(cpuid),
		      APU_PCIL_CORE_X_IDS_WAKE_MASK);

	pm_client_lock_release();
}
