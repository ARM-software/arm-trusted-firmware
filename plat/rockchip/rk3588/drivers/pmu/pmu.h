/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PMU_H__
#define __PMU_H__

#include <lib/mmio.h>

#define PMU0_PWR_CON			0x0000
#define PMU0_WAKEUP_INT_CON		0x0008
#define PMU0_WAKEUP_INT_ST		0x000c
#define PMU0_PMIC_STABLE_CNT_THRES	0x0010
#define PMU0_WAKEUP_RST_CLR_CNT_THRES	0x0014
#define PMU0_OSC_STABLE_CNT_THRES	0x0018
#define PMU0_PWR_CHAIN_STABLE_CON	0x001c
#define PMU0_DDR_RET_CON(i)		(0x0020 + (i) * 4)
#define PMU0_INFO_TX_CON		0x0030

#define PMU1_VERSION_ID			0x4000
#define PMU1_PWR_CON			0x4004
#define PMU1_PWR_FSM			0x4008
#define PMU1_INT_MASK_CON		0x400c
#define PMU1_WAKEUP_INT_CON		0x4010
#define PMU1_WAKEUP_INT_ST		0x4014
#define PMU1_WAKEUP_EDGE_CON		0x4018
#define PMU1_WAKEUP_EDGE_ST		0x401c
#define PMU1_DDR_PWR_CON(i)		(0x4020 + (i) * 4)
#define PMU1_DDR_PWR_SFTCON(i)		(0x4030 + (i) * 4)
#define PMU1_DDR_PWR_FSM		0x4040
#define PMU1_DDR_PWR_ST			0x4044
#define PMU1_CRU_PWR_CON		0x4050
#define PMU1_CRU_PWR_SFTCON		0x4054
#define PMU1_CRU_PWR_FSM		0x4058
#define PMU1_PLLPD_CON(i)		(0x4060 + (i) * 4)
#define PMU1_PLLPD_SFTCON(i)		(0x4068 + (i) * 4)
#define PMU1_STABLE_CNT_THRESH		0x4080
#define PMU1_OSC_STABLE_CNT_THRESH	0x4084
#define PMU1_WAKEUP_RST_CLR_CNT_THRESH	0x4088
#define PMU1_PLL_LOCK_CNT_THRESH	0x408c
#define PMU1_WAKEUP_TIMEOUT_THRESH	0x4094
#define PMU1_PWM_SWITCH_CNT_THRESH	0x4098
#define PMU1_SYS_REG(i)			(0x4100 + (i) * 4)

