/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <cci.h>
#include <debug.h>
#include <gicv2.h>
#include <hi6220.h>
#include <hisi_ipc.h>
#include <hisi_pwrc.h>
#include <hisi_sram_map.h>
#include <mmio.h>
#include <psci.h>

#include "hikey_def.h"

#define HIKEY_CLUSTER_STATE_ON		0
#define HIKEY_CLUSTER_STATE_OFF		1

static uintptr_t hikey_sec_entrypoint;
/* There're two clusters in HiKey. */
static int hikey_cluster_state[] = {HIKEY_CLUSTER_STATE_OFF,
				    HIKEY_CLUSTER_STATE_OFF};

/*******************************************************************************
 * Handler called when a power domain is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
static int hikey_pwr_domain_on(u_register_t mpidr)
{
	int cpu, cluster;
	int curr_cluster;

	cluster = MPIDR_AFFLVL1_VAL(mpidr);
	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	curr_cluster = MPIDR_AFFLVL1_VAL(read_mpidr());
	if (cluster != curr_cluster)
		hisi_ipc_cluster_on(cpu, cluster);

	hisi_pwrc_set_core_bx_addr(cpu, cluster, hikey_sec_entrypoint);
	hisi_ipc_cpu_on(cpu, cluster);
	return 0;
}

static void hikey_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	unsigned long mpidr;
	int cpu, cluster;

	mpidr = read_mpidr();
	cluster = MPIDR_AFFLVL1_VAL(mpidr);
	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	if (hikey_cluster_state[cluster] == HIKEY_CLUSTER_STATE_OFF) {
		/*
		 * Enable CCI coherency for this cluster.
		 * No need for locks as no other cpu is active at the moment.
		 */
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));
		hikey_cluster_state[cluster] = HIKEY_CLUSTER_STATE_ON;
	}

	/* Zero the jump address in the mailbox for this cpu */
	hisi_pwrc_set_core_bx_addr(cpu, cluster, 0);

	/* Program the GIC per-cpu distributor or re-distributor interface */
	gicv2_pcpu_distif_init();
	/* Enable the GIC cpu interface */
	gicv2_cpuif_enable();
}

/*******************************************************************************
 * Handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void hikey_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned long mpidr;
	int cpu, cluster;

	gicv2_cpuif_disable();

	mpidr = read_mpidr();
	cluster = MPIDR_AFFLVL1_VAL(mpidr);
	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	if (target_state->pwr_domain_state[MPIDR_AFFLVL1] ==
	    PLAT_MAX_OFF_STATE) {
		hisi_ipc_spin_lock(HISI_IPC_SEM_CPUIDLE);
		cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));
		hisi_ipc_spin_unlock(HISI_IPC_SEM_CPUIDLE);

		hisi_ipc_cluster_off(cpu, cluster);
		hikey_cluster_state[cluster] = HIKEY_CLUSTER_STATE_OFF;
	}
	hisi_ipc_cpu_off(cpu, cluster);
}

/*******************************************************************************
 * Handler to reboot the system.
 ******************************************************************************/
static void __dead2 hikey_system_reset(void)
{
	/* Send the system reset request */
	mmio_write_32(AO_SC_SYS_STAT0, 0x48698284);
	isb();
	dsb();

	wfi();
	panic();
}

/*******************************************************************************
 * Handler called to check the validity of the power state parameter.
 ******************************************************************************/
int hikey_validate_power_state(unsigned int power_state,
			       psci_power_state_t *req_state)
{
	int pstate = psci_get_pstate_type(power_state);
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int i;

	assert(req_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on power level 0
		 * Ignore any other power level.
		 */
		if (pwr_lvl != MPIDR_AFFLVL0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[MPIDR_AFFLVL0] =
					PLAT_MAX_RET_STATE;
	} else {
		for (i = MPIDR_AFFLVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					PLAT_MAX_OFF_STATE;
	}

	/*
	 * We expect the 'state id' to be zero.
	 */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Handler called to check the validity of the non secure entrypoint.
 ******************************************************************************/
static int hikey_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint > DDR_BASE) && (entrypoint < (DDR_BASE + DDR_SIZE)))
		return PSCI_E_SUCCESS;

	return PSCI_E_INVALID_ADDRESS;
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_psci_ops_t hikey_psci_ops = {
	.cpu_standby			= NULL,
	.pwr_domain_on			= hikey_pwr_domain_on,
	.pwr_domain_on_finish		= hikey_pwr_domain_on_finish,
	.pwr_domain_off			= hikey_pwr_domain_off,
	.pwr_domain_suspend		= NULL,
	.pwr_domain_suspend_finish	= NULL,
	.system_off			= NULL,
	.system_reset			= hikey_system_reset,
	.validate_power_state		= hikey_validate_power_state,
	.validate_ns_entrypoint		= hikey_validate_ns_entrypoint,
	.get_sys_suspend_power_state	= NULL,
};

/*******************************************************************************
 * Export the platform specific power ops and initialize Power Controller
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	hikey_sec_entrypoint = sec_entrypoint;

	/*
	 * Initialize PSCI ops struct
	 */
	*psci_ops = &hikey_psci_ops;

	return 0;
}
