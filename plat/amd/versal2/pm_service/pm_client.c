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

#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include "def.h"
#include <plat_ipi.h>
#include <plat_pm_common.h>
#include "pm_api_sys.h"
#include "pm_client.h"

#define UNDEFINED_CPUID	UINT32_MAX

DEFINE_RENAME_SYSREG_RW_FUNCS(cpu_pwrctrl_val, S3_0_C15_C2_7)

/*
 * ARM v8.2, the cache will turn off automatically when cpu
 * power down. Therefore, there is no doubt to use the spin_lock here.
 */
static spinlock_t pm_client_secure_lock;
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
	},
	{
		.node_id = PM_DEV_CLUSTER0_ACPU_1,
		.ipi = &apu_ipi,
	},
	{
		.node_id = PM_DEV_CLUSTER1_ACPU_0,
		.ipi = &apu_ipi,
	},
	{
		.node_id = PM_DEV_CLUSTER1_ACPU_1,
		.ipi = &apu_ipi,
	},
	{
		.node_id = PM_DEV_CLUSTER2_ACPU_0,
		.ipi = &apu_ipi,
	},
	{
		.node_id = PM_DEV_CLUSTER2_ACPU_1,
		.ipi = &apu_ipi,
	},
	{
		.node_id = PM_DEV_CLUSTER3_ACPU_0,
		.ipi = &apu_ipi,
	},
	{
		.node_id = PM_DEV_CLUSTER3_ACPU_1,
		.ipi = &apu_ipi,
	},
};

const struct pm_proc *primary_proc = &pm_procs_all[0];

/**
 * pm_get_proc() - returns pointer to the proc structure.
 * @cpuid: id of the cpu whose proc struct pointer should be returned.
 *
 * Return: Pointer to a proc structure if proc is found, otherwise NULL.
 */
const struct pm_proc *pm_get_proc(uint32_t cpuid)
{
	const struct pm_proc *proc = NULL;

	if (cpuid < ARRAY_SIZE(pm_procs_all)) {
		proc = &pm_procs_all[cpuid];
	} else {
		ERROR("cpuid: %d proc NULL\n", cpuid);
	}

	return proc;
}

/**
 * irq_to_pm_node_idx - Get PM node index corresponding to the interrupt number.
 * @irq: Interrupt number.
 *
 * Return: PM node index corresponding to the specified interrupt.
 */
enum pm_device_node_idx irq_to_pm_node_idx(uint32_t irq)
{
	enum pm_device_node_idx dev_idx = XPM_NODEIDX_DEV_MIN;

	assert(irq <= IRQ_MAX);

	switch (irq) {
	case 11:
		dev_idx = XPM_NODEIDX_DEV_I2C_2;
		break;
	case 12:
		dev_idx = XPM_NODEIDX_DEV_I2C_3;
		break;
	case 13:
		dev_idx = XPM_NODEIDX_DEV_I2C_4;
		break;
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
		dev_idx = XPM_NODEIDX_DEV_TTC_0;
		break;
	case 44:
		dev_idx = XPM_NODEIDX_DEV_TTC_1;
		break;
	case 45:
		dev_idx = XPM_NODEIDX_DEV_TTC_2;
		break;
	case 46:
		dev_idx = XPM_NODEIDX_DEV_TTC_3;
		break;
	case 47:
		dev_idx = XPM_NODEIDX_DEV_TTC_4;
		break;
	case 48:
		dev_idx = XPM_NODEIDX_DEV_TTC_5;
		break;
	case 49:
		dev_idx = XPM_NODEIDX_DEV_TTC_6;
		break;
	case 50:
		dev_idx = XPM_NODEIDX_DEV_TTC_7;
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
	case 95:
		dev_idx = XPM_NODEIDX_DEV_CAN_FD_2;
		break;
	case 96:
		dev_idx = XPM_NODEIDX_DEV_CAN_FD_3;
		break;
	case 100:
		dev_idx = XPM_NODEIDX_DEV_I2C_5;
		break;
	case 101:
		dev_idx = XPM_NODEIDX_DEV_I2C_6;
		break;
	case 102:
		dev_idx = XPM_NODEIDX_DEV_I2C_7;
		break;
	case 164:
		dev_idx = XPM_NODEIDX_DEV_MMI_GEM;
		break;
	case 200:
		dev_idx = XPM_NODEIDX_DEV_RTC;
		break;
	case 218:
		dev_idx = XPM_NODEIDX_DEV_SDIO_0;
		break;
	case 220:
		dev_idx = XPM_NODEIDX_DEV_SDIO_1;
		break;
	default:
		dev_idx = XPM_NODEIDX_DEV_MIN;
		break;
	}

