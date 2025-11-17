/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#ifndef __PMU_H__
#define __PMU_H__

#include <assert.h>

#include <lib/mmio.h>
#include <platform_def.h>

/* pmu */
#define PMU1_OFFSET			0x4000
#define PMU2_OFFSET			0x8000

#define PMU0_PWR_CON			0x0000
#define PMU0_PMIC_STABLE_CNT		0x0010
#define PMU0_INFO_TX_CON		0x0030
#define PMU0_INFO_SFT_CON		0x0034
#define PMU0_PWR_ACK_SEL_CON		0x0040
#define PMU0_BUS_IDLE_ACK		0x0044
#define PMU0_BUS_IDLE_ST		0x0048

#define PMU1_VERSION_ID			(PMU1_OFFSET + 0x000)
#define PMU1_PWR_CON			(PMU1_OFFSET + 0x004)
#define PMU1_PWR_FSM			(PMU1_OFFSET + 0x008)
#define PMU1_INT_MASK_CON		(PMU1_OFFSET + 0x00c)
#define PMU1_WAKEUP_INT_CON		(PMU1_OFFSET + 0x010)
#define PMU1_WAKEUP_INT_ST		(PMU1_OFFSET + 0x014)
#define PMU1_WAKEUP_EDGE_CON		(PMU1_OFFSET + 0x018)
#define PMU1_WAKEUP_EDGE_ST		(PMU1_OFFSET + 0x01c)
#define PMU1_WAKEUP_TIMEOUT_CNT_ST	(PMU1_OFFSET + 0x020)
#define PMU1_DDR_CH0PWR_CON		(PMU1_OFFSET + 0x100)
#define PMU1_DDR_CH0PWR_SFTCON		(PMU1_OFFSET + 0x110)
#define PMU1_DDR_AXIPWR_CON		(PMU1_OFFSET + 0x120)
#define PMU1_DDR_AXIPWR_SFTCON(i)	(PMU1_OFFSET + 0x130 + (i) * 4)
#define PMU1_DDR_PWR_FSM		(PMU1_OFFSET + 0x140)
#define PMU1_DDR_ST			(PMU1_OFFSET + 0x144)
#define PMU1_DDR_AXI_ST			(PMU1_OFFSET + 0x148)
#define PMU1_CRU_PWR_CON(i)		(PMU1_OFFSET + 0x200 + (i) * 4)
#define PMU1_CRU_PWR_SFTCON(i)		(PMU1_OFFSET + 0x208 + (i) * 4)
#define PMU1_CRU_PWR_FSM		(PMU1_OFFSET + 0x210)
#define PMU1_PLLPD_CON			(PMU1_OFFSET + 0x220)
#define PMU1_PLLPD_SFTCON		(PMU1_OFFSET + 0x228)
#define PMU1_PMIC_STABLE_CNT		(PMU1_OFFSET + 0x300)
#define PMU1_OSC_STABLE_CNT		(PMU1_OFFSET + 0x304)
#define PMU1_WAKEUP_RST_CLR_CNT		(PMU1_OFFSET + 0x308)
#define PMU1_PLL_LOCK_CNT		(PMU1_OFFSET + 0x30c)
#define PMU1_WAKEUP_TIMEOUT		(PMU1_OFFSET + 0x310)
#define PMU1_PWM_SWITCH_CNT		(PMU1_OFFSET + 0x314)
#define PMU1_SLEEP_CNT			(PMU1_OFFSET + 0x318)