#define PMU2_PWR_CON1			0x8000
#define PMU2_DSU_PWR_CON		0x8004
#define PMU2_DSU_PWR_SFTCON		0x8008
#define PMU2_DSU_AUTO_PWR_CON		0x800c
#define PMU2_CPU_AUTO_PWR_CON(i)	(0x8010 + (i) * 4)
#define PMU2_CPU_PWR_SFTCON(i)		(0x8030 + (i) * 4)
#define PMU2_CORE_PWR_CON(i)		(0x8050 + (i) * 4)
#define PMU2_CORE_PWR_SFTCON(i)		(0x8058 + (i) * 4)
#define PMU2_CORE_AUTO_PWR_CON(i)	(0x8060 + (i) * 4)
#define PMU2_CLUSTER_NOC_AUTO_CON	0x8068
#define PMU2_CLUSTER_DBG_PWR_CON	0x806c
#define PMU2_CLUSTER_IDLE_CON		0x8070
#define PMU2_CLUSTER_IDLE_SFTCON	0x8074
#define PMU2_CLUSTER_IDLE_ACK		0x8078
#define PMU2_CLUSTER_IDLE_ST		0x807c
#define PMU2_CLUSTER_ST			0x8080
#define PMU2_SCU_PWR_FSM_STATUS(i)	(0x8084 + (i) * 4)
#define PMU2_CORE_PCHANNEL_STATUS(i)	(0x808c + (i) * 4)
#define PMU2_CPU_PWR_CHAIN_STABLE_CON	0x8098
#define PMU2_CLUSTER_MEMPWR_GATE_SFTCON	0x809c
#define PMU2_DSU_STABLE_CNT_THRESH	0x80b0
#define PMU2_DSU_PWRUP_CNT_THRESH	0x80b4
#define PMU2_DSU_PWRDN_CNT_THRESH	0x80b8
#define PMU2_CORE0_STABLE_CNT_THRESH	0x80bc
#define PMU2_CORE0_PWRUP_CNT_THRESH	0x80c0
#define PMU2_CORE0_PWRDN_CNT_THRESH	0x80c4
#define PMU2_CORE1_STABLE_CNT_THRESH	0x80c8
#define PMU2_CORE1_PWRUP_CNT_THRESH	0x80cc
#define PMU2_CORE1_PWRDN_CNT_THRESH	0x80d0
#define PMU2_DBG_RST_CNT_THRESH(i)	(0x80d4 + (i) * 4)
#define PMU2_BUS_IDLE_CON(i)		(0x8100 + (i) * 4)
#define PMU2_BUS_IDLE_SFTCON(i)		(0x810c + (i) * 4)
#define PMU2_BUS_IDLE_ACK(i)		(0x8118 + (i) * 4)
#define PMU2_BUS_IDLE_ST(i)		(0x8120 + (i) * 4)
#define PMU2_BIU_AUTO_CON(i)		(0x8128 + (i) * 4)
#define PMU2_PWR_GATE_CON(i)		(0x8140 + (i) * 4)
#define PMU2_PWR_GATE_SFTCON(i)		(0x814c + (i) * 4)
#define PMU2_VOL_GATE_CON(i)		(0x8158 + (i) * 4)
#define PMU2_PWR_UP_CHAIN_STABLE_CON(i)	(0x8164 + (i) * 4)
#define PMU2_PWR_DWN_CHAIN_STABLE_CON(i)(0x8170 + (i) * 4)
#define PMU2_PWR_STABLE_CHAIN_CNT_THRES	0x817c
#define PMU2_PWR_GATE_ST(i)		(0x8180 + (i) * 4)
#define PMU2_PWR_GATE_FSM		0x8188
#define PMU2_VOL_GATE_FAST_CON		0x818c
#define PMU2_GPU_PWRUP_CNT		0x8190
#define PMU2_GPU_PWRDN_CNT		0x8194
#define PMU2_NPU_PWRUP_CNT		0x8198
#define PMU2_NPU_PWRDN_CNT		0x819c
#define PMU2_MEMPWR_GATE_SFTCON(i)	(0x81a0 + (i) * 4)
#define PMU2_MEMPWR_MD_GATE_SFTCON(i)	(0x81b0 + (i) * 4)
#define PMU2_MEMPWR_MD_GATE_STATUS	0x81bc
#define PMU2_SUBMEM_PWR_ACK_BYPASS(i)	(0x81c0 + (i) * 4)
#define PMU2_QCHANNEL_PWR_CON		0x81d0
#define PMU2_QCHANNEL_PWR_SFTCON	0x81d4
#define PMU2_QCHANNEL_STATUS		0x81d8
#define PMU2_DEBUG_INFO_SEL		0x81e0
#define PMU2_VOP_SUBPD_STATE		0x81e4
#define PMU2_PWR_CHAIN0_ST(i)		(0x81e8 + (i) * 4)
#define PMU2_PWR_CHAIN1_ST(i)		(0x81f0 + (i) * 4)
#define PMU2_PWR_MEM_ST(i)		(0x81f8 + (i) * 4)
#define PMU2_BISR_CON(i)		(0x8200 + (i) * 4)
#define PMU2_BISR_STATUS(i)		(0x8280 + (i) * 4)

#define PMU2_QCH_PWR_MSK		0x7f

#define PD_CTR_LOOP			500
#define PD_CHECK_LOOP			500
#define WFEI_CHECK_LOOP			500
#define BUS_IDLE_LOOP			1000
#define QCH_PWR_LOOP			5000

/* PMU1SCRU */
#define PMU1SCRU_GATE_CON(i)		(0x800 + (i) * 4)

/* PMU_GRF */
#define PMU0_GRF_SOC_CON(i)		((i) * 4)
#define PMU0_GRF_OS_REGS(i)		(0x80 + ((i) - 8) * 4)
#define PMU1_GRF_SOC_CON(i)		((i) * 4)
#define PMU0_GRF_IO_RET_CON(i)		(0x20 + (i) * 4)

/* PMU_SGRF */
#define PMU0_SGRF_SOC_CON(i)		((i) * 4)
#define PMU1_SGRF_SOC_CON(i)		((i) * 4)

