/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <denver.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/psci/psci.h>
#include <mce.h>
#include <mce_private.h>
#include <plat/common/platform.h>
#include <se.h>
#include <smmu.h>
#include <t194_nvg.h>
#include <tegra194_private.h>
#include <tegra_platform.h>
#include <tegra_private.h>

extern uint32_t __tegra194_cpu_reset_handler_data,
		__tegra194_cpu_reset_handler_end;

/* TZDRAM offset for saving SMMU context */
#define TEGRA194_SMMU_CTX_OFFSET	16U

/* state id mask */
#define TEGRA194_STATE_ID_MASK		0xFU
/* constants to get power state's wake time */
#define TEGRA194_WAKE_TIME_MASK		0x0FFFFFF0U
#define TEGRA194_WAKE_TIME_SHIFT	4U
/* default core wake mask for CPU_SUSPEND */
#define TEGRA194_CORE_WAKE_MASK		0x180cU

static struct t19x_psci_percpu_data {
	uint32_t wake_time;
} __aligned(CACHE_WRITEBACK_GRANULE) t19x_percpu_data[PLATFORM_CORE_COUNT];

int32_t tegra_soc_validate_power_state(uint32_t power_state,
					psci_power_state_t *req_state)
{
	uint8_t state_id = (uint8_t)psci_get_pstate_id(power_state) &
			   TEGRA194_STATE_ID_MASK;
	uint32_t cpu = plat_my_core_pos();
	int32_t ret = PSCI_E_SUCCESS;

	/* save the core wake time (in TSC ticks)*/
	t19x_percpu_data[cpu].wake_time = (power_state & TEGRA194_WAKE_TIME_MASK)
			<< TEGRA194_WAKE_TIME_SHIFT;

	/*
	 * Clean t19x_percpu_data[cpu] to DRAM. This needs to be done to ensure
	 * that the correct value is read in tegra_soc_pwr_domain_suspend(),
	 * which is called with caches disabled. It is possible to read a stale
	 * value from DRAM in that function, because the L2 cache is not flushed
	 * unless the cluster is entering CC6/CC7.
	 */
	clean_dcache_range((uint64_t)&t19x_percpu_data[cpu],
			sizeof(t19x_percpu_data[cpu]));

	/* Sanity check the requested state id */
	switch (state_id) {
	case PSTATE_ID_CORE_IDLE:

		/* Core idle request */
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_RET_STATE;
		req_state->pwr_domain_state[MPIDR_AFFLVL1] = PSCI_LOCAL_STATE_RUN;
		break;

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
	uint32_t cpu = plat_my_core_pos();
	mce_cstate_info_t cstate_info = { 0 };

	/* Program default wake mask */
	cstate_info.wake_mask = TEGRA194_CORE_WAKE_MASK;
	cstate_info.update_wake_mask = 1;
	mce_update_cstate_info(&cstate_info);

	/* Enter CPU idle */
	(void)mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE,
				  (uint64_t)TEGRA_NVG_CORE_C6,
				  t19x_percpu_data[cpu].wake_time,
				  0U);

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state;
	uint8_t stateid_afflvl0, stateid_afflvl2;
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t smmu_ctx_base;
	uint32_t val;
	mce_cstate_info_t sc7_cstate_info = {
		.cluster = (uint32_t)TEGRA_NVG_CLUSTER_CC6,
		.ccplex = (uint32_t)TEGRA_NVG_CG_CG7,
		.system = (uint32_t)TEGRA_NVG_SYSTEM_SC7,
		.system_state_force = 1U,
		.update_wake_mask = 1U,
	};
	uint32_t cpu = plat_my_core_pos();
	int32_t ret = 0;

	/* get the state ID */
	pwr_domain_state = target_state->pwr_domain_state;
	stateid_afflvl0 = pwr_domain_state[MPIDR_AFFLVL0] &
		TEGRA194_STATE_ID_MASK;
	stateid_afflvl2 = pwr_domain_state[PLAT_MAX_PWR_LVL] &
		TEGRA194_STATE_ID_MASK;

	if ((stateid_afflvl0 == PSTATE_ID_CORE_POWERDN)) {

		/* Enter CPU powerdown */
		(void)mce_command_handler((uint64_t)MCE_CMD_ENTER_CSTATE,
					  (uint64_t)TEGRA_NVG_CORE_C7,
					  t19x_percpu_data[cpu].wake_time,
					  0U);

	} else if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {

		/* save 'Secure Boot' Processor Feature Config Register */
		val = mmio_read_32(TEGRA_MISC_BASE + MISCREG_PFCFG);
		mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_SECURE_BOOTP_FCFG, val);

		/* save SMMU context */
		smmu_ctx_base = params_from_bl2->tzdram_base +
				tegra194_get_smmu_ctx_offset();
		tegra_smmu_save_context((uintptr_t)smmu_ctx_base);

		/*
		 * Suspend SE, RNG1 and PKA1 only on silcon and fpga,
		 * since VDK does not support atomic se ctx save
		 */
		if (tegra_platform_is_silicon() || tegra_platform_is_fpga()) {
			ret = tegra_se_suspend();
			assert(ret == 0);
		}

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

		/* set system suspend state for house-keeping */
		tegra194_set_system_suspend_entry();
	} else {
		; /* do nothing */
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Helper function to check if this is the last ON CPU in the cluster
 ******************************************************************************/
static bool tegra_last_on_cpu_in_cluster(const plat_local_state_t *states,
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
	plat_local_state_t target = states[core_pos];
	mce_cstate_info_t cstate_info = { 0 };

	/* CPU suspend */
	if (target == PSTATE_ID_CORE_POWERDN) {

		/* Program default wake mask */
		cstate_info.wake_mask = TEGRA194_CORE_WAKE_MASK;
		cstate_info.update_wake_mask = 1;
		mce_update_cstate_info(&cstate_info);
	}

	/* CPU off */
	if (target == PLAT_MAX_OFF_STATE) {

		/* Enable cluster powerdn from last CPU in the cluster */
		if (tegra_last_on_cpu_in_cluster(states, ncpu)) {

			/* Enable CC6 state and turn off wake mask */
			cstate_info.cluster = (uint32_t)TEGRA_NVG_CLUSTER_CC6;
			cstate_info.ccplex = (uint32_t)TEGRA_NVG_CG_CG7;
			cstate_info.system_state_force = 1;
			cstate_info.update_wake_mask = 1U;
			mce_update_cstate_info(&cstate_info);

		} else {

			/* Turn off wake_mask */
			cstate_info.update_wake_mask = 1U;
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
	if ((lvl == (uint32_t)MPIDR_AFFLVL2) && (states[cpu] == PSTATE_ID_SOC_POWERDN)) {
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
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint8_t stateid_afflvl2 = pwr_domain_state[PLAT_MAX_PWR_LVL] &
		TEGRA194_STATE_ID_MASK;
	uint64_t val;

	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {
		/*
		 * The TZRAM loses power when we enter system suspend. To
		 * allow graceful exit from system suspend, we need to copy
		 * BL3-1 over to TZDRAM.
		 */
		val = params_from_bl2->tzdram_base +
		      tegra194_get_cpu_reset_handler_size();
		memcpy((void *)(uintptr_t)val, (void *)(uintptr_t)BL31_BASE,
		       (uintptr_t)&__BL31_END__ - (uintptr_t)BL31_BASE);
	}

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_suspend_pwrdown_early(const psci_power_state_t *target_state)
{
	return PSCI_E_NOT_SUPPORTED;
}

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

#if ENABLE_STRICT_CHECKING_MODE
		/*
		 * Enable strict checking after programming the GSC for
		 * enabling TZSRAM and TZDRAM
		 */
		mce_enable_strict_checking();
#endif

		/* Init SMMU */
		tegra_smmu_init();

		/* Resume SE, RNG1 and PKA1 */
		tegra_se_resume();

		/*
		 * Program XUSB STREAMIDs
		 * ======================
		 * T19x XUSB has support for XUSB virtualization. It will
		 * have one physical function (PF) and four Virtual functions
		 * (VF)
		 *
		 * There were below two SIDs for XUSB until T186.
		 * 1) #define TEGRA_SID_XUSB_HOST    0x1bU
		 * 2) #define TEGRA_SID_XUSB_DEV    0x1cU
		 *
		 * We have below four new SIDs added for VF(s)
		 * 3) #define TEGRA_SID_XUSB_VF0    0x5dU
		 * 4) #define TEGRA_SID_XUSB_VF1    0x5eU
		 * 5) #define TEGRA_SID_XUSB_VF2    0x5fU
		 * 6) #define TEGRA_SID_XUSB_VF3    0x60U
		 *
		 * When virtualization is enabled then we have to disable SID
		 * override and program above SIDs in below newly added SID
		 * registers in XUSB PADCTL MMIO space. These registers are
		 * TZ protected and so need to be done in ATF.
		 *
		 * a) #define XUSB_PADCTL_HOST_AXI_STREAMID_PF_0 (0x136cU)
		 * b) #define XUSB_PADCTL_DEV_AXI_STREAMID_PF_0  (0x139cU)
		 * c) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_0 (0x1370U)
		 * d) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_1 (0x1374U)
		 * e) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_2 (0x1378U)
		 * f) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_3 (0x137cU)
		 *
		 * This change disables SID override and programs XUSB SIDs
		 * in above registers to support both virtualization and
		 * non-virtualization platforms
		 */
		if (tegra_platform_is_silicon() || tegra_platform_is_fpga()) {

			mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
				XUSB_PADCTL_HOST_AXI_STREAMID_PF_0, TEGRA_SID_XUSB_HOST);
			mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
				XUSB_PADCTL_HOST_AXI_STREAMID_VF_0, TEGRA_SID_XUSB_VF0);
			mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
				XUSB_PADCTL_HOST_AXI_STREAMID_VF_1, TEGRA_SID_XUSB_VF1);
			mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
				XUSB_PADCTL_HOST_AXI_STREAMID_VF_2, TEGRA_SID_XUSB_VF2);
			mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
				XUSB_PADCTL_HOST_AXI_STREAMID_VF_3, TEGRA_SID_XUSB_VF3);
			mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
				XUSB_PADCTL_DEV_AXI_STREAMID_PF_0, TEGRA_SID_XUSB_DEV);
		}

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
	mce_system_shutdown();

	wfi();

	/* wait for the system to power down */
	for (;;) {
		;
	}
}

int32_t tegra_soc_prepare_system_reset(void)
{
	/* System reboot */
	mce_system_reboot();

	return PSCI_E_SUCCESS;
}
