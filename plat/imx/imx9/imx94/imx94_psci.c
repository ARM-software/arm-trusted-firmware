/*
 * Copyright 2024-2025 NXP
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
	0xffffffff, 0xffffffff, 0xffffffff
};

/*
 * IRQ masks used to check if any of the below IRQ is
 * enabled as the wakeup source:
 * lpuart1: 21, flexcan2-5: 40, 42, 44, 46, usdhc1-3: 96, 97, 116
 */
uint32_t wakeup_irq_mask[IMR_NUM] = {
	0x600000, 0x5500, 0xFFC0, 0x100003, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x80
};

struct per_hsk_cfg per_hsk_cfg[] = {
	{ CPU_PER_LPI_IDX_CAN2, 40U },
	{ CPU_PER_LPI_IDX_CAN3, 42U },
	{ CPU_PER_LPI_IDX_CAN4, 44U },
	{ CPU_PER_LPI_IDX_CAN5, 46U },

	{ CPU_PER_LPI_IDX_LPUART1, 21U },
	{ CPU_PER_LPI_IDX_LPUART2, 22U },
	{ CPU_PER_LPI_IDX_LPUART3, 74U },
	{ CPU_PER_LPI_IDX_LPUART4, 75U },
	{ CPU_PER_LPI_IDX_LPUART5, 76U },
	{ CPU_PER_LPI_IDX_LPUART6, 77U },
	{ CPU_PER_LPI_IDX_LPUART7, 78U },
	{ CPU_PER_LPI_IDX_LPUART8, 79U },
	{ CPU_PER_LPI_IDX_LPUART9, 80U },
	{ CPU_PER_LPI_IDX_LPUART10, 81U },
	{ CPU_PER_LPI_IDX_LPUART11, 82U },
	{ CPU_PER_LPI_IDX_LPUART12, 83U },

	{ CPU_PER_LPI_IDX_GPIO2, 54U },
	{ CPU_PER_LPI_IDX_GPIO3, 56U },
	{ CPU_PER_LPI_IDX_GPIO4, 58U },
	{ CPU_PER_LPI_IDX_GPIO5, 60U },
	{ CPU_PER_LPI_IDX_GPIO6, 62U },
	{ CPU_PER_LPI_IDX_GPIO7, 64U },
};

struct gpio_ctx gpios[GPIO_NUM] = {
	GPIO_CTX(GPIO2_BASE, 32U),
	GPIO_CTX(GPIO3_BASE, 26U),
	GPIO_CTX(GPIO4_BASE, 32U),
	GPIO_CTX(GPIO5_BASE, 32U),
	GPIO_CTX(GPIO6_BASE, 32U),
	GPIO_CTX(GPIO7_BASE, 22U),
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

/* Export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	uint32_t mask = DEBUG_WAKEUP_MASK | EVENT_WAKEUP_MASK;

	/* sec_entrypoint is used for warm reset */
	secure_entrypoint = sec_entrypoint;
	imx_set_cpu_boot_entry(0U, secure_entrypoint, SCMI_CPU_VEC_FLAGS_BOOT);

	/*
	 * Set NON-IRQ wakeup mask for both last core and cluster.
	 * Disable wakeup on DEBUG_WAKEUP
	 */
	scmi_core_nonIrq_wake_set(imx9_scmi_handle, IMX9_SCMI_CPU_A55C0, 0U, 1U, mask);
	scmi_core_nonIrq_wake_set(imx9_scmi_handle, IMX9_SCMI_CPU_A55P, 0U, 1U, mask);

	/*
	 * Setup A55 Cluster state for Cpuidle.
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
			0U
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
	 * Set core/custer to GIC wakeup source since NOCMIX is not
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
