/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cortex_a57.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <bpmp.h>
#include <flowctrl.h>
#include <pmc.h>
#include <platform_def.h>
#include <security_engine.h>
#include <tegra_def.h>
#include <tegra_private.h>
#include <tegra_platform.h>

/*
 * Register used to clear CPU reset signals. Each CPU has two reset
 * signals: CPU reset (3:0) and Core reset (19:16).
 */
#define CPU_CMPLX_RESET_CLR		0x454
#define CPU_CORE_RESET_MASK		0x10001

/* Clock and Reset controller registers for system clock's settings */
#define SCLK_RATE			0x30
#define SCLK_BURST_POLICY		0x28
#define SCLK_BURST_POLICY_DEFAULT	0x10000000

static int cpu_powergate_mask[PLATFORM_MAX_CPUS_PER_CLUSTER];

int32_t tegra_soc_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int state_id = psci_get_pstate_id(power_state);

	/* Sanity check the requested state id */
	switch (state_id) {
	case PSTATE_ID_CORE_POWERDN:
		/*
		 * Core powerdown request only for afflvl 0
		 */
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = state_id & 0xff;

		break;

	case PSTATE_ID_CLUSTER_IDLE:
	case PSTATE_ID_CLUSTER_POWERDN:
		/*
		 * Cluster powerdown/idle request only for afflvl 1
		 */
		req_state->pwr_domain_state[MPIDR_AFFLVL1] = state_id;
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PSTATE_ID_CORE_POWERDN;

		break;

	case PSTATE_ID_SOC_POWERDN:
		/*
		 * System powerdown request only for afflvl 2
		 */
		for (uint32_t i = MPIDR_AFFLVL0; i < PLAT_MAX_PWR_LVL; i++)
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;

		req_state->pwr_domain_state[PLAT_MAX_PWR_LVL] =
			PLAT_SYS_SUSPEND_STATE_ID;

		break;

	default:
		ERROR("%s: unsupported state id (%d)\n", __func__, state_id);
		return PSCI_E_INVALID_PARAMS;
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
	plat_local_state_t target = PSCI_LOCAL_STATE_RUN;
	int cpu = plat_my_core_pos();
	int core_pos = read_mpidr() & MPIDR_CPU_MASK;
	uint32_t bpmp_reply, data[3];
	int ret;

	/* get the power state at this level */
	if (lvl == MPIDR_AFFLVL1)
		target = *(states + core_pos);
	if (lvl == MPIDR_AFFLVL2)
		target = *(states + cpu);

	if ((lvl == MPIDR_AFFLVL1) && (target == PSTATE_ID_CLUSTER_IDLE)) {

		/* initialize the bpmp interface */
		(void)tegra_bpmp_init();

		/* Cluster idle */
		data[0] = (uint32_t)cpu;
		data[1] = TEGRA_PM_CC6;
		data[2] = TEGRA_PM_SC1;
		ret = tegra_bpmp_send_receive_atomic(MRQ_DO_IDLE,
				(void *)&data, (int)sizeof(data),
				(void *)&bpmp_reply, (int)sizeof(bpmp_reply));

		/* check if cluster idle entry is allowed */
		if ((ret != 0L) || (bpmp_reply != BPMP_CCx_ALLOWED)) {

			/* Cluster idle not allowed */
			target = PSCI_LOCAL_STATE_RUN;
		}

	} else if ((lvl == MPIDR_AFFLVL1) && (target == PSTATE_ID_CLUSTER_POWERDN)) {

		/* initialize the bpmp interface */
		(void)tegra_bpmp_init();

		/* Cluster power-down */
		data[0] = (uint32_t)cpu;
		data[1] = TEGRA_PM_CC7;
		data[2] = TEGRA_PM_SC1;
		ret = tegra_bpmp_send_receive_atomic(MRQ_DO_IDLE,
				(void *)&data, (int)sizeof(data),
				(void *)&bpmp_reply, (int)sizeof(bpmp_reply));

		/* check if cluster power down is allowed */
		if ((ret != 0L) || (bpmp_reply != BPMP_CCx_ALLOWED)) {

			/* Cluster power down not allowed */
			target = PSCI_LOCAL_STATE_RUN;
		}

	} else if (((lvl == MPIDR_AFFLVL2) || (lvl == MPIDR_AFFLVL1)) &&
	    (target == PSTATE_ID_SOC_POWERDN)) {

		/* System Suspend */
		target = PSTATE_ID_SOC_POWERDN;

	} else {
		; /* do nothing */
	}

	return target;
}

int tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr();
	const plat_local_state_t *pwr_domain_state =
		target_state->pwr_domain_state;
	unsigned int stateid_afflvl2 = pwr_domain_state[MPIDR_AFFLVL2];
	unsigned int stateid_afflvl1 = pwr_domain_state[MPIDR_AFFLVL1];
	unsigned int stateid_afflvl0 = pwr_domain_state[MPIDR_AFFLVL0];
	int ret = PSCI_E_SUCCESS;

	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {

		assert((stateid_afflvl0 == PLAT_MAX_OFF_STATE) ||
			(stateid_afflvl0 == PSTATE_ID_SOC_POWERDN));
		assert((stateid_afflvl1 == PLAT_MAX_OFF_STATE) ||
			(stateid_afflvl1 == PSTATE_ID_SOC_POWERDN));

		if (tegra_chipid_is_t210_b01()) {

			/* Suspend se/se2 and pka1 */
			if (tegra_se_suspend() != 0) {
				ret = PSCI_E_INTERN_FAIL;
			}

			/* Save tzram contents */
			if (tegra_se_save_tzram() != 0) {
				ret = PSCI_E_INTERN_FAIL;
			}
		}

		/* enter system suspend */
		if (ret == PSCI_E_SUCCESS) {
			tegra_fc_soc_powerdn(mpidr);
		}

	} else if (stateid_afflvl1 == PSTATE_ID_CLUSTER_IDLE) {

		assert(stateid_afflvl0 == PSTATE_ID_CORE_POWERDN);

		/* Prepare for cluster idle */
		tegra_fc_cluster_idle(mpidr);

	} else if (stateid_afflvl1 == PSTATE_ID_CLUSTER_POWERDN) {

		assert(stateid_afflvl0 == PSTATE_ID_CORE_POWERDN);

		/* Prepare for cluster powerdn */
		tegra_fc_cluster_powerdn(mpidr);

	} else if (stateid_afflvl0 == PSTATE_ID_CORE_POWERDN) {

		/* Prepare for cpu powerdn */
		tegra_fc_cpu_powerdn(mpidr);

	} else {
		ERROR("%s: Unknown state id (%d, %d, %d)\n", __func__,
			stateid_afflvl2, stateid_afflvl1, stateid_afflvl0);
		ret = PSCI_E_NOT_SUPPORTED;
	}

	return ret;
}

int tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	const plat_params_from_bl2_t *plat_params = bl31_get_plat_params();
	uint32_t val;

	/* platform parameter passed by the previous bootloader */
	if (plat_params->l2_ecc_parity_prot_dis != 1) {
		/* Enable ECC Parity Protection for Cortex-A57 CPUs */
		val = read_l2ctlr_el1();
		val |= (uint64_t)CORTEX_A57_L2_ECC_PARITY_PROTECTION_BIT;
		write_l2ctlr_el1(val);
	}

	/*
	 * Check if we are exiting from SOC_POWERDN.
	 */
	if (target_state->pwr_domain_state[PLAT_MAX_PWR_LVL] ==
			PLAT_SYS_SUSPEND_STATE_ID) {

		/*
		 * Security engine resume
		 */
		if (tegra_chipid_is_t210_b01()) {
			tegra_se_resume();
		}

		/*
		 * Lock scratch registers which hold the CPU vectors
		 */
		tegra_pmc_lock_cpu_vectors();

		/*
		 * Enable WRAP to INCR burst type conversions for
		 * incoming requests on the AXI slave ports.
		 */
		val = mmio_read_32(TEGRA_MSELECT_BASE + MSELECT_CONFIG);
		val &= ~ENABLE_UNSUP_TX_ERRORS;
		val |= ENABLE_WRAP_TO_INCR_BURSTS;
		mmio_write_32(TEGRA_MSELECT_BASE + MSELECT_CONFIG, val);

		/*
		 * Restore Boot and Power Management Processor (BPMP) reset
		 * address and reset it.
		 */
		tegra_fc_reset_bpmp();
	}

	/*
	 * T210 has a dedicated ARMv7 boot and power mgmt processor, BPMP. It's
	 * used for power management and boot purposes. Inform the BPMP that
	 * we have completed the cluster power up.
	 */
	tegra_fc_lock_active_cluster();

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_on(u_register_t mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;
	uint32_t mask = CPU_CORE_RESET_MASK << cpu;

	/* Deassert CPU reset signals */
	mmio_write_32(TEGRA_CAR_RESET_BASE + CPU_CMPLX_RESET_CLR, mask);

	/* Turn on CPU using flow controller or PMC */
	if (cpu_powergate_mask[cpu] == 0) {
		tegra_pmc_cpu_on(cpu);
		cpu_powergate_mask[cpu] = 1;
	} else {
		tegra_fc_cpu_on(cpu);
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	tegra_fc_cpu_off(read_mpidr() & MPIDR_CPU_MASK);
	return PSCI_E_SUCCESS;
}

int tegra_soc_prepare_system_reset(void)
{
	/*
	 * Set System Clock (SCLK) to POR default so that the clock source
	 * for the PMC APB clock would not be changed due to system reset.
	 */
	mmio_write_32((uintptr_t)TEGRA_CAR_RESET_BASE + SCLK_BURST_POLICY,
		SCLK_BURST_POLICY_DEFAULT);
	mmio_write_32((uintptr_t)TEGRA_CAR_RESET_BASE + SCLK_RATE, 0);

	/* Wait 1 ms to make sure clock source/device logic is stabilized. */
	mdelay(1);

	return PSCI_E_SUCCESS;
}
