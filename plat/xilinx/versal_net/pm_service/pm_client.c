/*
 * Copyright (c) 2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
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
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#include <plat_ipi.h>
#include <platform_def.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include <versal_net_def.h>

#define UNDEFINED_CPUID		(~0)

DEFINE_RENAME_SYSREG_RW_FUNCS(cpu_pwrctrl_val, S3_0_C15_C2_7)
DEFINE_BAKERY_LOCK(pm_client_secure_lock);

static const struct pm_ipi apu_ipi = {
	.local_ipi_id = IPI_ID_APU,
	.remote_ipi_id = IPI_ID_PMC,
	.buffer_base = IPI_BUFFER_APU_BASE,
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
 * pm_get_proc() - returns pointer to the proc structure
 * @param cpuid	id of the cpu whose proc struct pointer should be returned
 *
 * @return pointer to a proc structure if proc is found, otherwise NULL
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
 * pm_client_suspend() - Client-specific suspend actions
 *
 * This function should contain any PU-specific actions
 * required prior to sending suspend request to PMU
 * Actions taken depend on the state system is suspending to.
 *
 * @param proc	processor which need to suspend
 * @param state	desired suspend state
 */
void pm_client_suspend(const struct pm_proc *proc, uint32_t state)
{
	uint32_t cpu_id = plat_my_core_pos();
	uintptr_t val;

	bakery_lock_get(&pm_client_secure_lock);

	/* TODO: Set wakeup source */

	val = read_cpu_pwrctrl_val();
	val |= CORE_PWRDN_EN_BIT_MASK;
	write_cpu_pwrctrl_val(val);

	isb();

	/* Clear power down interrupt status before enabling */
	mmio_write_32(APU_PCIL_CORE_X_ISR_POWER_REG(cpu_id),
		      APU_PCIL_CORE_X_ISR_POWER_MASK);
	/* Enable power down interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IEN_POWER_REG(cpu_id),
		      APU_PCIL_CORE_X_IEN_POWER_MASK);
	/* Clear wakeup interrupt status before enabling */
	mmio_write_32(APU_PCIL_CORE_X_ISR_WAKE_REG(cpu_id),
		      APU_PCIL_CORE_X_ISR_WAKE_MASK);
	/* Enable wake interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IEN_WAKE_REG(cpu_id),
		      APU_PCIL_CORE_X_IEN_WAKE_MASK);

	bakery_lock_release(&pm_client_secure_lock);
}

/**
 * pm_get_cpuid() - get the local cpu ID for a global node ID
 * @param nid	node id of the processor
 *
 * @return the cpu ID (starting from 0) for the subsystem
 */
static uint32_t pm_get_cpuid(uint32_t nid)
{
	for (size_t i = 0; i < ARRAY_SIZE(pm_procs_all); i++) {
		if (pm_procs_all[i].node_id == nid) {
			return i;
		}
	}
	return UNDEFINED_CPUID;
}

/**
 * pm_client_wakeup() - Client-specific wakeup actions
 *
 * This function should contain any PU-specific actions
 * required for waking up another APU core
 *
 * @param proc	Processor which need to wakeup
 */
void pm_client_wakeup(const struct pm_proc *proc)
{
	uint32_t cpuid = pm_get_cpuid(proc->node_id);
	uintptr_t val;

	if (cpuid == UNDEFINED_CPUID) {
		return;
	}

	bakery_lock_get(&pm_client_secure_lock);

	/* Clear powerdown request */
	val = read_cpu_pwrctrl_val();
	val &= ~CORE_PWRDN_EN_BIT_MASK;
	write_cpu_pwrctrl_val(val);

	isb();

	/* Disabled power down interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IDS_POWER_REG(cpuid),
			APU_PCIL_CORE_X_IDS_POWER_MASK);
	/* Clear wakeup interrupt status before disabling */
	mmio_write_32(APU_PCIL_CORE_X_ISR_WAKE_REG(cpuid),
		      APU_PCIL_CORE_X_ISR_WAKE_MASK);
	/* Disable wake interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IDS_WAKE_REG(cpuid),
		      APU_PCIL_CORE_X_IDS_WAKE_MASK);

	bakery_lock_release(&pm_client_secure_lock);
}

/**
 * pm_client_abort_suspend() - Client-specific abort-suspend actions
 *
 * This function should contain any PU-specific actions
 * required for aborting a prior suspend request
 */
void pm_client_abort_suspend(void)
{
	uint32_t cpu_id = plat_my_core_pos();
	uintptr_t val;

	/* Enable interrupts at processor level (for current cpu) */
	gicv3_cpuif_enable(plat_my_core_pos());

	bakery_lock_get(&pm_client_secure_lock);

	/* Clear powerdown request */
	val = read_cpu_pwrctrl_val();
	val &= ~CORE_PWRDN_EN_BIT_MASK;
	write_cpu_pwrctrl_val(val);

	isb();

	/* Disabled power down interrupt */
	mmio_write_32(APU_PCIL_CORE_X_IDS_POWER_REG(cpu_id),
			APU_PCIL_CORE_X_IDS_POWER_MASK);

	bakery_lock_release(&pm_client_secure_lock);
}