/* sys grf */
#define GRF_CPU_STATUS0			0x0420

#define CORES_PM_DISABLE		0x0
#define PD_CHECK_LOOP			500
#define WFEI_CHECK_LOOP			500

/* The ways of cores power domain contorlling */
enum cores_pm_ctr_mode {
	core_pwr_pd = 0,
	core_pwr_wfi = 1,
	core_pwr_wfi_int = 2
};

/* PMU0_PWR_CON */
enum pmu0_pwr_con {
	pmu0_powermode_en = 0,
	pmu0_pmu1_pwr_bypass = 1,
	pmu0_pmu1_bus_bypass = 2,
	pmu0_wkup_bypass = 3,
	pmu0_pmic_bypass = 4,
	pmu0_reset_bypass = 5,
	pmu0_freq_sw_bypass = 6,
	pmu0_osc_dis_bypass = 7,
	pmu0_pmu1_pwr_gt_en = 8,
	pmu0_pmu1_pwr_gt_sft_en = 9,
	pmu0_pmu1_mem_gt_sft_en = 10,
	pmu0_pmu1_bus_idle_en = 11,
	pmu0_pmu1_bus_idle_sft_en = 12,
	pmu0_pmu1_biu_auto_en = 13,
	pmu0_pwr_off_io_en = 14,
};

/* PMU1_PWR_CON */
enum pmu1_pwr_con {
	powermode_en = 0,
	dsu_bypass = 1,
	bus_bypass = 4,
	ddr_bypass = 5,
	pwrdn_bypass = 6,
	cru_bypass = 7,
	qch_bypass = 8,
	core_bypass = 9,
	cpu_sleep_wfi_dis = 12,
};

/* PMU1_DDR_PWR_CON */
enum pmu1_ddr_pwr_con {
	ddr_sref_en = 0,
	ddr_sref_a_en = 1,
	ddrio_ret_en = 2,
	ddrio_ret_exit_en = 5,
	ddrio_rstiov_en = 6,
	ddrio_rstiov_exit_en = 7,
	ddr_gating_a_en = 8,
	ddr_gating_c_en = 9,
	ddr_gating_p_en = 10,
};

/* PMU_CRU_PWR_CON */
enum pmu1_cru_pwr_con {
	alive_32k_en = 0,
	osc_dis_en = 1,
	wakeup_rst_en = 2,
	input_clamp_en = 3,
	alive_osc_mode_en = 4,
	power_off_en = 5,
	pwm_switch_en = 6,
	pwm_gpio_ioe_en = 7,
	pwm_switch_io = 8,
	pd_clk_src_gate_en = 9,
};

/* PMU_PLLPD_CON */
enum pmu1_pllpd_con {
	B0PLL_PD_EN,
	B1PLL_PD_EN,
	LPLL_PD_EN,
	D0APLL_PD_EN,
	D0BPLL_PD_EN,
	D1APLL_PD_EN,
	D1BPLL_PD_EN,
	D2APLL_PD_EN,
	D2BPLL_PD_EN,
	D3APLL_PD_EN,
	D3BPLL_PD_EN,
	V0PLL_PD_EN,
	AUPLL_PD_EN,
	GPLL_PD_EN,
	CPLL_PD_EN,
	NPLL_PD_EN,
	PPLL_PD_EN = 0,
	SPLL_PD_EN = 1,
};

enum pmu1_wakeup_int {
	WAKEUP_CPU0_INT_EN,
	WAKEUP_CPU1_INT_EN,
	WAKEUP_CPU2_INT_EN,
	WAKEUP_CPU3_INT_EN,
	WAKEUP_CPU4_INT_EN,
	WAKEUP_CPU5_INT_EN,
	WAKEUP_CPU6_INT_EN,
	WAKEUP_CPU7_INT_EN,
	WAKEUP_GPIO0_INT_EN,
	WAKEUP_SDMMC_EN,
	WAKEUP_SDIO_EN,
	WAKEUP_USBDEV_EN,
	WAKEUP_UART0_EN,
	WAKEUP_VAD_EN,
	WAKEUP_TIMER_EN,
	WAKEUP_SOC_INT_EN,
	WAKEUP_TIMEROUT_EN,
	WAKEUP_PMUMCU_CEC_EN = 20,
};

