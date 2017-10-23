/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <common/debug.h>
#include <denver.h>
#include <mce.h>
#include <plat/common/platform.h>
#include <lib/psci/psci.h>
#include <smmu.h>
#include <string.h>
#include <tegra_private.h>
#include <t194_nvg.h>
#include <stdbool.h>

extern void tegra_secure_entrypoint(void);

#if ENABLE_SYSTEM_SUSPEND_CTX_SAVE_TZDRAM
extern void tegra186_cpu_reset_handler(void);
extern uint32_t __tegra186_cpu_reset_handler_data,
		__tegra186_cpu_reset_handler_end;

/* TZDRAM offset for saving SMMU context */
#define TEGRA186_SMMU_CTX_OFFSET	16U
#endif

/* state id mask */
#define TEGRA186_STATE_ID_MASK		0xFU
/* constants to get power state's wake time */
#define TEGRA186_WAKE_TIME_MASK		0x0FFFFFF0U
#define TEGRA186_WAKE_TIME_SHIFT	4U
/* default core wake mask for CPU_SUSPEND */
#define TEGRA194_CORE_WAKE_MASK		0x180cU
/* context size to save during system suspend */
#define TEGRA186_SE_CONTEXT_SIZE	3U

static uint32_t se_regs[TEGRA186_SE_CONTEXT_SIZE];
static struct t19x_psci_percpu_data {
	uint32_t wake_time;
} __aligned(CACHE_WRITEBACK_GRANULE) t19x_percpu_data[PLATFORM_CORE_COUNT];

/*
 * tegra_fake_system_suspend acts as a boolean var controlling whether
 * we are going to take fake system suspend code or normal system suspend code
 * path. This variable is set inside the sip call handlers, when the kernel
 * requests an SIP call to set the suspend debug flags.
 */
bool tegra_fake_system_suspend;

int32_t tegra_soc_validate_power_state(uint32_t power_state,
					psci_power_state_t *req_state)
{
	uint8_t state_id = (uint8_t)psci_get_pstate_id(power_state) &
			   TEGRA186_STATE_ID_MASK;
	uint32_t cpu = plat_my_core_pos();
	int32_t ret = PSCI_E_SUCCESS;

	/* save the core wake time (in TSC ticks)*/
	t19x_percpu_data[cpu].wake_time = (power_state & TEGRA186_WAKE_TIME_MASK)
			<< TEGRA186_WAKE_TIME_SHIFT;

	/*
	 * Clean percpu_data[cpu] to DRAM. This needs to be done to ensure that
	 * the correct value is read in tegra_soc_pwr_domain_suspend(), which
	 * is called with caches disabled. It is possible to read a stale value
	 * from DRAM in that function, because the L2 cache is not flushed
	 * unless the cluster is entering CC6/CC7.
	 */
	clean_dcache_range((uint64_t)&t19x_percpu_data[cpu],
			sizeof(t19x_percpu_data[cpu]));

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

int32_t tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state;
	uint8_t stateid_afflvl0, stateid_afflvl2;
#if ENABLE_SYSTEM_SUSPEND_CTX_SAVE_TZDRAM
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t smmu_ctx_base;
#endif
	uint32_t val;
	mce_cstate_info_t sc7_cstate_info = {
		.cluster = (uint32_t)TEGRA_NVG_CLUSTER_CC6,
		.system = (uint32_t)TEGRA_NVG_SYSTEM_SC7,
		.system_state_force = 1U,
		.update_wake_mask = 1U,
	};
	uint32_t cpu = plat_my_core_pos();
	int32_t ret = 0;

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
			(uint32_t)TEGRA_NVG_CORE_C6 : (uint32_t)TEGRA_NVG_CORE_C7;
		ret = mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE, (uint64_t)val,
				percpu_data[cpu].wake_time, 0);
		assert(ret == 0);

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

#if ENABLE_SYSTEM_SUSPEND_CTX_SAVE_TZDRAM
		/* save SMMU context */
		smmu_ctx_base = params_from_bl2->tzdram_base +
			((uintptr_t)&__tegra186_cpu_reset_handler_data -
			 (uintptr_t)&tegra186_cpu_reset_handler) +
			TEGRA186_SMMU_CTX_OFFSET;
		tegra_smmu_save_context((uintptr_t)smmu_ctx_base);
#else
		tegra_smmu_save_context(0);
