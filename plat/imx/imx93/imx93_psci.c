/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include "../drivers/arm/gic/v3/gicv3_private.h"

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <dram.h>
#include <ele_api.h>
#include <plat_imx8.h>
#include <pwr_ctrl.h>

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

/* platform secure warm boot entry */
static uintptr_t secure_entrypoint;

static bool boot_stage = true;

/*
 * GPC IRQ mask used to check if any the below interrupt is enabled
 * as wakeup source;
 *
 * lpuart3-6: 68-70,flexcan2: 51, usdhc1: 86, usdhc2: 87; fec: 181,
 * eqos: 183, usdhc3: 205, lpuart7: 210, lpaurt8: 211.
 */
uint32_t wakeupmix_irq_mask[IMR_NUM] = {
	0x0, 0x80000, 0xC000F0, 0x0,
	0x0, 0xA00000, 0xC2000, 0x0,
};

struct gpio_ctx wakeupmix_gpio_ctx[3] = {
	GPIO_CTX(GPIO2_BASE | BIT(28), 30),
	GPIO_CTX(GPIO3_BASE | BIT(28), 32),
	GPIO_CTX(GPIO4_BASE | BIT(28), 30),
};

uint32_t clock_root[6];

/*
 * Empty implementation of these hooks avoid setting the GICR_WAKER.Sleep bit
 * on ARM GICv3 implementations without LPI support.
 */
void arm_gicv3_distif_pre_save(unsigned int rdist_proc_num)
{
}

void arm_gicv3_distif_post_restore(unsigned int rdist_proc_num)
{
}

static struct qchannel_hsk_config {
	const uint32_t lpcg_idx;
	const uint32_t root_idx;
	const unsigned int wakeup_irq;
	uint32_t root_ctrl;
	uint32_t lpcg_cur;
	uint32_t lpcg_auth;
	bool active_wakeup;
} hsk_config[] = {
	{ CAN1_LPCG, CAN1_ROOT, 8 },
	{ CAN2_LPCG, CAN2_ROOT, 51 },

	{ UART1_LPCG, UART1_ROOT, 19 },
	{ UART2_LPCG, UART2_ROOT, 20 },
	{ UART3_LPCG, UART3_ROOT, 68 },
	{ UART4_LPCG, UART4_ROOT, 69 },
	{ UART5_LPCG, UART5_ROOT, 70 },
	{ UART6_LPCG, UART6_ROOT, 71 },
	{ UART7_LPCG, UART7_ROOT, 210 },
	{ UART8_LPCG, UART8_ROOT, 211 },

	/* wakeup_irq == 0: no IRQ check, always enable qchannel handshake (GPIO) */
	{ GPIO1_LPCG, },
	{ GPIO2_LPCG, },
	{ GPIO3_LPCG, },
	{ GPIO4_LPCG, },
};

/*
 * For peripherals like CANs, GPIOs & UARTs that need to support async wakeup
 * when clock is gated, LPCGs of these IPs need to be changed to CPU LPM
 * controlled, and for CANs &UARTs, we also need to make sure its ROOT clock
 * slice is enabled.
 */
