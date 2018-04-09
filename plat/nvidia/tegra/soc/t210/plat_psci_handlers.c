/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
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
#include <lib/utils.h>
#include <memctrl.h>
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
static bool tegra_bpmp_available = true;

int32_t tegra_soc_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int state_id = psci_get_pstate_id(power_state);
	const plat_params_from_bl2_t *plat_params = bl31_get_plat_params();

	/* Sanity check the requested state id */
	switch (state_id) {
	case PSTATE_ID_CORE_POWERDN:
		/*
		 * Core powerdown request only for afflvl 0
		 */
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = state_id & 0xff;

		break;

	case PSTATE_ID_CLUSTER_IDLE:

		/*
		 * Cluster idle request for afflvl 0
		 */
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PSTATE_ID_CORE_POWERDN;
		req_state->pwr_domain_state[MPIDR_AFFLVL1] = state_id;
		break;

	case PSTATE_ID_SOC_POWERDN:

		/*
		 * sc7entry-fw must be present in the system when the bpmp
		 * firmware is not present, for a successful System Suspend
		 * entry.
		 */
		if (!tegra_bpmp_init() && !plat_params->sc7entry_fw_base)
			return PSCI_E_NOT_SUPPORTED;

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
 * specified affinity level.
 ******************************************************************************/
plat_local_state_t tegra_soc_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	plat_local_state_t target = PSCI_LOCAL_STATE_RUN;
	int cpu = plat_my_core_pos();
	int core_pos = read_mpidr() & MPIDR_CPU_MASK;
	uint32_t bpmp_reply, data[3], val;
	int ret;

	/* get the power state at this level */
	if (lvl == MPIDR_AFFLVL1)
		target = *(states + core_pos);
	if (lvl == MPIDR_AFFLVL2)
		target = *(states + cpu);

	if ((lvl == MPIDR_AFFLVL1) && (target == PSTATE_ID_CLUSTER_IDLE)) {

		/* initialize the bpmp interface */
		ret = tegra_bpmp_init();
		if (ret != 0U) {

			/*
			 * flag to indicate that BPMP firmware is not
			 * available and the CPU has to handle entry/exit
			 * for all power states
			 */
			tegra_bpmp_available = false;

			/* Cluster idle not allowed */
			target = PSCI_LOCAL_STATE_RUN;

			/*******************************************
			 * BPMP is not present, so handle CC6 entry
			 * from the CPU
			 ******************************************/

			/* check if cluster idle state has been enabled */
			val = mmio_read_32(TEGRA_CL_DVFS_BASE + DVFS_DFLL_CTRL);
			if (val == ENABLE_CLOSED_LOOP) {
				/*
				 * Acquire the cluster idle lock to stop
				 * other CPUs from powering up.
				 */
				tegra_fc_ccplex_pgexit_lock();

				/* Cluster idle only from the last standing CPU */
				if (tegra_pmc_is_last_on_cpu() && tegra_fc_is_ccx_allowed()) {
					/* Cluster idle allowed */
					target = PSTATE_ID_CLUSTER_IDLE;
				} else {
					/* release cluster idle lock */
					tegra_fc_ccplex_pgexit_unlock();
				}
			}
		} else {

			/* Cluster power-down */
			data[0] = (uint32_t)cpu;
			data[1] = TEGRA_PM_CC6;
			data[2] = TEGRA_PM_SC1;
			ret = tegra_bpmp_send_receive_atomic(MRQ_DO_IDLE,
					(void *)&data, (int)sizeof(data),
					(void *)&bpmp_reply,
					(int)sizeof(bpmp_reply));

			/* check if cluster power down is allowed */
			if ((ret != 0L) || (bpmp_reply != BPMP_CCx_ALLOWED)) {

				/* Cluster power down not allowed */
				target = PSCI_LOCAL_STATE_RUN;
			}
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

int32_t tegra_soc_cpu_standby(plat_local_state_t cpu_state)
{
	(void)cpu_state;
	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr();
	const plat_local_state_t *pwr_domain_state =
		target_state->pwr_domain_state;
	unsigned int stateid_afflvl2 = pwr_domain_state[MPIDR_AFFLVL2];
	unsigned int stateid_afflvl1 = pwr_domain_state[MPIDR_AFFLVL1];
	unsigned int stateid_afflvl0 = pwr_domain_state[MPIDR_AFFLVL0];
	uint32_t cfg;
	int ret = PSCI_E_SUCCESS;
	uint32_t val;

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
		}

	} else if (stateid_afflvl1 == PSTATE_ID_CLUSTER_IDLE) {

		assert(stateid_afflvl0 == PSTATE_ID_CORE_POWERDN);

		if (!tegra_bpmp_available) {

			/*
			 * When disabled, DFLL loses its state. Enable
			 * open loop state for the DFLL as we dont want
			 * garbage values being written to the pmic
			 * when we enter cluster idle state.
			 */
			mmio_write_32(TEGRA_CL_DVFS_BASE + DVFS_DFLL_CTRL,
				      ENABLE_OPEN_LOOP);

			/* Find if the platform uses OVR2/MAX77621 PMIC */
			cfg = mmio_read_32(TEGRA_CL_DVFS_BASE + DVFS_DFLL_OUTPUT_CFG);
			if (cfg & DFLL_OUTPUT_CFG_CLK_EN_BIT) {
				/* OVR2 */

				/* PWM tristate */
				val = mmio_read_32(TEGRA_MISC_BASE + PINMUX_AUX_DVFS_PWM);
				val |= PINMUX_PWM_TRISTATE;
				mmio_write_32(TEGRA_MISC_BASE + PINMUX_AUX_DVFS_PWM, val);

				/*
				 * SCRATCH201[1] is being used to identify CPU
				 * PMIC in warmboot code.
				 * 0 : OVR2
				 * 1 : MAX77621
				 */
				tegra_pmc_write_32(PMC_SCRATCH201, 0x0);
			} else {
				/* MAX77621 */
				tegra_pmc_write_32(PMC_SCRATCH201, 0x2);
			}
		}

		/* Prepare for cluster idle */
		tegra_fc_cluster_idle(mpidr);

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

static void tegra_reset_all_dma_masters(void)
{
	uint32_t val, mask;

	/*
	 * Reset all possible DMA masters in the system.
	 */
	val = GPU_RESET_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_GPU_RESET_REG_OFFSET, val);

	val = NVENC_RESET_BIT | TSECB_RESET_BIT | APE_RESET_BIT |
	      NVJPG_RESET_BIT | NVDEC_RESET_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_Y, val);

	val = HOST1X_RESET_BIT | ISP_RESET_BIT | USBD_RESET_BIT |
	      VI_RESET_BIT | SDMMC4_RESET_BIT | SDMMC1_RESET_BIT |
	      SDMMC2_RESET_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_L, val);

	val = USB2_RESET_BIT | APBDMA_RESET_BIT | AHBDMA_RESET_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_H, val);

	val = XUSB_DEV_RESET_BIT | XUSB_HOST_RESET_BIT | TSEC_RESET_BIT |
	      PCIE_RESET_BIT | SDMMC3_RESET_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_U, val);

	val = SE_RESET_BIT | HDA_RESET_BIT | SATA_RESET_BIT;
	mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_V, val);

	/*
	 * If any of the DMA masters are still alive, assume
	 * that the system has been compromised and reboot.
	 */
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_GPU_RESET_REG_OFFSET);
	mask = GPU_RESET_BIT;
	if ((val & mask) != mask)
		tegra_pmc_system_reset();

	mask = NVENC_RESET_BIT | TSECB_RESET_BIT | APE_RESET_BIT |
	      NVJPG_RESET_BIT | NVDEC_RESET_BIT;
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_Y);
	if ((val & mask) != mask)
		tegra_pmc_system_reset();

	mask = HOST1X_RESET_BIT | ISP_RESET_BIT | USBD_RESET_BIT |
	       VI_RESET_BIT | SDMMC4_RESET_BIT | SDMMC1_RESET_BIT |
	       SDMMC2_RESET_BIT;
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_L);
	if ((val & mask) != mask)
		tegra_pmc_system_reset();

	mask = USB2_RESET_BIT | APBDMA_RESET_BIT | AHBDMA_RESET_BIT;
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_H);
	if ((val & mask) != mask)
		tegra_pmc_system_reset();

	mask = XUSB_DEV_RESET_BIT | XUSB_HOST_RESET_BIT | TSEC_RESET_BIT |
	       PCIE_RESET_BIT | SDMMC3_RESET_BIT;
	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_U);
	if ((val & mask) != mask)
		tegra_pmc_system_reset();

	val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_SET_V);
	mask = SE_RESET_BIT | HDA_RESET_BIT | SATA_RESET_BIT;
	if ((val & mask) != mask)
		tegra_pmc_system_reset();
}