#endif

		if (!tegra_fake_system_suspend) {

			/* Prepare for system suspend */
			mce_update_cstate_info(&sc7_cstate_info);

			do {
				val = (uint32_t)mce_command_handler(
						(uint32_t)MCE_CMD_IS_SC7_ALLOWED,
						(uint32_t)TEGRA_NVG_CORE_C7,
						MCE_CORE_SLEEP_TIME_INFINITE,
						0U);
			} while (val == 0U);

			/* Instruct the MCE to enter system suspend state */
			ret = mce_command_handler(
					(uint64_t)MCE_CMD_ENTER_CSTATE,
					(uint64_t)TEGRA_NVG_CORE_C7,
					MCE_CORE_SLEEP_TIME_INFINITE,
					0U);
			assert(ret == 0);
		}
	} else {
		; /* do nothing */
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler to calculate the proper target power level at the
 * specified affinity level
 ******************************************************************************/
plat_local_state_t tegra_soc_get_target_pwr_state(uint32_t lvl,
					     const plat_local_state_t *states,
					     uint32_t ncpu)
{
	plat_local_state_t target = *states;
	int32_t cluster_powerdn = 1;
	uint32_t core_pos = (uint32_t)read_mpidr() & MPIDR_CPU_MASK;
	uint32_t num_cpus = ncpu, pos = 0;
	mce_cstate_info_t cstate_info = { 0 };

	/* get the current core's power state */
	target = states[core_pos];

	/* CPU suspend */
	if ((lvl == MPIDR_AFFLVL1) && (target == PSTATE_ID_CORE_POWERDN)) {

		/* Program default wake mask */
		cstate_info.wake_mask = TEGRA194_CORE_WAKE_MASK;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);
	}

	/* CPU off */
	if ((lvl == MPIDR_AFFLVL1) && (target == PLAT_MAX_OFF_STATE)) {

		/* find out the number of ON cpus in the cluster */
		do {
			target = states[pos];
			if (target != PLAT_MAX_OFF_STATE) {
				cluster_powerdn = 0;
			}
			--num_cpus;
			pos++;
		} while (num_cpus != 0U);

		/* Enable cluster powerdn from last CPU in the cluster */
		if (cluster_powerdn != 0) {

			/* Enable CC6 */
			/* todo */

			/* If cluster group needs to be railgated, request CG7 */
			/* todo */

			/* Turn off wake mask */
			cstate_info.update_wake_mask = 1U;
			mce_update_cstate_info(&cstate_info);

		} else {
			/* Turn off wake_mask */
			cstate_info.update_wake_mask = 1U;
			mce_update_cstate_info(&cstate_info);
		}
	}

	/* System Suspend */
	if ((lvl == MPIDR_AFFLVL2) || (target == PSTATE_ID_SOC_POWERDN)) {
		return PSTATE_ID_SOC_POWERDN;
	}

	/* default state */
	return PSCI_LOCAL_STATE_RUN;
}

#if ENABLE_SYSTEM_SUSPEND_CTX_SAVE_TZDRAM
int32_t tegra_soc_pwr_domain_power_down_wfi(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state =
		target_state->pwr_domain_state;
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint8_t stateid_afflvl2 = pwr_domain_state[PLAT_MAX_PWR_LVL] &
		TEGRA186_STATE_ID_MASK;
	uint64_t val;
	u_register_t ns_sctlr_el1;

	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {
		/*
		 * The TZRAM loses power when we enter system suspend. To
		 * allow graceful exit from system suspend, we need to copy
		 * BL3-1 over to TZDRAM.
		 */
		val = params_from_bl2->tzdram_base +
			((uintptr_t)&__tegra186_cpu_reset_handler_end -
			 (uintptr_t)tegra186_cpu_reset_handler);
		memcpy((void *)(uintptr_t)val, (void *)(uintptr_t)BL31_BASE,
		       (uintptr_t)&__BL31_END__ - (uintptr_t)BL31_BASE);

		/*
		 * In fake suspend mode, ensure that the loopback procedure
		 * towards system suspend exit is started, instead of calling
		 * WFI. This is done by disabling both MMU's of EL1 & El3
		 * and calling tegra_secure_entrypoint().
		 */
		if (tegra_fake_system_suspend) {

			/*
			 * Disable EL1's MMU.
			 */
			ns_sctlr_el1 = read_sctlr_el1();
			ns_sctlr_el1 &= (~((u_register_t)SCTLR_M_BIT));
			write_sctlr_el1(ns_sctlr_el1);

			/*
			 * Disable MMU to power up the CPU in a "clean"
			 * state
			 */
			disable_mmu_el3();
			tegra_secure_entrypoint();
			panic();
		}
	}

	return PSCI_E_SUCCESS;
}
#endif

int32_t tegra_soc_pwr_domain_on(u_register_t mpidr)
{
	uint64_t target_cpu = mpidr & MPIDR_CPU_MASK;
	uint64_t target_cluster = (mpidr & MPIDR_CLUSTER_MASK) >>
			MPIDR_AFFINITY_BITS;
	int32_t ret = 0;

	if (target_cluster > ((uint32_t)PLATFORM_CLUSTER_COUNT - 1U)) {
		ERROR("%s: unsupported CPU (0x%lx)\n", __func__ , mpidr);
		return PSCI_E_NOT_PRESENT;
	}

	/* construct the target CPU # */
	target_cpu += (target_cluster << 1U);

	ret = mce_command_handler((uint64_t)MCE_CMD_ONLINE_CORE, target_cpu, 0U, 0U);
	if (ret < 0) {
		return PSCI_E_DENIED;
	}

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint8_t stateid_afflvl2 = target_state->pwr_domain_state[PLAT_MAX_PWR_LVL];

	/*
	 * Reset power state info for CPUs when onlining, we set
	 * deepest power when offlining a core but that may not be
	 * requested by non-secure sw which controls idle states. It
	 * will re-init this info from non-secure software when the
	 * core come online.
	 */

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
	}

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;
	int32_t ret = 0;

	(void)target_state;

	/* Disable Denver's DCO operations */
	if (impl == DENVER_IMPL) {
		denver_disable_dco();
	}

	/* Turn off CPU */
	ret = mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE,
			(uint64_t)TEGRA_NVG_CORE_C7, MCE_CORE_SLEEP_TIME_INFINITE, 0U);
	assert(ret == 0);

	return PSCI_E_SUCCESS;
}

__dead2 void tegra_soc_prepare_system_off(void)
{
	/* System power off */

	/* SC8 */

	wfi();

	/* wait for the system to power down */
	for (;;) {
		;
	}
}

int32_t tegra_soc_prepare_system_reset(void)
{
	return PSCI_E_SUCCESS;
}
