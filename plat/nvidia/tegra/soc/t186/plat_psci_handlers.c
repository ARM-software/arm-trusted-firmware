/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <denver.h>
#include <mce.h>
#include <platform.h>
#include <psci.h>
#include <smmu.h>
#include <string.h>
#include <t18x_ari.h>
#include <tegra_private.h>

extern void prepare_cpu_pwr_dwn(void);
extern void tegra186_cpu_reset_handler(void);
extern uint32_t __tegra186_cpu_reset_handler_end,
		__tegra186_smmu_context;

/* state id mask */
#define TEGRA186_STATE_ID_MASK		0xF
/* constants to get power state's wake time */
#define TEGRA186_WAKE_TIME_MASK		0x0FFFFFF0
#define TEGRA186_WAKE_TIME_SHIFT	4
/* default core wake mask for CPU_SUSPEND */
#define TEGRA186_CORE_WAKE_MASK		0x180c
/* context size to save during system suspend */
#define TEGRA186_SE_CONTEXT_SIZE	3

static uint32_t se_regs[TEGRA186_SE_CONTEXT_SIZE];
static struct t18x_psci_percpu_data {
	unsigned int wake_time;
} __aligned(CACHE_WRITEBACK_GRANULE) percpu_data[PLATFORM_CORE_COUNT];

/* System power down state */
uint32_t tegra186_system_powerdn_state = TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_POWER_OFF;

int32_t tegra_soc_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int state_id = psci_get_pstate_id(power_state) & TEGRA186_STATE_ID_MASK;
	int cpu = plat_my_core_pos();

	/* save the core wake time (in TSC ticks)*/
	percpu_data[cpu].wake_time = (power_state & TEGRA186_WAKE_TIME_MASK)
			<< TEGRA186_WAKE_TIME_SHIFT;

	/*
	 * Clean percpu_data[cpu] to DRAM. This needs to be done to ensure that
	 * the correct value is read in tegra_soc_pwr_domain_suspend(), which
	 * is called with caches disabled. It is possible to read a stale value
	 * from DRAM in that function, because the L2 cache is not flushed
	 * unless the cluster is entering CC6/CC7.
	 */
	clean_dcache_range((uint64_t)&percpu_data[cpu],
			sizeof(percpu_data[cpu]));

	/* Sanity check the requested state id */
	switch (state_id) {
	case PSTATE_ID_CORE_IDLE:
	case PSTATE_ID_CORE_POWERDN:

		/* Core powerdown request */
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = state_id;
		req_state->pwr_domain_state[MPIDR_AFFLVL1] = state_id;

		break;

	default:
		ERROR("%s: unsupported state id (%d)\n", __func__, state_id);
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state;
	unsigned int stateid_afflvl0, stateid_afflvl2;
	int cpu = plat_my_core_pos();
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	mce_cstate_info_t cstate_info = { 0 };
	uint64_t smmu_ctx_base;
	uint32_t val;

	/* get the state ID */
	pwr_domain_state = target_state->pwr_domain_state;
	stateid_afflvl0 = pwr_domain_state[MPIDR_AFFLVL0] &
		TEGRA186_STATE_ID_MASK;
	stateid_afflvl2 = pwr_domain_state[PLAT_MAX_PWR_LVL] &
		TEGRA186_STATE_ID_MASK;

	if ((stateid_afflvl0 == PSTATE_ID_CORE_IDLE) ||
	    (stateid_afflvl0 == PSTATE_ID_CORE_POWERDN)) {

		/* Enter CPU idle/powerdown */
		val = (stateid_afflvl0 == PSTATE_ID_CORE_IDLE) ?
			TEGRA_ARI_CORE_C6 : TEGRA_ARI_CORE_C7;
		(void)mce_command_handler(MCE_CMD_ENTER_CSTATE, val,
				percpu_data[cpu].wake_time, 0);

	} else if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {

		/* save SE registers */
		se_regs[0] = mmio_read_32(TEGRA_SE0_BASE +
				SE_MUTEX_WATCHDOG_NS_LIMIT);
		se_regs[1] = mmio_read_32(TEGRA_RNG1_BASE +
				RNG_MUTEX_WATCHDOG_NS_LIMIT);
		se_regs[2] = mmio_read_32(TEGRA_PKA1_BASE +
				PKA_MUTEX_WATCHDOG_NS_LIMIT);

		/* save 'Secure Boot' Processor Feature Config Register */
		val = mmio_read_32(TEGRA_MISC_BASE + MISCREG_PFCFG);
		mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV6, val);

		/* save SMMU context to TZDRAM */
		smmu_ctx_base = params_from_bl2->tzdram_base +
			((uintptr_t)&__tegra186_smmu_context -
			 (uintptr_t)tegra186_cpu_reset_handler);
		tegra_smmu_save_context((uintptr_t)smmu_ctx_base);

		/* Prepare for system suspend */
		cstate_info.cluster = TEGRA_ARI_CLUSTER_CC7;
		cstate_info.system = TEGRA_ARI_SYSTEM_SC7;
		cstate_info.system_state_force = 1;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);

		/* Loop until system suspend is allowed */
		do {
			val = mce_command_handler(MCE_CMD_IS_SC7_ALLOWED,
					TEGRA_ARI_CORE_C7,
					MCE_CORE_SLEEP_TIME_INFINITE,
					0);
		} while (val == 0);

		/* Instruct the MCE to enter system suspend state */
		(void)mce_command_handler(MCE_CMD_ENTER_CSTATE,
			TEGRA_ARI_CORE_C7, MCE_CORE_SLEEP_TIME_INFINITE, 0);
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler to calculate the proper target power level at the
 * specified affinity level
 ******************************************************************************/
