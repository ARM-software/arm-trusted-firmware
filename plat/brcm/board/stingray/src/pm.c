/*
 * Copyright (c) 2015 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/ccn.h>
#include <drivers/delay_timer.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>

#ifdef USE_PAXC
#include <chimp.h>
#endif
#include <cmn_plat_util.h>
#include <ihost_pm.h>
#include <plat_brcm.h>
#include <platform_def.h>

static uint64_t plat_sec_entrypoint;

/*******************************************************************************
 * SR handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
static int brcm_pwr_domain_on(u_register_t mpidr)
{
	int cpuid;

	cpuid = plat_brcm_calc_core_pos(mpidr);
	INFO("mpidr :%lu, cpuid:%d\n", mpidr, cpuid);

#ifdef USE_SINGLE_CLUSTER
	if (cpuid > 1)
		return PSCI_E_INTERN_FAIL;
#endif

	ihost_power_on_cluster(mpidr);

	ihost_power_on_secondary_core(mpidr, plat_sec_entrypoint);

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * SR handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
static void brcm_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	unsigned long cluster_id = MPIDR_AFFLVL1_VAL(read_mpidr());

	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					PLAT_LOCAL_STATE_OFF);

	if (target_state->pwr_domain_state[MPIDR_AFFLVL1] ==
					PLAT_LOCAL_STATE_OFF) {
		INFO("Cluster #%lu entering to snoop/dvm domain\n", cluster_id);
		ccn_enter_snoop_dvm_domain(1 << cluster_id);
	}

	/* Enable the gic cpu interface */
	plat_brcm_gic_pcpu_init();

	/* Program the gic per-cpu distributor or re-distributor interface */
	plat_brcm_gic_cpuif_enable();

	INFO("Gic Initialization done for this affinity instance\n");
}

static void __dead2 brcm_system_reset(void)
{
	uint32_t reset_type = SOFT_SYS_RESET_L1;

#ifdef USE_PAXC
	if (bcm_chimp_is_nic_mode())
		reset_type = SOFT_RESET_L3;
#endif
	INFO("System rebooting - L%d...\n", reset_type);

	plat_soft_reset(reset_type);

	/* Prevent the function to return due to the attribute */
	while (1)
		;
}

static int brcm_system_reset2(int is_vendor, int reset_type,
			      u_register_t cookie)
{
	INFO("System rebooting - L%d...\n", reset_type);

	plat_soft_reset(reset_type);

	/*
	 * plat_soft_reset cannot return (it is a __dead function),
	 * but brcm_system_reset2 has to return some value, even in
	 * this case.
	 */
	return 0;
}

/*******************************************************************************
 * Export the platform handlers via plat_brcm_psci_pm_ops. The ARM Standard
 * platform will take care of registering the handlers with PSCI.
 ******************************************************************************/
const plat_psci_ops_t plat_brcm_psci_pm_ops = {
	.pwr_domain_on		= brcm_pwr_domain_on,
	.pwr_domain_on_finish	= brcm_pwr_domain_on_finish,
	.system_reset		= brcm_system_reset,
	.system_reset2		= brcm_system_reset2
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_brcm_psci_pm_ops;
	plat_sec_entrypoint = sec_entrypoint;

	return 0;
}