#define PMU2_SCU_PWR_CON		(PMU2_OFFSET + 0x000)
#define PMU2_SCU_PWR_SFTCON		(PMU2_OFFSET + 0x004)
#define PMU2_SCU_AUTO_PWR_CON		(PMU2_OFFSET + 0x008)
#define PMU2_SCU_PWR_FSM_STATUS		(PMU2_OFFSET + 0x00c)
#define PMU2_DBG_PWR_CON		(PMU2_OFFSET + 0x018)
#define PMU2_CLUSTER_PWR_ST		(PMU2_OFFSET + 0x01c)
#define PMU2_CLUSTER_IDLE_CON		(PMU2_OFFSET + 0x020)
#define PMU2_CLUSTER_IDLE_SFTCON	(PMU2_OFFSET + 0x024)
#define PMU2_SCU_PWRUP_CNT		(PMU2_OFFSET + 0x040)
#define PMU2_SCU_PWRDN_CNT		(PMU2_OFFSET + 0x044)
#define PMU2_SCU_STABLE_CNT		(PMU2_OFFSET + 0x048)
#define PMU2_CPU_AUTO_PWR_CON(i)	(PMU2_OFFSET + 0x080 + (i) * 4)
#define PMU2_CPU_PWR_SFTCON(i)		(PMU2_OFFSET + 0x0a0 + (i) * 4)
#define PMU2_BUS_IDLE_CON		(PMU2_OFFSET + 0x100)
#define PMU2_BUS_IDLE_SFTCON		(PMU2_OFFSET + 0x110)
#define PMU2_BUS_IDLE_ACK		(PMU2_OFFSET + 0x120)
#define PMU2_BUS_IDLE_ST		(PMU2_OFFSET + 0x128)
#define PMU2_NOC_AUTO_CON		(PMU2_OFFSET + 0x130)
#define PMU2_NOC_AUTO_SFTCON		(PMU2_OFFSET + 0x140)
#define PMU2_PWR_GATE_CON		(PMU2_OFFSET + 0x200)
#define PMU2_PWR_GATE_SFTCON		(PMU2_OFFSET + 0x210)
#define PMU2_VOL_GATE_CON		(PMU2_OFFSET + 0x220)
#define PMU2_PWR_GATE_ST		(PMU2_OFFSET + 0x230)
#define PMU2_PWR_GATE_FSM		(PMU2_OFFSET + 0x238)
#define PMU2_PWRDN_ACK_ST		(PMU2_OFFSET + 0x240)
#define PMU2_PWR_C0_ACK_SEL_CON		(PMU2_OFFSET + 0x280)

/* PMU0CRU */
#define PMU0CRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define PMU0CRU_CLKSEL_CON_CNT		9
#define PMU0CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define PMU0CRU_CLKGATE_CON_CNT		4
#define PMU0CRU_SOFTRST_CON(i)		(0xa00 + (i) * 0x4)
#define PMU0CRU_SOFTRST_CON_CNT		4
#define PMU0CRU_AUTOCS_BUSCLK_MUX_CON(i)	(0xd00 + (i) * 0x4)

/* PMU1CRU */
#define PMU1CRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define PMU1CRU_CLKSEL_CON_CNT		2
#define PMU1CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define PMU1CRU_CLKGATE_CON_CNT		2
#define PMU1CRU_SOFTRST_CON(i)		(0xa00 + (i) * 0x4)
#define PMU1CRU_SOFTRST_CON_CNT		2

/* PMUSCRU */
#define PMUSCRU_CLKGATE_CON0		0x800
#define PMUSCRU_CLKGATE_CON_CNT		1
#define PMUSCRU_SOFTRST_CON0		0xa00

/* PMUGRF */
#define PMUGRF_SOC_CON(i)		((i) * 4)
#define PMUGRF_AAD_CON0		0x80
#define PMUGRF_MEM_CON(i)		(0x140 + (i) * 4)
#define PMUGRF_SOC_STATUS(i)		(0x170 + (i) * 4)
#define PMUGRF_OS_REG(i)		(0x200 + (i) * 4)

#define PMU_MCU_RST			BIT(8)
#define PMU_MCU_HALT			BIT(9)

#define PMU_MCU_STOP_MSK		\
	(PMU_MCU_HALT | PMU_MCU_RST)

#define CORES_PM_DISABLE		0x0

/* pmuioc */
#define PMUIO0_IOC_GPIO0A_IOMUX_SEL_L	0x000
#define PMUIO0_IOC_GPIO0A_IOMUX_SEL_H	0x004
#define PMUIO0_IOC_GPIO0B_IOMUX_SEL_L	0x008
#define PMUIO0_IOC_GPIO0B_IOMUX_SEL_H	0x00c

#define PMUIO1_IOC_GPIO0C_IOMUX_SEL_L	0x010
#define PMUIO1_IOC_GPIO0C_IOMUX_SEL_H	0x014
#define PMUIO1_IOC_GPIO0D_IOMUX_SEL_L	0x018