enum pmu2_dsu_auto_pwr_con {
	dsu_pm_en = 0,
	dsu_pm_int_wakeup_en = 1,
	dsu_pm_sft_wakeup_en = 3,
};

enum pmu2_cpu_auto_pwr_con {
	cpu_pm_en = 0,
	cpu_pm_int_wakeup_en = 1,
	cpu_pm_sft_wakeup_en = 3,
};

enum pmu2_core_auto_pwr_con {
	core_pm_en = 0,
	core_pm_int_wakeup_en = 1,
	core_pm_int_wakeup_glb_msk = 2,
	core_pm_sft_wakeup_en = 3,
};

enum pmu2_dsu_power_con {
	DSU_PWRDN_EN,
	DSU_PWROFF_EN,
	BIT_FULL_EN,
	DSU_RET_EN,
	CLUSTER_CLK_SRC_GT_EN,
};

enum pmu2_core_power_con {
	CORE_PWRDN_EN,
	CORE_PWROFF_EN,
	CORE_CPU_PWRDN_EN,
	CORE_PWR_CNT_EN,
};

enum pmu2_cluster_idle_con {
	IDLE_REQ_BIGCORE0_EN = 0,
	IDLE_REQ_BIGCORE1_EN = 2,
	IDLE_REQ_DSU_EN = 4,
	IDLE_REQ_LITDSU_EN = 5,
	IDLE_REQ_ADB400_CORE_QCH_EN = 6,
};

enum qos_id {
	QOS_ISP0_MWO = 0,
	QOS_ISP0_MRO = 1,
	QOS_ISP1_MWO = 2,
	QOS_ISP1_MRO = 3,
	QOS_VICAP_M0 = 4,
	QOS_VICAP_M1 = 5,
	QOS_FISHEYE0 = 6,
	QOS_FISHEYE1 = 7,
	QOS_VOP_M0 = 8,
	QOS_VOP_M1 = 9,
	QOS_RKVDEC0 = 10,
	QOS_RKVDEC1 = 11,
	QOS_AV1 = 12,
	QOS_RKVENC0_M0RO = 13,
	QOS_RKVENC0_M1RO = 14,
	QOS_RKVENC0_M2WO = 15,
	QOS_RKVENC1_M0RO = 16,
	QOS_RKVENC1_M1RO = 17,
	QOS_RKVENC1_M2WO = 18,
	QOS_DSU_M0 = 19,
	QOS_DSU_M1 = 20,
	QOS_DSU_MP = 21,
	QOS_DEBUG = 22,
	QOS_GPU_M0 = 23,
	QOS_GPU_M1 = 24,
	QOS_GPU_M2 = 25,
	QOS_GPU_M3 = 26,
	QOS_NPU1 = 27,
	QOS_NPU0_MRO = 28,
	QOS_NPU2 = 29,
	QOS_NPU0_MWR = 30,
	QOS_MCU_NPU = 31,
	QOS_JPEG_DEC = 32,
	QOS_JPEG_ENC0 = 33,
	QOS_JPEG_ENC1 = 34,
	QOS_JPEG_ENC2 = 35,
	QOS_JPEG_ENC3 = 36,
	QOS_RGA2_MRO = 37,
	QOS_RGA2_MWO = 38,
	QOS_RGA3_0 = 39,
	QOS_RGA3_1 = 40,
	QOS_VDPU = 41,
	QOS_IEP = 42,
	QOS_HDCP0 = 43,
	QOS_HDCP1 = 44,
	QOS_HDMIRX = 45,
	QOS_GIC600_M0 = 46,
	QOS_GIC600_M1 = 47,
	QOS_MMU600PCIE_TCU = 48,
	QOS_MMU600PHP_TBU = 49,
	QOS_MMU600PHP_TCU = 50,
	QOS_USB3_0 = 51,
	QOS_USB3_1 = 52,
	QOS_USBHOST_0 = 53,
	QOS_USBHOST_1 = 54,
	QOS_EMMC = 55,
	QOS_FSPI = 56,
	QOS_SDIO = 57,
	QOS_DECOM = 58,
	QOS_DMAC0 = 59,
	QOS_DMAC1 = 60,
	QOS_DMAC2 = 61,
	QOS_GIC600M = 62,
	QOS_DMA2DDR = 63,
	QOS_MCU_DDR = 64,
	QOS_VAD = 65,
	QOS_MCU_PMU = 66,
	QOS_CRYPTOS = 67,
	QOS_CRYPTONS = 68,
	QOS_DCF = 69,
	QOS_SDMMC = 70,
};

