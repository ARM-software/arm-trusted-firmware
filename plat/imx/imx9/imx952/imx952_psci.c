/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/psci/psci.h>
#include <scmi_imx9.h>

#include <imx9_psci_common.h>
#include <imx9_sys_sleep.h>
#include <imx_scmi_client.h>
#include <plat_imx8.h>

uint32_t mask_all[IMR_NUM] = {
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff
};

/* IRQ masks used to check if any wakeup source is enabled:
 * lpuart3-8: 64-69, flexcan2,3: 38, 40
 * usdhc1,2,3: 86, 87, 191
 * netc: 304
 */
uint32_t wakeup_irq_mask[IMR_NUM] = {
	0x0, 0x0140, 0xc0003f, 0x0,
	0x0, 0x80000000, 0x0, 0x0,
	0x0, 0x10000, 0x0, 0x0, 0x0
};

struct per_hsk_cfg per_hsk_cfg[] = {
	{ CPU_PER_LPI_IDX_CAN1, 8 },
	{ CPU_PER_LPI_IDX_CAN2, 38 },
	{ CPU_PER_LPI_IDX_CAN3, 40 },

	{ CPU_PER_LPI_IDX_LPUART1, 19 },
	{ CPU_PER_LPI_IDX_LPUART3, 64 },
	{ CPU_PER_LPI_IDX_LPUART4, 65 },
	{ CPU_PER_LPI_IDX_LPUART5, 66 },
	{ CPU_PER_LPI_IDX_LPUART6, 67 },
	{ CPU_PER_LPI_IDX_LPUART7, 68 },
	{ CPU_PER_LPI_IDX_LPUART8, 69 },

	{ CPU_PER_LPI_IDX_GPIO2, 49 },
	{ CPU_PER_LPI_IDX_GPIO3, 51 },
	{ CPU_PER_LPI_IDX_GPIO4, 53 },
	{ CPU_PER_LPI_IDX_GPIO5, 55 },
};

struct gpio_ctx gpios[GPIO_NUM] = {
	GPIO_CTX(GPIO2_BASE, 32U),
	GPIO_CTX(GPIO3_BASE, 32U),
	GPIO_CTX(GPIO4_BASE, 30U),
	GPIO_CTX(GPIO5_BASE, 18U),
};

struct wdog_ctx wdogs[WDOG_NUM] = {
	{ WDOG3_BASE },
	{ WDOG4_BASE },
};

static const plat_psci_ops_t imx_plat_psci_ops = {
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_suspend = imx_pwr_domain_suspend,
	.pwr_domain_suspend_finish = imx_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.pwr_domain_pwr_down = imx_pwr_domain_pwr_down,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	uint32_t mask = DEBUG_WAKEUP_MASK | EVENT_WAKEUP_MASK;

	/* sec_entrypoint is used for warm reset */
	secure_entrypoint = sec_entrypoint;
	imx_set_cpu_boot_entry(0, secure_entrypoint, SCMI_CPU_VEC_FLAGS_BOOT);

	/*
	 * Set NON-IRQ wakeup mask for both last core and cluster.
	 * Disable wakeup on DEBUG_WAKEUP
	 */
	scmi_core_nonIrq_wake_set(imx9_scmi_handle, IMX9_SCMI_CPU_A55C0, 0U, 1U, mask);
	scmi_core_nonIrq_wake_set(imx9_scmi_handle, IMX9_SCMI_CPU_A55P, 0U, 1U, mask);

	/*
	 * Setup A55 Cluster state for cpuidle.
	 */
	struct scmi_lpm_config cpu_lpm_cfg[] = {
		{
			SCMI_PWR_MIX_SLICE_IDX_A55P,
			SCMI_CPU_PD_LPM_ON_RUN,
			BIT_32(SCMI_PWR_MEM_SLICE_IDX_A55L3)
		},
		{
			SCMI_PWR_MIX_SLICE_IDX_NOC,
			SCMI_CPU_PD_LPM_ON_ALWAYS,
			0U
		},
		{
			SCMI_PWR_MIX_SLICE_IDX_WAKEUP,
			SCMI_CPU_PD_LPM_ON_ALWAYS,
			0
		}
	};
	/* Set the default LPM state for suspend/hotplug */
	scmi_core_lpm_mode_set(imx9_scmi_handle,
			       IMX9_SCMI_CPU_A55P,
			       ARRAY_SIZE(cpu_lpm_cfg),
			       cpu_lpm_cfg);

	/* Set the LPM state for cpuidle for A55C0 (boot core) */
	cpu_lpm_cfg[0].power_domain = SCMI_PWR_MIX_SLICE_IDX_A55C0;
	cpu_lpm_cfg[0].lpmsetting = SCMI_CPU_PD_LPM_ON_RUN;
	cpu_lpm_cfg[0].retentionmask = 0U;
	scmi_core_lpm_mode_set(imx9_scmi_handle, IMX9_SCMI_CPU_A55C0,
			       1U, cpu_lpm_cfg);

	/*
	 * Set core & cluster to GIC wakeup source since NOCMIX is not
	 * powered down, config the target mode to WAIT
	 */
	scmi_core_set_sleep_mode(imx9_scmi_handle, IMX9_SCMI_CPU_A55C0,
				 SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_WAIT);

	scmi_core_set_sleep_mode(imx9_scmi_handle, IMX9_SCMI_CPU_A55P,
				 SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_WAIT);

	/* Enable the wdog3 per handshake */
	struct scmi_per_lpm_config per_lpm[1] = {
		{ CPU_PER_LPI_IDX_WDOG3, SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP },
	};

	scmi_per_lpm_mode_set(imx9_scmi_handle, IMX9_SCMI_CPU_A55P,
			      1U, per_lpm);

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
