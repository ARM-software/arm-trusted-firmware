/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <cortex_a57.h>
#include <denver.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <mce.h>
#include <smmu.h>
#include <stdbool.h>
#include <t18x_ari.h>
#include <tegra186_private.h>
#include <tegra_private.h>

extern void memcpy16(void *dest, const void *src, unsigned int length);

/* state id mask */
#define TEGRA186_STATE_ID_MASK		0xFU
/* constants to get power state's wake time */
#define TEGRA186_WAKE_TIME_MASK		0x0FFFFFF0U
#define TEGRA186_WAKE_TIME_SHIFT	4U
/* default core wake mask for CPU_SUSPEND */
#define TEGRA186_CORE_WAKE_MASK		0x180cU
/* context size to save during system suspend */
#define TEGRA186_SE_CONTEXT_SIZE	3U

static uint32_t se_regs[TEGRA186_SE_CONTEXT_SIZE];
static struct tegra_psci_percpu_data {
	uint32_t wake_time;
} __aligned(CACHE_WRITEBACK_GRANULE) tegra_percpu_data[PLATFORM_CORE_COUNT];

int32_t tegra_soc_validate_power_state(uint32_t power_state,
					psci_power_state_t *req_state)
{
	uint8_t state_id = (uint8_t)psci_get_pstate_id(power_state) & TEGRA186_STATE_ID_MASK;
	uint32_t cpu = plat_my_core_pos();
	int32_t ret = PSCI_E_SUCCESS;

	/* save the core wake time (in TSC ticks)*/
	tegra_percpu_data[cpu].wake_time = (power_state & TEGRA186_WAKE_TIME_MASK)
			<< TEGRA186_WAKE_TIME_SHIFT;

	/*
	 * Clean percpu_data[cpu] to DRAM. This needs to be done to ensure that
	 * the correct value is read in tegra_soc_pwr_domain_suspend(), which
	 * is called with caches disabled. It is possible to read a stale value
	 * from DRAM in that function, because the L2 cache is not flushed
	 * unless the cluster is entering CC6/CC7.
	 */
	clean_dcache_range((uint64_t)&tegra_percpu_data[cpu],
			sizeof(tegra_percpu_data[cpu]));

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
		ret = PSCI_E_INVALID_PARAMS;
		break;
	}

	return ret;
}