enum pmu2_pdid {
	PD_GPU = 0,
	PD_NPU = 1,
	PD_VCODEC = 2,
	PD_NPUTOP = 3,
	PD_NPU1 = 4,
	PD_NPU2 = 5,
	PD_VENC0 = 6,
	PD_VENC1 = 7,
	PD_RKVDEC0 = 8,
	PD_RKVDEC1 = 9,
	PD_VDPU = 10,
	PD_RGA30 = 11,
	PD_AV1 = 12,
	PD_VI = 13,
	PD_FEC = 14,
	PD_ISP1 = 15,
	PD_RGA31 = 16,
	PD_VOP = 17,
	PD_VO0 = 18,
	PD_VO1 = 19,
	PD_AUDIO = 20,
	PD_PHP = 21,
	PD_GMAC = 22,
	PD_PCIE = 23,
	PD_NVM = 24,
	PD_NVM0 = 25,
	PD_SDIO = 26,
	PD_USB = 27,
	PD_SECURE = 28,
	PD_SDMMC = 29,
	PD_CRYPTO = 30,
	PD_CENTER = 31,
	PD_DDR01 = 32,
	PD_DDR23 = 33,
};

enum pmu2_pd_repair_id {
	PD_RPR_PMU = 0,
	PD_RPR_GPU = 1,
	PD_RPR_NPUTOP = 2,
	PD_RPR_NPU1 = 3,
	PD_RPR_NPU2 = 4,
	PD_RPR_VENC0 = 5,
	PD_RPR_VENC1 = 6,
	PD_RPR_RKVDEC0 = 7,
	PD_RPR_RKVDEC1 = 8,
	PD_RPR_VDPU = 9,
	PD_RPR_RGA30 = 10,
	PD_RPR_AV1 = 11,
	PD_RPR_VI = 12,
	PD_RPR_FEC = 13,
	PD_RPR_ISP1 = 14,
	PD_RPR_RGA31 = 15,
	PD_RPR_VOP = 16,
	PD_RPR_VO0 = 17,
	PD_RPR_VO1 = 18,
	PD_RPR_AUDIO = 19,
	PD_RPR_PHP = 20,
	PD_RPR_GMAC = 21,
	PD_RPR_PCIE = 22,
	PD_RPR_NVM0 = 23,
	PD_RPR_SDIO = 24,
	PD_RPR_USB = 25,
	PD_RPR_SDMMC = 26,
	PD_RPR_CRYPTO = 27,
	PD_RPR_CENTER = 28,
	PD_RPR_DDR01 = 29,
	PD_RPR_DDR23 = 30,
	PD_RPR_BUS = 31,
};

enum pmu2_bus_id {
	BUS_ID_GPU = 0,
	BUS_ID_NPUTOP = 1,
	BUS_ID_NPU1 = 2,
	BUS_ID_NPU2 = 3,
	BUS_ID_RKVENC0 = 4,
	BUS_ID_RKVENC1 = 5,
	BUS_ID_RKVDEC0 = 6,
	BUS_ID_RKVDEC1 = 7,
	BUS_ID_VDPU = 8,
	BUS_ID_AV1 = 9,
	BUS_ID_VI = 10,
	BUS_ID_ISP = 11,
	BUS_ID_RGA31 = 12,
	BUS_ID_VOP = 13,
	BUS_ID_VOP_CHANNEL = 14,
	BUS_ID_VO0 = 15,
	BUS_ID_VO1 = 16,
	BUS_ID_AUDIO = 17,
	BUS_ID_NVM = 18,
	BUS_ID_SDIO = 19,
	BUS_ID_USB = 20,
	BUS_ID_PHP = 21,
	BUS_ID_VO1USBTOP = 22,
	BUS_ID_SECURE = 23,
	BUS_ID_SECURE_CENTER_CHANNEL = 24,
	BUS_ID_SECURE_VO1USB_CHANNEL = 25,
	BUS_ID_CENTER = 26,
	BUS_ID_CENTER_CHANNEL = 27,
	BUS_ID_MSCH0 = 28,
	BUS_ID_MSCH1 = 29,
	BUS_ID_MSCH2 = 30,
	BUS_ID_MSCH3 = 31,
	BUS_ID_MSCH = 32,
	BUS_ID_BUS = 33,
	BUS_ID_TOP = 34,
};

