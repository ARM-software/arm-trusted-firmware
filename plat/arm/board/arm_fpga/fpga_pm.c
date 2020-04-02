/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include "fpga_private.h"
#include <platform_def.h>

/*
 * This is a basic PSCI implementation that allows secondary CPUs to be
 * released from their initial state and continue to the warm boot entrypoint.
 *
 * The secondary CPUs are placed in a holding pen and released by calls
 * to fpga_pwr_domain_on(mpidr), which updates the hold entry for the CPU
 * specified by the mpidr argument - the (polling) target CPU will then branch
 * to the BL31 warm boot sequence at the entrypoint address.
 *
 * Additionally, the secondary CPUs are kept in a low-power wfe() state
 * (placed there at the end of each poll) and woken when necessary through
 * calls to sev() in fpga_pwr_domain_on(mpidr), once the hold state for the
 * relevant CPU has been updated.
 *
 * Hotplug is currently implemented using a wfi-loop, which removes the
 * dependencies on any power controllers or other mechanism that is specific
 * to the running system as specified by the FPGA image.
 */

uint64_t hold_base[PLATFORM_CORE_COUNT];
uintptr_t fpga_sec_entrypoint;

/*
 * Calls to the CPU specified by the mpidr will set its hold entry to a value
 * indicating that it should stop polling and branch off to the warm entrypoint.
 */
static int fpga_pwr_domain_on(u_register_t mpidr)
{
	int pos = plat_core_pos_by_mpidr(mpidr);
	unsigned long current_mpidr = read_mpidr_el1();

	if (pos < 0) {
		panic();
	}

	if (mpidr == current_mpidr) {
		return PSCI_E_ALREADY_ON;
	}
	hold_base[pos] = PLAT_FPGA_HOLD_STATE_GO;
	flush_dcache_range((uintptr_t)&hold_base[pos], sizeof(uint64_t));
	sev(); /* Wake any CPUs from wfe */

	return PSCI_E_SUCCESS;
}

void fpga_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	fpga_pwr_gic_on_finish();
}

static void fpga_pwr_domain_off(const psci_power_state_t *target_state)
{
	fpga_pwr_gic_off();

	while (1) {
		wfi();
	}
}

static void fpga_cpu_standby(plat_local_state_t cpu_state)
{
	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	u_register_t scr = read_scr_el3();
	write_scr_el3(scr|SCR_IRQ_BIT);
	dsb();
	wfi();
	write_scr_el3(scr);
}

plat_psci_ops_t plat_fpga_psci_pm_ops = {
	.pwr_domain_on = fpga_pwr_domain_on,
	.pwr_domain_on_finish = fpga_pwr_domain_on_finish,
	.pwr_domain_off = fpga_pwr_domain_off,
	.cpu_standby = fpga_cpu_standby
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	fpga_sec_entrypoint = sec_entrypoint;
	flush_dcache_range((uint64_t)&fpga_sec_entrypoint,
			   sizeof(fpga_sec_entrypoint));
	*psci_ops = &plat_fpga_psci_pm_ops;
	return 0;
}