plat_local_state_t tegra_soc_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	plat_local_state_t target = *states;
	int cpu = plat_my_core_pos(), ret, cluster_powerdn = 1;
	int core_pos = read_mpidr() & MPIDR_CPU_MASK;
	mce_cstate_info_t cstate_info = { 0 };

	/* get the power state at this level */
	if (lvl == MPIDR_AFFLVL1)
		target = *(states + core_pos);
	if (lvl == MPIDR_AFFLVL2)
		target = *(states + cpu);

	/* CPU suspend */
	if (lvl == MPIDR_AFFLVL1 && target == PSTATE_ID_CORE_POWERDN) {

		/* Program default wake mask */
		cstate_info.wake_mask = TEGRA186_CORE_WAKE_MASK;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);

		/* Check if CCx state is allowed. */
		ret = mce_command_handler(MCE_CMD_IS_CCX_ALLOWED,
				TEGRA_ARI_CORE_C7, percpu_data[cpu].wake_time,
				0);
		if (ret)
			return PSTATE_ID_CORE_POWERDN;
	}

	/* CPU off */
	if (lvl == MPIDR_AFFLVL1 && target == PLAT_MAX_OFF_STATE) {

		/* find out the number of ON cpus in the cluster */
		do {
			target = *states++;
			if (target != PLAT_MAX_OFF_STATE)
				cluster_powerdn = 0;
		} while (--ncpu);

		/* Enable cluster powerdn from last CPU in the cluster */
		if (cluster_powerdn) {

			/* Enable CC7 state and turn off wake mask */
			cstate_info.cluster = TEGRA_ARI_CLUSTER_CC7;
			cstate_info.update_wake_mask = 1;
			mce_update_cstate_info(&cstate_info);

			/* Check if CCx state is allowed. */
			ret = mce_command_handler(MCE_CMD_IS_CCX_ALLOWED,
						  TEGRA_ARI_CORE_C7,
						  MCE_CORE_SLEEP_TIME_INFINITE,
						  0);
			if (ret)
				return PSTATE_ID_CORE_POWERDN;

		} else {

			/* Turn off wake_mask */
			cstate_info.update_wake_mask = 1;
			mce_update_cstate_info(&cstate_info);
		}
	}

	/* System Suspend */
	if (((lvl == MPIDR_AFFLVL2) || (lvl == MPIDR_AFFLVL1)) &&
	    (target == PSTATE_ID_SOC_POWERDN))
		return PSTATE_ID_SOC_POWERDN;

	/* default state */
	return PSCI_LOCAL_STATE_RUN;
}