enum pmu2_mem_st {
	PD_NPU_TOP_MEM_ST = 11,
	PD_NPU1_MEM_ST = 12,
	PD_NPU2_MEM_ST = 13,
	PD_VENC0_MEM_ST = 14,
	PD_VENC1_MEM_ST = 15,
	PD_RKVDEC0_MEM_ST = 16,
	PD_RKVDEC1_MEM_ST = 17,
	PD_RGA30_MEM_ST = 19,
	PD_AV1_MEM_ST = 20,
	PD_VI_MEM_ST = 21,
	PD_FEC_MEM_ST = 22,
	PD_ISP1_MEM_ST = 23,
	PD_RGA31_MEM_ST = 24,
	PD_VOP_MEM_ST = 25,
	PD_VO0_MEM_ST = 26,
	PD_VO1_MEM_ST = 27,
	PD_AUDIO_MEM_ST = 28,
	PD_PHP_MEM_ST = 29,
	PD_GMAC_MEM_ST = 30,
	PD_PCIE_MEM_ST = 31,
	PD_NVM0_MEM_ST = 33,
	PD_SDIO_MEM_ST = 34,
	PD_USB_MEM_ST = 35,
	PD_SDMMC_MEM_ST = 37,
};

enum pmu2_qid {
	QID_PHPMMU_TBU = 0,
	QID_PHPMMU_TCU = 1,
	QID_PCIEMMU_TBU0 = 2,
	QID_PCIEMU_TCU = 3,
	QID_PHP_GICITS = 4,
	QID_BUS_GICITS0 = 5,
	QID_BUS_GICITS1 = 6,
};

/* PMU_DSU_PWR_CON */
enum pmu_dsu_pwr_con {
	DSU_PWRDN_ENA = 2,
	DSU_PWROFF_ENA,
	DSU_RET_ENA = 6,
	CLUSTER_CLK_SRC_GATE_ENA,
	DSU_PWR_CON_END
};

enum cpu_power_state {
	CPU_POWER_ON,
	CPU_POWER_OFF,
	CPU_EMULATION_OFF,
	CPU_RETENTION,
	CPU_DEBUG
};

enum dsu_power_state {
	DSU_POWER_ON,
	CLUSTER_TRANSFER_IDLE,
	DSU_POWER_DOWN,
	DSU_OFF,
	DSU_WAKEUP,
	DSU_POWER_UP,
	CLUSTER_TRANSFER_RESUME,
	DSU_FUNCTION_RETENTION
};

/* PMU2_CLUSTER_STS 0x8080 */
enum pmu2_cluster_sts_bits {
	pd_cpu0_dwn = 0,
	pd_cpu1_dwn,
	pd_cpu2_dwn,
	pd_cpu3_dwn,
	pd_cpu4_dwn,
	pd_cpu5_dwn,
	pd_cpu6_dwn,
	pd_cpu7_dwn,
	pd_core0_dwn,
	pd_core1_dwn
};

#define CLUSTER_STS_NONBOOT_CPUS_DWN	0xfe

enum cpu_off_trigger {
	CPU_OFF_TRIGGER_WFE = 0,
	CPU_OFF_TRIGGER_REQ_EML,
	CPU_OFF_TRIGGER_REQ_WFI,
	CPU_OFF_TRIGGER_REQ_WFI_NBT_CPU,
	CPU_OFF_TRIGGER_REQ_WFI_NBT_CPU_SRAM
};

/*****************************************************************************
 * power domain on or off
 *****************************************************************************/
enum pmu_pd_state {
	pmu_pd_on = 0,
	pmu_pd_off = 1
};

enum bus_state {
	bus_active,
	bus_idle,
};

#define RK_CPU_STATUS_OFF		0
#define RK_CPU_STATUS_ON		1
#define RK_CPU_STATUS_BUSY		-1

#define PD_CTR_LOOP			500
#define MAX_WAIT_COUNT			500

#define pmu_bus_idle_st(id)	\
	(!!(mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST((id) / 32)) & BIT((id) % 32)))

#define pmu_bus_idle_ack(id)	\
	(!!(mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ACK((id) / 32)) & BIT((id) % 32)))

void pm_pll_wait_lock(uint32_t pll_base);
#endif /* __PMU_H__ */