/* PMU_PWR_CON */
enum pmu0_pwr_con {
	pmu_powermode0_en = 0,
	pmu_pmu1_pd_byp = 1,
	pmu_pmu1_bus_byp = 2,
	pmu_pmu0_pmic_stable_byp = 4,
	pmu_pmu1_pwrgt = 8,
	pmu_pmu1_pwrgt_sft = 9,
	pmu_pmu1_mempwr_sft_gt = 10,
	pmu_pmu1_idle_en = 11,
	pmu_pmu1_idle_sft_en = 12,
	pmu_pmu1_noc_auto_en = 13
};

enum pmu1_pwr_con {
	pmu_powermode_en = 0,
	pmu_scu0_byp = 1,
	pmu_scu1_byp = 2,
	pmu_bus_byp = 4,
	pmu_ddr_byp = 5,
	pmu_pwrgt_byp = 6,
	pmu_cru_byp = 7,
	pmu_pdpmu1_byp = 9,
	pmu_wfi_byp = 12,
	pmu_slp_cnt_en = 13,
	pmu_wkup_pmu_sft_en = 14,
};

enum pmu_wakeup_int {
	pmu_wkup_cpu0_int = 0,
	pmu_wkup_cpu1_int = 1,
	pmu_wkup_cpu2_int = 2,
	pmu_wkup_cpu3_int = 3,
	pmu_wkup_gpio0_int = 4,
	pmu_wkup_sdmmc0_int = 5,
	pmu_wkup_sdmmc1_int = 6,
	pmu_wkup_sdio_int = 7,
	pmu_wkup_usbdev_int = 8,
	pmu_wkup_uart0_int = 9,
	pmu_wkup_i2c2_int = 10,
	pmu_wkup_pwm1_int = 11,
	pmu_wkup_timer_int = 12,
	pmu_wkup_hptimer_int = 13,
	pmu_wkup_sys_int = 14,
	pmu_wkup_aov_int = 15,
	pmu_wkup_aad_int = 16,
	pmu_wkup_timeout = 17,
};

/* PMU_DDR_PWR_CON */
enum pmu_ddr_ch_pwr_con {
	pmu_ddr_sref_c_en = 0,
	pmu_ddr_ioret_en = 1,
	pmu_ddr_ioret_exit_en = 2,
	pmu_ddr_rstiov_en = 3,
	pmu_ddr_rstiov_exit_en = 4,
	pmu_ddrctl_c_auto_gating_en = 5,
	pmu_ddrphy_auto_gating_en = 6,
};

enum pmu_ddr_axi_pwr_con {
	pmu_ddr_sref_a_ch0_en = 0,
	pmu_ddr_sref_a_ch1_en = 1,
	pmu_ddr_sref_a_ch2_en = 2,
	pmu_ddr_sref_a_ch3_en = 3,
};

/* PMU_CRU_PWR_CON0 */
enum pmu_cru_pwr_con0 {
	pmu_alive_32k_en = 0,
	pmu_osc_dis_en = 1,
	pmu_wakeup_rst_en = 2,
	pmu_input_clamp_en = 3,
	pmu_alive_osc_mode_en = 4,
	pmu_power_off_en = 5,
	pmu_pwm_switch_en = 6,
	pmu_pwm_gpio_ioe_en = 7,
	pmu_pwm_switch_io = 8,
	pmu_pwm_clkgt_en = 9,
};

/* PMU_CRU_PWR_CON1 */
enum pmu_cru_pwr_con1 {
	pmu_peri_clksrc_gt_en = 0,
	pmu_vepu_clksrc_gt_en = 1,
	pmu_vcp_clksrc_gt_en = 2,
	pmu_vi_clksrc_gt_en = 3,
	pmu_npu_clksrc_gt_en = 4,
	pmu_vdo_clksrc_gt_en = 5,
	pmu_aisp_clksrc_gt_en = 6,
	pmu_core_clksrc_gt_en = 7,
	pmu_ddr_clksrc_gt_en = 8,
	pmu_bus_clksrc_gt_en = 9,
};

/* PMU_SCU_PWR_CON */
enum pmu_scu_pwr_con {
	pmu_l2_flush_en = 0,
	pmu_l2_ilde_en = 1,
	pmu_scu_pd_en = 2,
	pmu_scu_pwroff_en = 3,
	pmu_clst_cpu_pd_en = 5,
	pmu_clst_clksrc_gt_en = 6,
	pmu_std_wfi_bypass = 8,
	pmu_std_wfil2_bypass = 9,
};