void peripheral_qchannel_hsk(bool en)
{
	unsigned int i;
	struct qchannel_hsk_config *cfg;
	uintptr_t lpcg_base;

	for (i = 0; i < ARRAY_SIZE(hsk_config); i++) {
		cfg = &hsk_config[i];

		if (en) {
			if (cfg->wakeup_irq != 0U &&
			    !is_wakeup_source(cfg->wakeup_irq)) {
				cfg->active_wakeup = false;
				continue;
			}

			cfg->active_wakeup = true;
			if (cfg->root_idx != 0U) {
				cfg->root_ctrl =
					mmio_read_32(CCM_ROOT_SLICE(cfg->root_idx));
				mmio_clrbits_32(CCM_ROOT_SLICE(cfg->root_idx),
						ROOT_CLK_OFF);
			}

			lpcg_base = LPCG(cfg->lpcg_idx);
			cfg->lpcg_auth = mmio_read_32(lpcg_base + LPCG_AUTH);
			cfg->lpcg_cur = mmio_read_32(lpcg_base + LPCG_CUR);
			mmio_setbits_32(lpcg_base + LPCG_AUTH, CPU_LPM_EN);
			mmio_write_32(lpcg_base + LPCG_CUR, 0x2);
		} else if (cfg->active_wakeup) {
			lpcg_base = LPCG(cfg->lpcg_idx);
			mmio_write_32(lpcg_base + LPCG_AUTH, cfg->lpcg_auth);
			mmio_write_32(lpcg_base + LPCG_CUR, cfg->lpcg_cur);
			if (cfg->root_idx != 0U) {
				mmio_write_32(CCM_ROOT_SLICE(cfg->root_idx),
					      cfg->root_ctrl);
			}
		}
	}
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET) {
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int pwr_type = psci_get_pstate_type(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (pwr_type == PSTATE_TYPE_STANDBY) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	if (pwr_type == PSTATE_TYPE_POWERDOWN && state_id == 0x33) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	return PSCI_E_SUCCESS;
}

void imx_set_cpu_boot_entry(unsigned int core_id, uint64_t boot_entry)
{
	/* set the cpu core reset entry: BLK_CTRL_S */
	mmio_write_32(BLK_CTRL_S_BASE + CA55_RVBADDR0_L + core_id * 8, boot_entry >> 2);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core_id;

	core_id = MPIDR_AFFLVL1_VAL(mpidr);

	imx_set_cpu_boot_entry(core_id, secure_entrypoint);

	/*
	 * When the core is first time boot up, the core is already ON after SoC POR,
	 * So 'SW_WAKEUP' can not work, so need to toggle core's reset then release
	 * the core from cpu_wait.
	 */
	if (boot_stage) {
		/* assert CPU core SW reset */
		mmio_clrbits_32(SRC_SLICE(SRC_A55C0 + core_id) + 0x24, BIT(2) | BIT(0));
		/* deassert CPU core SW reset */
		mmio_setbits_32(SRC_SLICE(SRC_A55C0 + core_id) + 0x24, BIT(2) | BIT(0));
		/* release the cpuwait to kick the cpu */
		mmio_clrbits_32(BLK_CTRL_S_BASE + CA55_CPUWAIT, BIT(core_id));
	} else {
		/* assert the CMC MISC SW WAKEUP BIT to kick the offline core */
		gpc_assert_sw_wakeup(CPU_A55C0 + core_id);
	}

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* below config is ok both for boot & hotplug */
	/* clear the CPU power mode */
	gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_RUN);
	/* clear the SW wakeup */
	gpc_deassert_sw_wakeup(CPU_A55C0 + core_id);
	/* switch to GIC wakeup source */
	gpc_select_wakeup_gic(CPU_A55C0 + core_id);

	if (boot_stage) {
		/* SRC config */
		/* config the MEM LPM */
		src_mem_lpm_en(SRC_A55P0_MEM + core_id, MEM_OFF);
		/* LPM config to only ON in run mode to its domain */
		src_mix_set_lpm(SRC_A55C0 + core_id, core_id, CM_MODE_WAIT);
		/* Set CNT_MODE=0 to reduce unnecessary latency */
		src_ack_cnt_mode(SRC_A55C0 + core_id, 0x0);
		/* white list config, only enable its own domain */
		src_authen_config(SRC_A55C0 + core_id, 1 << core_id, 0x1);

		boot_stage = false;
	}
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int i;

	write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);

	/*
	 * mask all the GPC IRQ wakeup to make sure no IRQ can wakeup this core
	 * as we need to use SW_WAKEUP for hotplug purpose
	 */
	for (i = 0U; i < IMR_NUM; i++) {
		gpc_set_irq_mask(CPU_A55C0 + core_id, i, 0xffffffff);
	}
	/* switch to GPC wakeup source */
	gpc_select_wakeup_raw_irq(CPU_A55C0 + core_id);
	/* config the target mode to suspend */
	gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_SUSPEND);
}