int tegra_soc_pwr_domain_power_down_wfi(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr();
	const plat_local_state_t *pwr_domain_state =
		target_state->pwr_domain_state;
	unsigned int stateid_afflvl2 = pwr_domain_state[PLAT_MAX_PWR_LVL];
	const plat_params_from_bl2_t *plat_params = bl31_get_plat_params();
	uint32_t val;

	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {

		if (tegra_chipid_is_t210_b01()) {
			/* Save tzram contents */
			tegra_se_save_tzram();
		}

		/* de-init the interface */
		tegra_bpmp_suspend();

		/*
		 * The CPU needs to load the System suspend entry firmware
		 * if nothing is running on the BPMP.
		 */
		if (!tegra_bpmp_available) {

			/*
			 * BPMP firmware is not running on the co-processor, so
			 * we need to explicitly load the firmware to enable
			 * entry/exit to/from System Suspend and set the BPMP
			 * on its way.
			 */

			/* Power off BPMP before we proceed */
			tegra_fc_bpmp_off();

			/* bond out IRAM banks B, C and D */
			mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_BOND_OUT_U,
				IRAM_B_LOCK_BIT | IRAM_C_LOCK_BIT |
				IRAM_D_LOCK_BIT);

			/* bond out APB/AHB DMAs */
			mmio_write_32(TEGRA_CAR_RESET_BASE + TEGRA_BOND_OUT_H,
				APB_DMA_LOCK_BIT | AHB_DMA_LOCK_BIT);

			/* Power off BPMP before we proceed */
			tegra_fc_bpmp_off();

			/*
			 * Reset all the hardware blocks that can act as DMA
			 * masters on the bus.
			 */
			tegra_reset_all_dma_masters();

			/*
			 * Mark PMC as accessible to the non-secure world
			 * to allow the COP to execute System Suspend
			 * sequence
			 */
			val = mmio_read_32(TEGRA_MISC_BASE + APB_SLAVE_SECURITY_ENABLE);
			val &= ~PMC_SECURITY_EN_BIT;
			mmio_write_32(TEGRA_MISC_BASE + APB_SLAVE_SECURITY_ENABLE, val);

			/* clean up IRAM of any cruft */
			zeromem((void *)(uintptr_t)TEGRA_IRAM_BASE,
					TEGRA_IRAM_A_SIZE);

			/* Copy the firmware to BPMP's internal RAM */
			(void)memcpy((void *)(uintptr_t)TEGRA_IRAM_BASE,
				(const void *)(plat_params->sc7entry_fw_base + SC7ENTRY_FW_HEADER_SIZE_BYTES),
				plat_params->sc7entry_fw_size - SC7ENTRY_FW_HEADER_SIZE_BYTES);

			/* Power on the BPMP and execute from IRAM base */
			tegra_fc_bpmp_on(TEGRA_IRAM_BASE);

			/* Wait until BPMP powers up */
			do {
				val = mmio_read_32(TEGRA_RES_SEMA_BASE + STA_OFFSET);
			} while (val != SIGN_OF_LIFE);
		}

		/* enter system suspend */
		tegra_fc_soc_powerdn(mpidr);
	}

	return PSCI_E_SUCCESS;
}

