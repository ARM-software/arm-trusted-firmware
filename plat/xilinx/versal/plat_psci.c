/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include "versal_private.h"

static uintptr_t versal_sec_entry;

static int versal_nopmc_pwr_domain_on(u_register_t mpidr)
{
	uint32_t r;
	unsigned int cpu_id = plat_core_pos_by_mpidr(mpidr);

	VERBOSE("%s: mpidr: 0x%lx\n", __func__, mpidr);

	if (cpu_id == -1)
		return PSCI_E_INTERN_FAIL;

	/*
	 * program RVBAR
	 */
	mmio_write_32(FPD_APU_RVBAR_L_0 + (cpu_id << 3), versal_sec_entry);
	mmio_write_32(FPD_APU_RVBAR_H_0 + (cpu_id << 3), versal_sec_entry >> 32);

	/*
	 * clear VINITHI
	 */
	r = mmio_read_32(FPD_APU_CONFIG_0);
	r &= ~(1 << FPD_APU_CONFIG_0_VINITHI_SHIFT << cpu_id);
	mmio_write_32(FPD_APU_CONFIG_0, r);

	/*
	 * FIXME: Add power up sequence, By default it works
	 * now without the need of it as it was powered up by
	 * default.
	 */

	/*
	 * clear power down request
	 */
	r = mmio_read_32(FPD_APU_PWRCTL);
	r &= ~(1 << cpu_id);
	mmio_write_32(FPD_APU_PWRCTL, r);

	/*
	 * release core reset
	 */
	r = mmio_read_32(CRF_RST_APU);
	r &= ~((CRF_RST_APU_ACPU_PWRON_RESET |
			CRF_RST_APU_ACPU_RESET) << cpu_id);
	mmio_write_32(CRF_RST_APU, r);

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
	.pwr_domain_on			= versal_nopmc_pwr_domain_on,
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