enum pmu_scu_pwr_sftcon {
	pmu_l2_flush_req_clst_cfg = 0,
	pmu_acinactm_clst_cfg = 1,
	pmu_scu_pd_sft_en = 2,
	pmu_scu_pwroff_sft_en = 3,
	pmu_scu_vol_gate = 5,
	pmu_scu_dwn_ack_sel = 6,
};

/* PMU_PLLPD_CON */
enum pmu_pllpd_con {
	pmu_gpll_pd_en = 0,
	pmu_dpll_pd_en = 1,
	pmu_cpll_pd_en = 2,
	pmu_ppll_pd_en = 3,
};

/* PMU_CLST_PWR_ST */
enum pmu_clst_pwr_st {
	pmu_cpu0_wfi = 0,
	pmu_cpu1_wfi = 1,
	pmu_cpu2_wfi = 2,
	pmu_cpu3_wfi = 3,
	pmu_standbywfil2 = 5,
	pmu_l2flushdone = 6,
	pmu_cpu0_pd_st = 8,
	pmu_cpu1_pd_st = 9,
	pmu_cpu2_pd_st = 10,
	pmu_cpu3_pd_st = 11,
	pmu_scu_pd_st = 12,
};

/* PMU_CLST_IDLE_CON */
enum pmu_clst_idle_con {
	pmu_idle_req_cpu = 2,
	pmu_core_clk_gt_msk = 3,
};

enum cores_pm_ctr_mode {
	core_pwr_pd = 0,
	core_pwr_wfi = 1,
	core_pwr_wfi_int = 2,
	core_pwr_wfi_reset = 3,
};

/* PMU_CPUX_AUTO_PWR_CON */
enum pmu_cpu_auto_pwr_con {
	pmu_cpu_pm_en = 0,
	pmu_cpu_pm_int_wakeup_en = 1,
	pmu_cpu_pm_dis_int = 2,
	pmu_cpu_pm_sft_wakeup_en = 3,
	pmu_cpu_auto_pwrdn_mode = 4,
	pmu_cpu_cluster_wakeup_en = 5,
};

enum qos_id {
	qos_cpu = 0,
	qos_npu = 1,
	qos_aiisp = 2,
	qos_dma2ddr = 3,
	qos_emmc = 4,
	qos_fspi = 5,
	qos_u2host = 6,
	qos_u3otg = 7,
	qos_rkvenc = 8,
	qos_saradc0 = 9,
	qos_sdmmc1 = 10,
	qos_lpmcu = 11,
	qos_dcf = 12,
	qos_mcu = 13,
	qos_rga = 14,
	qos_rkce = 15,
	qos_rkdma = 16,
	qos_decom = 17,
	qos_ooc = 18,
	qos_rkjpeg = 19,
	qos_rkvdec = 20,
	qos_vop = 21,
	qos_avsp_ro = 22,
	qos_avsp_wo = 23,
	qos_fec_ro = 24,
	qos_fec_wo = 25,
	qos_aad = 26,
	qos_afe = 27,
	qos_atdd = 28,
	qos_fspi1 = 29,
	qos_lpdma = 30,
	qos_spi2apb = 31,
	qos_gmac = 32,
	qos_isp = 33,
	qos_rkcan0 = 34,
	qos_rkcan1 = 35,
	qos_sdmmc0 = 36,
	qos_vicap = 37,
	qos_vpsi = 38,
	qos_vpss = 39,
	qos_saradc1 = 40,
	qos_saradc2 = 41,
};

enum pmu_bus_id {
	pmu_bus_id_ddr = 0,
	pmu_bus_id_subddr = 1,
	pmu_bus_id_cfgddr = 2,
	pmu_bus_id_peri = 3,
	pmu_bus_id_vepu = 4,
	pmu_bus_id_vcp = 5,
	pmu_bus_id_vi = 6,
	pmu_bus_id_cru = 7,
	pmu_bus_id_npu = 8,
	pmu_bus_id_vdo = 9,
	pmu_bus_id_aisp = 10,
	pmu_bus_id_bus = 11,
	pmu_bus_id_max,
};