int32_t tegra_soc_pwr_domain_suspend_pwrdown_early(const psci_power_state_t *target_state)
{
	return PSCI_E_NOT_SUPPORTED;
}

int tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	const plat_params_from_bl2_t *plat_params = bl31_get_plat_params();
	uint32_t cfg;
	uint32_t val, entrypoint = 0;
	uint64_t offset;

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
		 * address and reset it, if it is supported by the platform.
		 */
		if (!tegra_bpmp_available) {
			tegra_fc_bpmp_off();
		} else {
			entrypoint = tegra_pmc_read_32(PMC_SCRATCH39);
			tegra_fc_bpmp_on(entrypoint);

			/* initialise the interface */
			tegra_bpmp_resume();
		}

		if (plat_params->sc7entry_fw_base != 0U) {
			/* sc7entry-fw is part of TZDRAM area */
			offset = plat_params->tzdram_base - plat_params->sc7entry_fw_base;
			tegra_memctrl_tzdram_setup(plat_params->sc7entry_fw_base,
				plat_params->tzdram_size + offset);
		}

		if (!tegra_chipid_is_t210_b01()) {
			/* restrict PMC access to secure world */
			val = mmio_read_32(TEGRA_MISC_BASE + APB_SLAVE_SECURITY_ENABLE);
			val |= PMC_SECURITY_EN_BIT;
			mmio_write_32(TEGRA_MISC_BASE + APB_SLAVE_SECURITY_ENABLE, val);
		}
	}

	/*
	 * Check if we are exiting cluster idle state
	 */
	if (target_state->pwr_domain_state[MPIDR_AFFLVL1] ==
			PSTATE_ID_CLUSTER_IDLE) {

		if (!tegra_bpmp_available) {

			/* PWM un-tristate */
			cfg = mmio_read_32(TEGRA_CL_DVFS_BASE + DVFS_DFLL_OUTPUT_CFG);
			if (cfg & DFLL_OUTPUT_CFG_CLK_EN_BIT) {
				val = mmio_read_32(TEGRA_MISC_BASE + PINMUX_AUX_DVFS_PWM);
				val &= ~PINMUX_PWM_TRISTATE;
				mmio_write_32(TEGRA_MISC_BASE + PINMUX_AUX_DVFS_PWM, val);

				/* make sure the setting took effect */
				val = mmio_read_32(TEGRA_MISC_BASE + PINMUX_AUX_DVFS_PWM);
				assert((val & PINMUX_PWM_TRISTATE) == 0U);
			}

			/*
			 * Restore operation mode for the DFLL ring
			 * oscillator
			 */
			mmio_write_32(TEGRA_CL_DVFS_BASE + DVFS_DFLL_CTRL,
				      ENABLE_CLOSED_LOOP);

			/* release cluster idle lock */
			tegra_fc_ccplex_pgexit_unlock();
		}
	}

	/*
	 * T210 has a dedicated ARMv7 boot and power mgmt processor, BPMP. It's
	 * used for power management and boot purposes. Inform the BPMP that
	 * we have completed the cluster power up.
	 */
	tegra_fc_lock_active_cluster();

	/*
	 * Resume PMC hardware block for Tegra210 platforms
	 */
	if (!tegra_chipid_is_t210_b01()) {
		tegra_pmc_resume();
	}

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

	/*
	 * Program the PMC in order to restart the system.
	 */
	tegra_pmc_system_reset();

	return PSCI_E_SUCCESS;
}

__dead2 void tegra_soc_prepare_system_off(void)
{
	ERROR("Tegra System Off: operation not handled.\n");
	panic();
}