int tegra_soc_pwr_domain_power_down_wfi(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state =
		target_state->pwr_domain_state;
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	unsigned int stateid_afflvl2 = pwr_domain_state[PLAT_MAX_PWR_LVL] &
		TEGRA186_STATE_ID_MASK;
	uint64_t val;

	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {
		/*
		 * The TZRAM loses power when we enter system suspend. To
		 * allow graceful exit from system suspend, we need to copy
		 * BL3-1 over to TZDRAM.
		 */
		val = params_from_bl2->tzdram_base +
			((uintptr_t)&__tegra186_cpu_reset_handler_end -
			 (uintptr_t)tegra186_cpu_reset_handler);
		memcpy16((void *)(uintptr_t)val, (void *)(uintptr_t)BL31_BASE,
			 (uintptr_t)&__BL31_END__ - (uintptr_t)BL31_BASE);
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_on(u_register_t mpidr)
{
	uint32_t target_cpu = mpidr & MPIDR_CPU_MASK;
	uint32_t target_cluster = (mpidr & MPIDR_CLUSTER_MASK) >>
			MPIDR_AFFINITY_BITS;

	if (target_cluster > MPIDR_AFFLVL1) {
		ERROR("%s: unsupported CPU (0x%lx)\n", __func__, mpidr);
		return PSCI_E_NOT_PRESENT;
	}

	/* construct the target CPU # */
	target_cpu |= (target_cluster << 2);

	mce_command_handler(MCE_CMD_ONLINE_CORE, target_cpu, 0, 0);

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	int stateid_afflvl2 = target_state->pwr_domain_state[PLAT_MAX_PWR_LVL];
	int stateid_afflvl0 = target_state->pwr_domain_state[MPIDR_AFFLVL0];
	mce_cstate_info_t cstate_info = { 0 };

	/*
	 * Reset power state info for CPUs when onlining, we set
	 * deepest power when offlining a core but that may not be
	 * requested by non-secure sw which controls idle states. It
	 * will re-init this info from non-secure software when the
	 * core come online.
	 */
	if (stateid_afflvl0 == PLAT_MAX_OFF_STATE) {

		cstate_info.cluster = TEGRA_ARI_CLUSTER_CC1;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);
	}

	/*
	 * Check if we are exiting from deep sleep and restore SE
	 * context if we are.
	 */
	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {

		mmio_write_32(TEGRA_SE0_BASE + SE_MUTEX_WATCHDOG_NS_LIMIT,
			se_regs[0]);
		mmio_write_32(TEGRA_RNG1_BASE + RNG_MUTEX_WATCHDOG_NS_LIMIT,
			se_regs[1]);
		mmio_write_32(TEGRA_PKA1_BASE + PKA_MUTEX_WATCHDOG_NS_LIMIT,
			se_regs[2]);

		/* Init SMMU */
		tegra_smmu_init();

		/*
		 * Reset power state info for the last core doing SC7
		 * entry and exit, we set deepest power state as CC7
		 * and SC7 for SC7 entry which may not be requested by
		 * non-secure SW which controls idle states.
		 */
		cstate_info.cluster = TEGRA_ARI_CLUSTER_CC7;
		cstate_info.system = TEGRA_ARI_SYSTEM_SC1;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	int impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

	/* Disable Denver's DCO operations */
	if (impl == DENVER_IMPL)
		denver_disable_dco();

	/* Turn off CPU */
	(void)mce_command_handler(MCE_CMD_ENTER_CSTATE, TEGRA_ARI_CORE_C7,
			MCE_CORE_SLEEP_TIME_INFINITE, 0);

	return PSCI_E_SUCCESS;
}

__dead2 void tegra_soc_prepare_system_off(void)
{
	mce_cstate_info_t cstate_info = { 0 };
	uint32_t val;

	if (tegra186_system_powerdn_state == TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_POWER_OFF) {

		/* power off the entire system */
		mce_enter_ccplex_state(tegra186_system_powerdn_state);

	} else if (tegra186_system_powerdn_state == TEGRA_ARI_SYSTEM_SC8) {

		/* Prepare for quasi power down */
		cstate_info.cluster = TEGRA_ARI_CLUSTER_CC7;
		cstate_info.system = TEGRA_ARI_SYSTEM_SC8;
		cstate_info.system_state_force = 1;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);

		/* loop until other CPUs power down */
		do {
			val = mce_command_handler(MCE_CMD_IS_SC7_ALLOWED,
					TEGRA_ARI_CORE_C7,
					MCE_CORE_SLEEP_TIME_INFINITE,
					0);
		} while (val == 0);

		/* Enter quasi power down state */
		(void)mce_command_handler(MCE_CMD_ENTER_CSTATE,
			TEGRA_ARI_CORE_C7, MCE_CORE_SLEEP_TIME_INFINITE, 0);

		/* disable GICC */
		tegra_gic_cpuif_deactivate();

		/* power down core */
		prepare_cpu_pwr_dwn();

		/* flush L1/L2 data caches */
		dcsw_op_all(DCCISW);

	} else {
		ERROR("%s: unsupported power down state (%d)\n", __func__,
			tegra186_system_powerdn_state);
	}

	wfi();

	/* wait for the system to power down */
	for (;;) {
		;
	}
}

int tegra_soc_prepare_system_reset(void)
{
	mce_enter_ccplex_state(TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_REBOOT);

	return PSCI_E_SUCCESS;
}