enum pmu_pd_id {
	pmu_pd_npu = 0,
	pmu_pd_vdo = 1,
	pmu_pd_aiisp = 2,
	pmu_pd_id_max,
};

enum pmu_vd_id {
	pmu_vd_npu = 0,
	pmu_vd_id_max,
};

enum pmu_bus_state {
	pmu_bus_active = 0,
	pmu_bus_idle = 1,
};

enum pmu_pd_state {
	pmu_pd_on = 0,
	pmu_pd_off = 1
};

#define PD_CTR_LOOP			5000
#define WFEI_CHECK_LOOP			5000
#define BUS_IDLE_LOOP			1000
#define NONBOOT_CPUS_OFF_LOOP		500000

#define REBOOT_FLAG			0x5242C300
#define BOOT_BROM_DOWNLOAD		0xef08a53c

#define BOOTROM_SUSPEND_MAGIC		0x02468ace
#define BOOTROM_RESUME_MAGIC		0x13579bdf
#define WARM_BOOT_MAGIC			0x76543210
#define VALID_GLB_RST_MSK		0xffff

#define DEFAULT_BOOT_CPU		0

/*******************************************************
 *     sleep mode define
 *******************************************************/
#define SLP_ARMPD			BIT(0)
#define SLP_ARMOFF			BIT(1)
#define SLP_ARMOFF_DDRPD		BIT(2)
#define SLP_ARMOFF_LOGOFF		BIT(3)
#define SLP_ARMOFF_PMUOFF		BIT(4)
#define SLP_FROM_UBOOT			BIT(5)

/* all plls except ddr's pll*/
#define SLP_PMU_HW_PLLS_PD		BIT(8)
#define SLP_PMU_PMUALIVE_32K		BIT(9)
#define SLP_PMU_DIS_OSC			BIT(10)

#define SLP_CLK_GT			BIT(16)
#define SLP_PMIC_LP			BIT(17)

#define SLP_32K_IO			BIT(23)
#define SLP_32K_EXT			BIT(24)
#define SLP_TIME_OUT_WKUP		BIT(25)
#define SLP_PMU_DBG			BIT(26)
#define SLP_LP_PR			BIT(27)
#define SLP_ARCH_TIMER_RESET		BIT(28)
#define SLP_LP_AOA			BIT(29)

#define PM_INVALID_GPIO			0xffff
#define MAX_GPIO0_IO_CNT		26
#define MAX_SLEEP_IO_CFG_CNT		MAX_GPIO0_IO_CNT
#define MAX_VIRTUAL_PWROFF_IRQ_CNT	20

enum {
	RK_PM_VIRT_PWROFF_EN = 0,
	RK_PM_VIRT_PWROFF_IRQ_CFG = 1,
	RK_PM_VIRT_PWROFF_MAX,
};

/* sleep pin */
#define RKPM_SLEEP_PIN0_EN		BIT(0)	/* GPIO0_A3 */
#define RKPM_SLEEP_PIN1_EN		BIT(1)	/* GPIO0_A4 */
#define RKPM_SLEEP_PIN2_EN		BIT(2)	/* GPIO0_A5 */

#define RKPM_SLEEP_PIN0_ACT_LOW		BIT(0)	/* GPIO0_A3 */
#define RKPM_SLEEP_PIN1_ACT_LOW		BIT(1)	/* GPIO0_A4 */
#define RKPM_SLEEP_PIN2_ACT_LOW		BIT(2)	/* GPIO0_A5 */

/* io config */
#define RKPM_IO_CFG_IOMUX_SFT		0
#define RKPM_IO_CFG_GPIO_DIR_SFT	8
#define RKPM_IO_CFG_GPIO_LVL_SFT	9
#define RKPM_IO_CFG_PULL_SFT		10
#define RKPM_IO_CFG_ID_SFT		16

#define RKPM_IO_CFG_IOMUX_MSK		0x3f
#define RKPM_IO_CFG_GPIO_DIR_MSK	0x1
#define RKPM_IO_CFG_GPIO_LVL_MSK	0x1
#define RKPM_IO_CFG_PULL_MSK		0x3
#define RKPM_IO_CFG_ID_MSK		0xffff

