/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <msm8916_mmap.h>
#include "msm8916_pm.h"

static int msm8916_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core = MPIDR_AFFLVL0_VAL(mpidr);

	VERBOSE("PSCI: Booting CPU %d\n", core);
	msm8916_cpu_boot(core);

	return PSCI_E_SUCCESS;
}

static void msm8916_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

static void __dead2 msm8916_system_reset(void)
{
	mmio_write_32(MPM_PS_HOLD, 0);
	mdelay(1000);

	ERROR("PSCI: System reset failed\n");
	panic();
}

static const plat_psci_ops_t msm8916_psci_ops = {
	.pwr_domain_on			= msm8916_pwr_domain_on,
	.pwr_domain_on_finish		= msm8916_pwr_domain_on_finish,
	.system_off			= msm8916_system_reset,
	.system_reset			= msm8916_system_reset,
};

/* Defined and used in msm8916_helpers.S */
extern uintptr_t msm8916_entry_point;

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	msm8916_entry_point = sec_entrypoint;
	*psci_ops = &msm8916_psci_ops;
	return 0;
}
