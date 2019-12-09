/*
 * Copyright (c) 2019, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * APU specific definition of processors in the subsystem as well as functions
 * for getting information about and changing state of the APU.
 */

#include <plat_ipi.h>
#include <platform_def.h>
#include <versal_def.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <drivers/arm/gicv3.h>
#include <plat/common/platform.h>
#include "pm_client.h"

DEFINE_BAKERY_LOCK(pm_client_secure_lock);

static const struct pm_ipi apu_ipi = {
	.local_ipi_id = IPI_ID_APU,
	.remote_ipi_id = IPI_ID_PMC,
	.buffer_base = IPI_BUFFER_APU_BASE,
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
 * pm_client_suspend() - Client-specific suspend actions
 *
 * This function should contain any PU-specific actions
 * required prior to sending suspend request to PMU
 * Actions taken depend on the state system is suspending to.
 */
void pm_client_suspend(const struct pm_proc *proc, unsigned int state)
{
	bakery_lock_get(&pm_client_secure_lock);

	/* Set powerdown request */
	mmio_write_32(FPD_APU_PWRCTL, mmio_read_32(FPD_APU_PWRCTL) |
		      proc->pwrdn_mask);

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
	/* Enable interrupts at processor level (for current cpu) */
	gicv3_cpuif_enable(plat_my_core_pos());

	bakery_lock_get(&pm_client_secure_lock);

	/* Clear powerdown request */
	mmio_write_32(FPD_APU_PWRCTL, mmio_read_32(FPD_APU_PWRCTL) &
		      ~primary_proc->pwrdn_mask);

	bakery_lock_release(&pm_client_secure_lock);
}

/**
 * pm_get_proc() - returns pointer to the proc structure
 * @cpuid:	id of the cpu whose proc struct pointer should be returned
 *
 * Return: pointer to a proc structure if proc is found, otherwise NULL
 */
const struct pm_proc *pm_get_proc(unsigned int cpuid)
{
	if (cpuid < ARRAY_SIZE(pm_procs_all))
		return &pm_procs_all[cpuid];

	return NULL;
}