#define RKPM_IO_CFG_IOMUX_GPIO_VAL	0
#define RKPM_IO_CFG_GPIO_DIR_INPUT_VAL	0
#define RKPM_IO_CFG_GPIO_DIR_OUTPUT_VAL 1
#define RKPM_IO_CFG_GPIO_LVL_LOW_VAL	0
#define RKPM_IO_CFG_GPIO_LVL_HIGH_VAL	1
#define RKPM_IO_CFG_PULL_NONE_VAL	0
#define RKPM_IO_CFG_PULL_UP_VAL		1
#define RKPM_IO_CFG_PULL_DOWN_VAL	2

#define RKPM_IO_CFG_IOMUX(func)		((func) << RKPM_IO_CFG_IOMUX_SFT)
#define RKPM_IO_CFG_GPIO_DIR_INPUT	\
		(RKPM_IO_CFG_GPIO_DIR_INPUT_VAL << RKPM_IO_CFG_GPIO_DIR_SFT)
#define RKPM_IO_CFG_GPIO_DIR_OUTPUT	\
		(RKPM_IO_CFG_GPIO_DIR_OUTPUT_VAL << RKPM_IO_CFG_GPIO_DIR_SFT)
#define RKPM_IO_CFG_GPIO_LVL_LOW	\
		(RKPM_IO_CFG_GPIO_LVL_LOW_VAL << RKPM_IO_CFG_GPIO_LVL_SFT)
#define RKPM_IO_CFG_GPIO_LVL_HIGH	\
		(RKPM_IO_CFG_GPIO_LVL_HIGH_VAL << RKPM_IO_CFG_GPIO_LVL_SFT)
#define RKPM_IO_CFG_PULL_NONE		\
		(RKPM_IO_CFG_PULL_NONE_VAL << RKPM_IO_CFG_PULL_SFT)
#define RKPM_IO_CFG_PULL_UP		\
		(RKPM_IO_CFG_PULL_UP_VAL << RKPM_IO_CFG_PULL_SFT)
#define RKPM_IO_CFG_PULL_DOWN		\
		(RKPM_IO_CFG_PULL_DOWN_VAL << RKPM_IO_CFG_PULL_SFT)
#define RKPM_IO_CFG_ID(id)		((id) << RKPM_IO_CFG_ID_SFT)
#define RKPM_IO_CFG_IOMUX_GPIO		\
		RKPM_IO_CFG_IOMUX(RKPM_IO_CFG_IOMUX_GPIO_VAL)

#define RKPM_IO_CFG_GET_IOMUX(cfg)	\
		(((cfg) >> RKPM_IO_CFG_IOMUX_SFT) & RKPM_IO_CFG_IOMUX_MSK)
#define RKPM_IO_CFG_GET_GPIO_DIR(cfg)	\
		(((cfg) >> RKPM_IO_CFG_GPIO_DIR_SFT) & RKPM_IO_CFG_GPIO_DIR_MSK)
#define RKPM_IO_CFG_GET_GPIO_LVL(cfg)	\
		(((cfg) >> RKPM_IO_CFG_GPIO_LVL_SFT) & RKPM_IO_CFG_GPIO_LVL_MSK)
#define RKPM_IO_CFG_GET_PULL(cfg)	\
		(((cfg) >> RKPM_IO_CFG_PULL_SFT) & RKPM_IO_CFG_PULL_MSK)
#define RKPM_IO_CFG_GET_ID(cfg)		\
		(((cfg) >> RKPM_IO_CFG_ID_SFT) & RKPM_IO_CFG_ID_MSK)

#define pmu_bus_idle_st(id)	\
	(!!(mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST) & BIT(id)))

#define pmu_bus_idle_ack(id)	\
	(!!(mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ACK) & BIT(id)))

#define pmu_bus_idle_st_msk(msk)	\
	(mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST) & (msk))

#define pmu_bus_idle_ack_msk(msk)	\
	(mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ACK) & (msk))

static inline uint32_t read_mem_os_reg(uint32_t id)
{
	assert((id) < MAX_MEM_OS_REG_NUM);

	return mmio_read_32(MEM_OS_REG_BASE + 4 * (id));
}

static inline void write_mem_os_reg(uint32_t id, uint32_t val)
{
	assert((id) < MAX_MEM_OS_REG_NUM);

	mmio_write_32(MEM_OS_REG_BASE + 4 * (id), val);
}
#endif /* __PMU_H__ */