int32_t tegra_soc_cpu_standby(plat_local_state_t cpu_state)
{
	(void)cpu_state;
	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state;
	uint8_t stateid_afflvl0, stateid_afflvl2;
	uint32_t cpu = plat_my_core_pos();
	const plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
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
			(uint32_t)TEGRA_ARI_CORE_C6 : (uint32_t)TEGRA_ARI_CORE_C7;
		(void)mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE, (uint64_t)val,
				tegra_percpu_data[cpu].wake_time, 0U);

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
		mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_SECURE_BOOTP_FCFG, val);

		/* save SMMU context to TZDRAM */
		smmu_ctx_base = params_from_bl2->tzdram_base +
				tegra186_get_smmu_ctx_offset();
		tegra_smmu_save_context((uintptr_t)smmu_ctx_base);

		/* Prepare for system suspend */
		cstate_info.cluster = (uint32_t)TEGRA_ARI_CLUSTER_CC7;
		cstate_info.system = (uint32_t)TEGRA_ARI_SYSTEM_SC7;
		cstate_info.system_state_force = 1;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);

		/* Loop until system suspend is allowed */
		do {
			val = (uint32_t)mce_command_handler(
					(uint64_t)MCE_CMD_IS_SC7_ALLOWED,
					(uint64_t)TEGRA_ARI_CORE_C7,
					MCE_CORE_SLEEP_TIME_INFINITE,
					0U);
		} while (val == 0U);

		/* Instruct the MCE to enter system suspend state */
		(void)mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE,
			(uint64_t)TEGRA_ARI_CORE_C7, MCE_CORE_SLEEP_TIME_INFINITE, 0U);

		/* set system suspend state for house-keeping */
		tegra186_set_system_suspend_entry();

	} else {
		; /* do nothing */
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Helper function to check if this is the last ON CPU in the cluster
 ******************************************************************************/
static bool tegra_last_cpu_in_cluster(const plat_local_state_t *states,
			uint32_t ncpu)
{
	plat_local_state_t target;
	bool last_on_cpu = true;
	uint32_t num_cpus = ncpu, pos = 0;

	do {
		target = states[pos];
		if (target != PLAT_MAX_OFF_STATE) {
			last_on_cpu = false;
		}
		--num_cpus;
		pos++;
	} while (num_cpus != 0U);

	return last_on_cpu;
}

/*******************************************************************************
 * Helper function to get target power state for the cluster
 ******************************************************************************/
static plat_local_state_t tegra_get_afflvl1_pwr_state(const plat_local_state_t *states,
			uint32_t ncpu)
{
	uint32_t core_pos = (uint32_t)read_mpidr() & (uint32_t)MPIDR_CPU_MASK;
	uint32_t cpu = plat_my_core_pos();
	int32_t ret;
	plat_local_state_t target = states[core_pos];
	mce_cstate_info_t cstate_info = { 0 };

	/* CPU suspend */
	if (target == PSTATE_ID_CORE_POWERDN) {
		/* Program default wake mask */
		cstate_info.wake_mask = TEGRA186_CORE_WAKE_MASK;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);

		/* Check if CCx state is allowed. */
		ret = mce_command_handler((uint64_t)MCE_CMD_IS_CCX_ALLOWED,
				(uint64_t)TEGRA_ARI_CORE_C7,
				tegra_percpu_data[cpu].wake_time,
				0U);
		if (ret == 0) {
			target = PSCI_LOCAL_STATE_RUN;
		}
	}

	/* CPU off */
	if (target == PLAT_MAX_OFF_STATE) {
		/* Enable cluster powerdn from last CPU in the cluster */
		if (tegra_last_cpu_in_cluster(states, ncpu)) {
			/* Enable CC7 state and turn off wake mask */
			cstate_info.cluster = (uint32_t)TEGRA_ARI_CLUSTER_CC7;
			cstate_info.update_wake_mask = 1;
			mce_update_cstate_info(&cstate_info);

			/* Check if CCx state is allowed. */
			ret = mce_command_handler((uint64_t)MCE_CMD_IS_CCX_ALLOWED,
						  (uint64_t)TEGRA_ARI_CORE_C7,
						  MCE_CORE_SLEEP_TIME_INFINITE,
						  0U);
			if (ret == 0) {
				target = PSCI_LOCAL_STATE_RUN;
			}

		} else {

			/* Turn off wake_mask */
			cstate_info.update_wake_mask = 1;
			mce_update_cstate_info(&cstate_info);
			target = PSCI_LOCAL_STATE_RUN;
		}
	}

	return target;
}

/*******************************************************************************
 * Platform handler to calculate the proper target power level at the
 * specified affinity level
 ******************************************************************************/
plat_local_state_t tegra_soc_get_target_pwr_state(uint32_t lvl,
					     const plat_local_state_t *states,
					     uint32_t ncpu)
{
	plat_local_state_t target = PSCI_LOCAL_STATE_RUN;
	uint32_t cpu = plat_my_core_pos();

	/* System Suspend */
	if ((lvl == (uint32_t)MPIDR_AFFLVL2) &&
	    (states[cpu] == PSTATE_ID_SOC_POWERDN)) {
		target = PSTATE_ID_SOC_POWERDN;
	}

	/* CPU off, CPU suspend */
	if (lvl == (uint32_t)MPIDR_AFFLVL1) {
		target = tegra_get_afflvl1_pwr_state(states, ncpu);
	}

	/* target cluster/system state */
	return target;
}

