/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_private.h>
#include <pm_common.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include "pm_api_sys.h"
#include "pm_client.h"

static uintptr_t versal_sec_entry;

static int versal_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu_id = plat_core_pos_by_mpidr(mpidr);
	const struct pm_proc *proc;

	VERBOSE("%s: mpidr: 0x%lx\n", __func__, mpidr);

	if (cpu_id == -1)
		return PSCI_E_INTERN_FAIL;

	proc = pm_get_proc(cpu_id);

	/* Send request to PMC to wake up selected ACPU core */
	pm_req_wakeup(proc->node_id, (versal_sec_entry & 0xFFFFFFFF) | 0x1,
		      versal_sec_entry >> 32, 0);

	/* Clear power down request */
	pm_client_wakeup(proc);

	return PSCI_E_SUCCESS;
}

void versal_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* Enable the gic cpu interface */
	plat_versal_gic_pcpu_init();

	/* Program the gic per-cpu distributor or re-distributor interface */
	plat_versal_gic_cpuif_enable();
}

static const struct plat_psci_ops versal_nopmc_psci_ops = {
	.pwr_domain_on			= versal_pwr_domain_on,
	.pwr_domain_on_finish		= versal_pwr_domain_on_finish,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	versal_sec_entry = sec_entrypoint;

	*psci_ops = &versal_nopmc_psci_ops;

	return 0;
}