void imx_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* do cpu level config */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		imx_set_cpu_boot_entry(core_id, secure_entrypoint);
		/* config the target mode to WAIT */
		gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_WAIT);
	}

	/* do cluster level config */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* config the A55 cluster target mode to WAIT */
		gpc_set_cpu_mode(CPU_A55_PLAT, CM_MODE_WAIT);

		/* config DSU for cluster power down with L3 MEM RET */
		if (is_local_state_retn(CLUSTER_PWR_STATE(target_state))) {
			write_clusterpwrdn(DSU_CLUSTER_PWR_OFF | BIT(1));
		}
	}

	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		/*
		 * for the A55 cluster, the cache disable/flushing is controlled by HW,
		 * so flush cache explicitly before put DDR into retention to make sure
		 * no cache maintenance to DDR memory happens after DDR retention.
		 */
		dcsw_op_all(DCCISW);
		dram_enter_retention();

		/*
		 * if NICMIX or WAKEUPMIX power down, the TRDC_N/W config will lost,
		 * so need to request Sentinel to set the correct permission at the early
		 * beginning.
		 */
		ele_request_pwrdown();

		nicmix_pwr_down(core_id);
		wakeupmix_pwr_down();

		/* Config the A55 cluster target mode to SUSPEND */
		gpc_set_cpu_mode(CPU_A55_PLAT, CM_MODE_SUSPEND);

		/* Enable system suspend when A55 cluster is in SUSPEND MODE */
		gpc_set_cpu_ss_mode(CPU_A55_PLAT, SS_SUSPEND);

		/* Force M33 into system sleep if M33 is not enabled. */
		if (is_m33_disabled()) {
			gpc_force_cpu_suspend(CPU_M33);
		}
		/* Put OSC into power down */
		gpc_rosc_off(true);
		/* Put PMIC into standby mode */
		gpc_pmic_stby_en(true);

		peripheral_qchannel_hsk(true);

		/* power down PLL */
		pll_pwr_down(true);
	}
}

void imx_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* System level */
	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		/* Disable system suspend when A55 cluster is in SUSPEND MODE */
		gpc_set_cpu_ss_mode(CPU_A55_PLAT, 0x0);

		if (is_m33_disabled()) {
			gpc_unforce_cpu_suspend(CPU_M33);
		}

		/* Disable PMIC standby */
		gpc_pmic_stby_en(false);
		/* Disable OSC power down */
		gpc_rosc_off(false);
		/* power down PLL */
		pll_pwr_down(false);
		peripheral_qchannel_hsk(false);

		nicmix_pwr_up(core_id);
		wakeupmix_pwr_up();
		dram_exit_retention();
	}

	/* cluster level */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* set the cluster's target mode to RUN */
		gpc_set_cpu_mode(CPU_A55_PLAT, CM_MODE_RUN);
	}

	/* do core level */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* set A55 CORE's power mode to RUN */
		gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_RUN);
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}

	SYSTEM_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
}

void __dead2 imx_system_reset(void)
{
	mmio_write_32(WDOG3_BASE + WDOG_CNT, 0xd928c520);
	while ((mmio_read_32(WDOG3_BASE + WDOG_CS) & WDOG_CS_ULK) == 0U) {
		;
	}

	mmio_write_32(WDOG3_BASE + WDOG_TOVAL, 0x10);
	mmio_write_32(WDOG3_BASE + WDOG_CS, 0x21e3);

	while (1) {
		wfi();
	}
}

void __dead2 imx_system_off(void)
{
	mmio_setbits_32(BBNSM_BASE + BBNSM_CTRL, BBNSM_DP_EN | BBNSM_TOSP);

	while (1) {
		wfi();
	}
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_suspend = imx_pwr_domain_suspend,
	.pwr_domain_suspend_finish = imx_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	/* sec_entrypoint is used for warm reset */
	secure_entrypoint = sec_entrypoint;
	imx_set_cpu_boot_entry(0, sec_entrypoint);

	pwr_sys_init();
	nicmix_qos_init();
	wakeupmix_qos_init();

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