int32_t tegra_soc_pwr_domain_power_down_wfi(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state =
		target_state->pwr_domain_state;
	const plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint8_t stateid_afflvl2 = pwr_domain_state[PLAT_MAX_PWR_LVL] &
		TEGRA186_STATE_ID_MASK;
	uint64_t val;

	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {
		/*
		 * The TZRAM loses power when we enter system suspend. To
		 * allow graceful exit from system suspend, we need to copy
		 * BL3-1 over to TZDRAM.
		 */
		val = params_from_bl2->tzdram_base +
			tegra186_get_cpu_reset_handler_size();
		memcpy16((void *)(uintptr_t)val, (void *)(uintptr_t)BL31_BASE,
			 (uintptr_t)BL31_END - (uintptr_t)BL31_BASE);
	}

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_suspend_pwrdown_early(const psci_power_state_t *target_state)
{
	return PSCI_E_NOT_SUPPORTED;
}

int32_t tegra_soc_pwr_domain_on(u_register_t mpidr)
{
	int32_t ret = PSCI_E_SUCCESS;
	uint64_t target_cpu = mpidr & MPIDR_CPU_MASK;
	uint64_t target_cluster = (mpidr & MPIDR_CLUSTER_MASK) >>
			MPIDR_AFFINITY_BITS;

	if (target_cluster > ((uint32_t)PLATFORM_CLUSTER_COUNT - 1U)) {

		ERROR("%s: unsupported CPU (0x%lx)\n", __func__, mpidr);
		ret = PSCI_E_NOT_PRESENT;

	} else {
		/* construct the target CPU # */
		target_cpu |= (target_cluster << 2);

		(void)mce_command_handler((uint64_t)MCE_CMD_ONLINE_CORE, target_cpu, 0U, 0U);
	}

	return ret;
}

int32_t tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint8_t stateid_afflvl2 = target_state->pwr_domain_state[PLAT_MAX_PWR_LVL];
	uint8_t stateid_afflvl0 = target_state->pwr_domain_state[MPIDR_AFFLVL0];
	mce_cstate_info_t cstate_info = { 0 };
	uint64_t impl, val;
	const plat_params_from_bl2_t *plat_params = bl31_get_plat_params();

	impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

	/*
	 * Enable ECC and Parity Protection for Cortex-A57 CPUs (Tegra186
	 * A02p and beyond).
	 */
	if ((plat_params->l2_ecc_parity_prot_dis != 1) && (impl != DENVER_IMPL)) {

		val = read_l2ctlr_el1();
		val |= CORTEX_A57_L2_ECC_PARITY_PROTECTION_BIT;
		write_l2ctlr_el1(val);
	}

	/*
	 * Reset power state info for CPUs when onlining, we set
	 * deepest power when offlining a core but that may not be
	 * requested by non-secure sw which controls idle states. It
	 * will re-init this info from non-secure software when the
	 * core come online.
	 */
	if (stateid_afflvl0 == PLAT_MAX_OFF_STATE) {

		cstate_info.cluster = (uint32_t)TEGRA_ARI_CLUSTER_CC1;
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
		cstate_info.cluster = (uint32_t)TEGRA_ARI_CLUSTER_CC7;
		cstate_info.system = (uint32_t)TEGRA_ARI_SYSTEM_SC1;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);
	}

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t impl = (read_midr() >> MIDR_IMPL_SHIFT) & (uint64_t)MIDR_IMPL_MASK;

	(void)target_state;

	/* Disable Denver's DCO operations */
	if (impl == DENVER_IMPL) {
		denver_disable_dco();
	}

	/* Turn off CPU */
	(void)mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE,
			(uint64_t)TEGRA_ARI_CORE_C7, MCE_CORE_SLEEP_TIME_INFINITE, 0U);

	return PSCI_E_SUCCESS;
}

__dead2 void tegra_soc_prepare_system_off(void)
{
	/* power off the entire system */
	mce_enter_ccplex_state((uint32_t)TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_POWER_OFF);

	wfi();

	/* wait for the system to power down */
	for (;;) {
		;
	}
}

int32_t tegra_soc_prepare_system_reset(void)
{
	mce_enter_ccplex_state((uint32_t)TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_REBOOT);

	return PSCI_E_SUCCESS;
}