	return dev_idx;
}

/**
 * pm_client_suspend() - Client-specific suspend actions. This function
 *                       perform actions required prior to sending suspend
 *                       request.
 *                       Actions taken depend on the state system is
 *                       suspending to.
 * @proc: processor which need to suspend.
 * @state: desired suspend state.
 * @flag: 0 - Call from secure source.
 *	  1 - Call from non-secure source.
 */
void pm_client_suspend(const struct pm_proc *proc, uint32_t state, uint32_t flag)
{
	uint32_t cpu_id = plat_my_core_pos();
	uintptr_t val;
	/*
	 * Get the core index, use it calculate offset for secondary cores
	 * to match with register database
	 */
	uint32_t core_index = cpu_id + ((cpu_id / 2U) * 2U);

	pm_client_lock_get();

	if (state == PM_STATE_SUSPEND_TO_RAM) {
		pm_client_set_wakeup_sources((uint32_t)proc->node_id, flag);
	}

	val = read_cpu_pwrctrl_val();
	val |= CORE_PWRDN_EN_BIT_MASK;
	write_cpu_pwrctrl_val(val);

	isb();

	/* Enable power down interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IEN_POWER_REG(core_index),
		      APU_PCIL_CORE_X_IEN_POWER_MASK);
	/* Enable wake interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IEN_WAKE_REG(core_index),
		      APU_PCIL_CORE_X_IEN_WAKE_MASK);

	pm_client_lock_release();
}

/**
 * pm_get_cpuid() - get the local cpu ID for a global node ID.
 * @nid: node id of the processor.
 *
 * Return: the cpu ID (starting from 0) for the subsystem.
 */
static uint32_t pm_get_cpuid(uint32_t nid)
{
	uint32_t ret = (uint32_t) UNDEFINED_CPUID;
	size_t i;

	for (i = 0; i < ARRAY_SIZE(pm_procs_all); i++) {
		if (pm_procs_all[i].node_id == nid) {
			ret = (uint32_t)i;
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
 */
void pm_client_wakeup(const struct pm_proc *proc)
{
	uint32_t cpuid = pm_get_cpuid(proc->node_id);
	uintptr_t val;

	if (cpuid != (uint32_t) UNDEFINED_CPUID) {
		/*
		 * Get the core index and use it to calculate offset for
		 * disabling power down and wakeup interrupts.
		 * i.e., Convert cpu-id to core_index with the following mapping:
		 *  cpu-id -> core_index
		 *       0 -> 0
		 *       1 -> 1
		 *       2 -> 4
		 *       3 -> 5
		 *       4 -> 8
		 *       5 -> 9
		 *       6 -> 12
		 *       7 -> 13
		 * to match with register database.
		 */
		uint32_t core_index = cpuid + ((cpuid / 2U) * 2U);

		pm_client_lock_get();

		/* Clear powerdown request */
		val = read_cpu_pwrctrl_val();
		val &= ~CORE_PWRDN_EN_BIT_MASK;
		write_cpu_pwrctrl_val(val);

		isb();

		/* Disabled power down interrupt */
		mmio_write_32(APU_PCIL_CORE_X_IDS_POWER_REG(core_index),
			      APU_PCIL_CORE_X_IDS_POWER_MASK);
		/* Disable wake interrupt */
		mmio_write_32(APU_PCIL_CORE_X_IDS_WAKE_REG(core_index),
			      APU_PCIL_CORE_X_IDS_WAKE_MASK);

		pm_client_lock_release();
	}
}
