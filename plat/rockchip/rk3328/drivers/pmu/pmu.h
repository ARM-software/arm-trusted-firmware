/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMU_H
#define PMU_H

#include <soc.h>

struct rk3328_sleep_ddr_data {
	uint32_t pmu_debug_enable;
	uint32_t debug_iomux_save;
	uint32_t pmic_sleep_save;
	uint32_t pmu_wakeup_conf0;
	uint32_t pmu_pwrmd_com;
	uint32_t cru_mode_save;
	uint32_t clk_sel0, clk_sel1, clk_sel18,
		 clk_sel20, clk_sel24, clk_sel38;
	uint32_t clk_ungt_save[CRU_CLKGATE_NUMS];
	uint32_t cru_plls_con_save[MAX_PLL][CRU_PLL_CON_NUMS];
};

struct rk3328_sleep_sram_data {
	uint32_t pmic_sleep_save;
	uint32_t pmic_sleep_gpio_save[2];
	uint32_t ddr_grf_con0;
	uint32_t dpll_con_save[CRU_PLL_CON_NUMS];
	uint32_t pd_sr_idle_save;
	uint32_t uart2_ier;
};

/*****************************************************************************
 * The ways of cores power domain contorlling
 *****************************************************************************/
enum cores_pm_ctr_mode {
	core_pwr_pd = 0,
	core_pwr_wfi = 1,
	core_pwr_wfi_int = 2
};

enum pmu_cores_pm_by_wfi {
	core_pm_en = 0,
	core_pm_int_wakeup_en,
	core_pm_dis_int,
	core_pm_sft_wakeup_en
};

extern void *pmu_cpuson_entrypoint_start;
extern void *pmu_cpuson_entrypoint_end;

#define CORES_PM_DISABLE	0x0

/*****************************************************************************
 * pmu con,reg
 *****************************************************************************/
#define PMU_WAKEUP_CFG0		0x00
#define PMU_PWRDN_CON		0x0c
#define PMU_PWRDN_ST		0x10
#define PMU_PWRMD_COM		0x18
#define PMU_SFT_CON		0x1c
#define PMU_INT_CON		0x20
#define PMU_INT_ST		0x24
#define PMU_POWER_ST		0x44
#define PMU_CPUAPM_CON(n)	(0x80 + (n) * 4)
#define PMU_SYS_REG(n)		(0xa0 + (n) * 4)

#define CHECK_CPU_WFIE_BASE		(GRF_BASE + GRF_CPU_STATUS(1))

enum pmu_core_pwrst_shift {
	clst_cpu_wfe = 0,
	clst_cpu_wfi = 4,
};

#define clstl_cpu_wfe (clst_cpu_wfe)
#define clstb_cpu_wfe (clst_cpu_wfe)

enum pmu_pd_id {
	PD_CPU0 = 0,
	PD_CPU1,
	PD_CPU2,
	PD_CPU3,
};

enum pmu_power_mode_common {
	pmu_mode_en = 0,
	sref_enter_en,
	global_int_disable_cfg,
	cpu0_pd_en,
	wait_wakeup_begin_cfg = 4,
	l2_flush_en,
	l2_idle_en,
	ddrio_ret_de_req,
	ddrio_ret_en = 8,
};

enum pmu_sft_con {
	upctl_c_sysreq_cfg = 0,
	l2flushreq_req,
	ddr_io_ret_cfg,
	pmu_sft_ret_cfg,
};

#define CKECK_WFE_MSK		0x1
#define CKECK_WFI_MSK		0x10
#define CKECK_WFEI_MSK		0x11

#define PD_CTR_LOOP		500
#define CHK_CPU_LOOP		500
#define MAX_WAIT_CONUT		1000

#define WAKEUP_INT_CLUSTER_EN	0x1
#define PMIC_SLEEP_REG		0x34

#define PLL_IS_NORM_MODE(mode, pll_id)	\
		((mode & (PLL_NORM_MODE(pll_id)) & 0xffff) != 0)

#define CTLR_ENABLE_G1_BIT	BIT(1)
#define UART_FIFO_EMPTY		BIT(6)

#define UART_IER		0x04
#define UART_FCR		0x08
#define UART_LSR		0x14

#define UART_INT_DISABLE	0x00
#define UART_FIFO_RESET		0x07

#endif /* PMU_H */
