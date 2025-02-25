/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#ifndef __PMU_H__
#define __PMU_H__

#include <assert.h>

#include <mmio.h>

/* PMU */
#define PMU1_OFFSET			0x10000
#define PMU2_OFFSET			0x20000

#define PMU0_PWR_CON			0x0000
#define PMU0_PWR_STATUS			0x0004
#define PMU0_WAKEUP_INT_CON		0x0008
#define PMU0_WAKEUP_INT_ST		0x000c
#define PMU0_PMIC_STABLE_CNT_THRES	0x0010
#define PMU0_WAKEUP_RST_CLR_CNT_THRES	0x0014
#define PMU0_OSC_STABLE_CNT_THRES	0x0018
#define PMU0_PWR_C0_STABLE_CON		0x001c
#define PMU0_DDR_RET_CON(i)		(0x0020 + (i) * 4)
#define PMU0_INFO_TX_CON		0x0030

#define PMU1_VERSION_ID			(PMU1_OFFSET + 0x0000)
#define PMU1_PWR_CON			(PMU1_OFFSET + 0x0004)
#define PMU1_PWR_FSM			(PMU1_OFFSET + 0x0008)
#define PMU1_INT_MASK_CON		(PMU1_OFFSET + 0x000c)
#define PMU1_WAKEUP_INT_CON		(PMU1_OFFSET + 0x0010)
#define PMU1_WAKEUP_INT_ST		(PMU1_OFFSET + 0x0014)
#define PMU1_DDR_PWR_CON(i)		(PMU1_OFFSET + 0x0100 + (i) * 4)
#define PMU1_DDR_PWR_SFTCON(i)		(PMU1_OFFSET + 0x0110 + (i) * 4)
#define PMU1_DDR_AXIPWR_CON(i)		(PMU1_OFFSET + 0x0120 + (i) * 4)
#define PMU1_DDR_AXIPWR_SFTCON(i)	(PMU1_OFFSET + 0x0130 + (i) * 4)
#define PMU1_DDR_PWR_FSM		(PMU1_OFFSET + 0x0140)
#define PMU1_DDR_PWR_ST			(PMU1_OFFSET + 0x0144)
#define PMU1_DDR_AXIPWR_ST		(PMU1_OFFSET + 0x0148)
#define PMU1_CRU_PWR_CON(i)		(PMU1_OFFSET + 0x0200 + (i) * 4)
#define PMU1_CRU_PWR_SFTCON(i)		(PMU1_OFFSET + 0x0208 + (i) * 4)
#define PMU1_CRU_PWR_FSM		(PMU1_OFFSET + 0x0210)
#define PMU1_PLLPD_CON(i)		(PMU1_OFFSET + 0x0220 + (i) * 4)
#define PMU1_PLLPD_SFTCON(i)		(PMU1_OFFSET + 0x0228 + (i) * 4)
#define PMU1_STABLE_CNT_THRESH		(PMU1_OFFSET + 0x0300)
#define PMU1_OSC_STABLE_CNT_THRESH	(PMU1_OFFSET + 0x0304)
#define PMU1_WAKEUP_RST_CLR_CNT_THRESH	(PMU1_OFFSET + 0x0308)
#define PMU1_PLL_LOCK_CNT_THRESH	(PMU1_OFFSET + 0x030c)
#define PMU1_WAKEUP_TIMEOUT_THRESH	(PMU1_OFFSET + 0x0310)
#define PMU1_PWM_SWITCH_CNT_THRESH	(PMU1_OFFSET + 0x0314)
#define PMU1_SLEEP_CNT_THRESH		(PMU1_OFFSET + 0x0318)
#define PMU1_INFO_TX_CON		(PMU1_OFFSET + 0x0400)

#define PMU2_SCU0_PWR_CON		(PMU2_OFFSET + 0x0000)
#define PMU2_SCU1_PWR_CON		(PMU2_OFFSET + 0x0004)
#define PMU2_SCU0_PWR_SFTCON		(PMU2_OFFSET + 0x0008)
#define PMU2_SCU1_PWR_SFTCON		(PMU2_OFFSET + 0x000c)
#define PMU2_SCU0_AUTO_PWR_CON		(PMU2_OFFSET + 0x0010)
#define PMU2_SCU1_AUTO_PWR_CON		(PMU2_OFFSET + 0x0014)
#define PMU2_SCU_PWR_FSM_STATUS		(PMU2_OFFSET + 0x0018)
#define PMU2_DBG_PWR_CON(i)		(PMU2_OFFSET + 0x001c + (i) * 4)
#define PMU2_CLUSTER_PWR_ST		(PMU2_OFFSET + 0x0024)
#define PMU2_CLUSTER0_IDLE_CON		(PMU2_OFFSET + 0x0028)
#define PMU2_CLUSTER1_IDLE_CON		(PMU2_OFFSET + 0x002c)
#define PMU2_CLUSTER0_IDLE_SFTCON	(PMU2_OFFSET + 0x0030)
#define PMU2_CLUSTER1_IDLE_SFTCON	(PMU2_OFFSET + 0x0034)
#define PMU2_CLUSTER_IDLE_ACK		(PMU2_OFFSET + 0x0038)
#define PMU2_CLUSTER_IDLE_ST		(PMU2_OFFSET + 0x003c)
#define PMU2_SCU0_PWRUP_CNT_THRESH	(PMU2_OFFSET + 0x0040)
#define PMU2_SCU0_PWRDN_CNT_THRESH	(PMU2_OFFSET + 0x0044)
#define PMU2_SCU0_STABLE_CNT_THRESH	(PMU2_OFFSET + 0x0048)
#define PMU2_SCU1_PWRUP_CNT_THRESH	(PMU2_OFFSET + 0x004c)
#define PMU2_SCU1_PWRDN_CNT_THRESH	(PMU2_OFFSET + 0x0050)
#define PMU2_SCU1_STABLE_CNT_THRESH	(PMU2_OFFSET + 0x0054)
#define PMU2_CPU_AUTO_PWR_CON(i)	(PMU2_OFFSET + 0x0080 + ((i)) * 4)
#define PMU2_CPU_PWR_SFTCON(i)		(PMU2_OFFSET + 0x00a0 + ((i)) * 4)
#define PMU2_CCI_PWR_CON		(PMU2_OFFSET + 0x00e0)
#define PMU2_CCI_PWR_SFTCON		(PMU2_OFFSET + 0x00e4)
#define PMU2_CCI_PWR_ST			(PMU2_OFFSET + 0x00e8)
#define PMU2_CCI_POWER_STATE		(PMU2_OFFSET + 0x00ec)
#define PMU2_BUS_IDLE_CON(i)		(PMU2_OFFSET + 0x0100 + (i) * 4)
#define PMU2_BUS_IDLE_SFTCON(i)		(PMU2_OFFSET + 0x0110 + (i) * 4)
#define PMU2_BUS_IDLE_ACK		(PMU2_OFFSET + 0x0120)
#define PMU2_BUS_IDLE_ST		(PMU2_OFFSET + 0x0128)
#define PMU2_NOC_AUTO_CON(i)		(PMU2_OFFSET + 0x0130 + (i) * 4)
#define PMU2_NOC_AUTO_SFTCON(i)		(PMU2_OFFSET + 0x0140 + (i) * 4)
#define PMU2_BUS_IDLEACK_BYPASS_CON	(PMU2_OFFSET + 0x0150)
#define PMU2_PWR_GATE_CON(i)		(PMU2_OFFSET + 0x0200 + (i) * 4)
#define PMU2_PWR_GATE_SFTCON(i)		(PMU2_OFFSET + 0x0210 + (i) * 4)
#define PMU2_VOL_GATE_SFTCON(i)		(PMU2_OFFSET + 0x0220 + (i) * 4)
#define PMU2_PWR_GATE_ST		(PMU2_OFFSET + 0x0230)
#define PMU2_PWR_GATE_FSM		(PMU2_OFFSET + 0x0238)
#define PMU2_PD_DWN_ACK_STATE(i)	(PMU2_OFFSET + 0x0240 + (i) * 4)
#define PMU2_PD_DWN_LC_ACK_STATE(i)	(PMU2_OFFSET + 0x0248 + (i) * 4)
#define PMU2_PD_DWN_MEM_ACK_STATE(i)	(PMU2_OFFSET + 0x0250 + (i) * 4)
#define PMU2_PWR_UP_C0_STABLE_CON(i)	(PMU2_OFFSET + 0x0260 + (i) * 4)
#define PMU2_PWR_DWN_C0_STABLE_CON(i)	(PMU2_OFFSET + 0x0270 + (i) * 4)
#define PMU2_PWR_STABLE_C0_CNT_THRES	(PMU2_OFFSET + 0x027c)
#define PMU2_FAST_POWER_CON		(PMU2_OFFSET + 0x0284)
#define PMU2_FAST_PWRUP_CNT_THRESH_0	(PMU2_OFFSET + 0x0288)
#define PMU2_FAST_PWRDN_CNT_THRESH_0	(PMU2_OFFSET + 0x028c)
#define PMU2_FAST_PWRUP_CNT_THRESH_1	(PMU2_OFFSET + 0x0290)
#define PMU2_FAST_PWRDN_CNT_THRESH_1	(PMU2_OFFSET + 0x0294)
#define PMU2_FAST_PWRUP_CNT_THRESH_2	(PMU2_OFFSET + 0x0298)
#define PMU2_FAST_PWRDN_CNT_THRESH_2	(PMU2_OFFSET + 0x029c)
#define PMU2_MEM_PWR_GATE_SFTCON(i)	(PMU2_OFFSET + 0x0300)
#define PMU2_SUBMEM_PWR_GATE_SFTCON(i)	(PMU2_OFFSET + 0x0310)
#define PMU2_SUBMEM_PWR_ACK_BYPASS_SFTCON(i)	(PMU2_OFFSET + 0x0320)
#define PMU2_SUBMEM_PWR_GATE_STATUS	(PMU2_OFFSET + 0x0328)
#define PMU2_QCHANNEL_PWR_CON0		(PMU2_OFFSET + 0x0400)
#define PMU2_QCHANNEL_PWR_SFTCON0	(PMU2_OFFSET + 0x0404)
#define PMU2_QCHANNEL_STATUS0		(PMU2_OFFSET + 0x0408)
#define PMU2_C0_PWRACK_BYPASS_CON(i)	(PMU2_OFFSET + 0x0380 + (i) * 4)
#define PMU2_C1_PWRACK_BYPASS_CON(i)	(PMU2_OFFSET + 0x0390 + (i) * 4)
#define PMU2_C2_PWRACK_BYPASS_CON(i)	(PMU2_OFFSET + 0x03a0 + (i) * 4)
#define PMU2_DEBUG_INFO_SEL		(PMU2_OFFSET + 0x03f0)
#define PMU2_BISR_GLB_CON		(PMU2_OFFSET + 0x500)
#define PMU2_BISR_TIMEOUT_THRES		(PMU2_OFFSET + 0x504)
#define PMU2_BISR_PDGEN_CON(i)		(PMU2_OFFSET + 0x510 + (i) * 4)
#define PMU2_BISR_PDGEN_SFTCON(i)	(PMU2_OFFSET + 0x520 + (i) * 4)
#define PMU2_BISR_PDGDONE_CON(i)	(PMU2_OFFSET + 0x530 + (i) * 4)
#define PMU2_BISR_PDGINIT_CON(i)	(PMU2_OFFSET + 0x540 + (i) * 4)
#define PMU2_BISR_PDGDONE_STATUS(i)	(PMU2_OFFSET + 0x550 + (i) * 4)
#define PMU2_BISR_PDGCEDIS_STATUS(i)	(PMU2_OFFSET + 0x560 + (i) * 4)
#define PMU2_BISR_PWR_REPAIR_STATUS(i)	(PMU2_OFFSET + 0x570 + (i) * 4)

/* PMU1CRU */
#define PMU1CRU_CLKSEL_CON(i)		((i) * 0x4 + 0x300)
#define PMU1CRU_CLKSEL_CON_CNT		22
#define PMU1CRU_CLKGATE_CON(i)		((i) * 0x4 + 0x800)
#define PMU1CRU_CLKGATE_CON_CNT		8
#define PMU1CRU_SOFTRST_CON(i)		((i) * 0x4 + 0xa00)
#define PMU1CRU_SOFTRST_CON_CNT		8
#define PMU1CRU_DEEPSLOW_DET_CON	0xb40
#define PMU1CRU_DEEPSLOW_DET_ST		0xb44

/* PMU1SCRU */
#define PMU1SCRU_CLKSEL_CON(i)		((i) * 0x4 + 0x4000)
#define PMU1SCRU_CLKSEL_CON_CNT		3
#define PMU1SCRU_CLKGATE_CON(i)		((i) * 0x4 + 0x4028)
#define PMU1SCRU_CLKGATE_CON_CNT		3
#define PMU1SCRU_SOFTRST_CON(i)		((i) * 0x4 + 0x4050)
#define PMU1SCRU_SOFTRST_CONCNT		3

/* PMU0GRF */
#define PMU0GRF_SOC_CON(i)		((i) * 4)
#define PMU0GRF_IO_RET_CON(i)		(0x20 + (i) * 4)
#define PMU0GRF_OS_REG(i)		((i) * 4)

/* PMU1GRF */
#define PMU1GRF_SOC_CON(i)		((i) * 4)
#define PMU1GRF_SOC_ST			0x60
#define PMU1GRF_MEM_CON(i)		(0x80 + (i) * 4)
#define PMU1GRF_OS_REG(i)		(0x200 + (i) * 4)

#define PMU_MCU_HALT			BIT(7)
#define PMU_MCU_SLEEP			BIT(9)
#define PMU_MCU_DEEPSLEEP		BIT(10)
#define PMU_MCU_STOP_MSK		\
	(PMU_MCU_HALT | PMU_MCU_SLEEP | PMU_MCU_DEEPSLEEP)

#define CORES_PM_DISABLE		0x0

/* pmuioc */
#define PMUIO0_IOC_GPIO0A_IOMUX_SEL_L	0x000
#define PMUIO0_IOC_GPIO0A_IOMUX_SEL_H	0x004
#define PMUIO0_IOC_GPIO0B_IOMUX_SEL_L	0x008

#define PMUIO1_IOC_GPIO0B_IOMUX_SEL_H	0x000
#define PMUIO1_IOC_GPIO0C_IOMUX_SEL_L	0x004
#define PMUIO1_IOC_GPIO0C_IOMUX_SEL_H	0x008
#define PMUIO1_IOC_GPIO0D_IOMUX_SEL_L	0x00c
#define PMUIO1_IOC_GPIO0D_IOMUX_SEL_H	0x010

/* PMU_PWR_CON */
enum pmu0_pwr_con {
	pmu_powermode0_en = 0,
	pmu_pmu1_pd_byp = 1,
	pmu_pmu1_bus_byp = 2,
	pmu_pmu0_wkup_byp = 3,
	pmu_pmu0_pmic_byp = 4,
	pmu_pmu0_reset_byp = 5,
	pmu_pmu0_freq_switch_byp = 6,
	pmu_pmu0_osc_dis_byp = 7,
	pmu_pmu1_pwrgt = 8,
	pmu_pmu1_pwrgt_sft = 9,
	pmu_pmu1_mempwr_sft_gt = 10,
	pmu_pmu1_idle_en = 11,
	pmu_pmu1_idle_sft_en = 12,
	pmu_pmu1_noc_auto_en = 13,
	pmu_pmu1_off_io_en = 14,
};

enum pmu1_pwr_con {
	pmu_powermode_en = 0,
	pmu_scu0_byp = 1,
	pmu_scu1_byp = 2,
	pmu_cci_byp = 3,
	pmu_bus_byp = 4,
	pmu_ddr_byp = 5,
	pmu_pwrgt_byp = 6,
	pmu_cru_byp = 7,
	pmu_qch_byp = 8,
	pmu_wfi_byp = 12,
	pmu_slp_cnt_en = 13,
};

enum pmu_wakeup_int {
	pmu_wkup_cpu0_int = 0,
	pmu_wkup_cpu1_int = 1,
	pmu_wkup_cpu2_int = 2,
	pmu_wkup_cpu3_int = 3,
	pmu_wkup_cpu4_int = 4,
	pmu_wkup_cpu5_int = 5,
	pmu_wkup_cpu6_int = 6,
	pmu_wkup_cpu7_int = 7,
	pmu_wkup_gpio0_int = 8,
	pmu_wkup_sdmmc_int = 9,
	pmu_wkup_sdio_int = 10,
	pmu_wkup_usbdev_int = 11,
	pmu_wkup_uart_int = 12,
	pmu_wkup_mcu_int = 13,
	pmu_wkup_timer_int = 14,
	pmu_wkup_sys_int = 15,
	pmu_wkup_pwm_int = 16,
	pmu_wkup_tsadc_int = 17,
	pmu_wkup_hptimer_int = 18,
	pmu_wkup_saradc_int = 19,
	pmu_wkup_timeout = 20,
};

/* PMU_DDR_PWR_CON */
enum pmu_ddr_pwr_con {
	pmu_ddr_sref_c_en = 0,
	pmu_ddr_ioret_en = 1,
	pmu_ddr_ioret_exit_en = 2,
	pmu_ddr_rstiov_en = 3,
	pmu_ddr_rstiov_exit_en = 4,
	pmu_ddr_gating_c_en = 5,
	pmu_ddr_gating_p_en = 6,
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
	pmu_io_sleep_en = 9,
};

/* PMU_CRU_PWR_CON1 */
enum pmu_cru_pwr_con1 {
	pmu_bus_clksrc_gt_en = 0,
	pmu_vpu_clksrc_gt_en = 1,
	pmu_vo_clksrc_gt_en = 2,
	pmu_gpu_clksrc_gt_en = 3,
	pmu_rkenc_clksrc_gt_en = 4,
	pmu_rkvdec_clksrc_gt_en = 5,
	pmu_core_clksrc_gt_en = 6,
	pmu_ddr_clksrc_gt_en = 7,
};

/* PMU_SCU_PWR_CON */
enum pmu_scu_pwr_con {
	pmu_l2_flush_en = 0,
	pmu_l2_ilde_en = 1,
	pmu_scu_pd_en = 2,
	pmu_scu_pwroff_en = 3,
	pmu_clst_cpu_pd_en = 5,
	pmu_std_wfi_bypass = 8,
	pmu_std_wfil2_bypass = 9,
	pmu_scu_vol_gt_en = 10,
};

/* PMU_PLLPD_CON */
enum pmu_pllpd_con {
	pmu_d0apll_pd_en = 0,
	pmu_d0bpll_pd_en = 1,
	pmu_d1apll_pd_en = 2,
	pmu_d1bpll_pd_en = 3,
	pmu_bpll_pd_en = 4,
	pmu_lpll_pd_en = 5,
	pmu_spll_pd_en = 6,
	pmu_gpll_pd_en = 7,
	pmu_cpll_pd_en = 8,
	pmu_ppll_pd_en = 9,
	pmu_aupll_pd_en = 10,
	pmu_vpll_pd_en = 11,
};

/* PMU_CLST_PWR_ST */
enum pmu_clst_pwr_st {
	pmu_cpu0_wfi = 0,
	pmu_cpu1_wfi = 1,
	pmu_cpu2_wfi = 2,
	pmu_cpu3_wfi = 3,
	pmu_cpu4_wfi = 4,
	pmu_cpu5_wfi = 5,
	pmu_cpu6_wfi = 6,
	pmu_cpu7_wfi = 7,
	pmu_scu0_standbywfil2 = 8,
	pmu_scu1_standbywfil2 = 9,
	pmu_scu0_l2flushdone = 10,
	pmu_scu1_l2flushdone = 11,
	pmu_cpu0_pd_st = 16,
	pmu_cpu1_pd_st = 17,
	pmu_cpu2_pd_st = 18,
	pmu_cpu3_pd_st = 19,
	pmu_cpu4_pd_st = 20,
	pmu_cpu5_pd_st = 21,
	pmu_cpu6_pd_st = 22,
	pmu_cpu7_pd_st = 23,
	pmu_scu0_pd_st = 24,
	pmu_scu1_pd_st = 25,
};

/* PMU_CLST_IDLE_CON */
enum pmu_clst_idle_con {
	pmu_adb400s_idle_req = 0,
	pmu_clst_biu_idle_req = 1,
	pmu_clst_clk_gt_msk = 2,
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
};

enum qos_id {
	qos_decom = 0,
	qos_dmac0 = 1,
	qos_dmac1 = 2,
	qos_dmac2 = 3,
	qos_bus_mcu = 4,
	qos_can0 = 5,
	qos_can1 = 6,
	qos_cci_m0 = 7,
	qos_cci_m1 = 8,
	qos_cci_m2 = 9,
	qos_dap_lite = 10,
	qos_hdcp1 = 11,
	qos_ddr_mcu = 12,
	qos_fspi1 = 13,
	qos_gmac0 = 14,
	qos_gmac1 = 15,
	qos_sdio = 16,
	qos_sdmmc = 17,
	qos_flexbus = 18,
	qos_gpu = 19,
	qos_vepu1 = 20,
	qos_npu_mcu = 21,
	qos_npu_nsp0 = 22,
	qos_npu_nsp1 = 23,
	qos_npu_m0 = 24,
	qos_npu_m1 = 25,
	qos_npu_m0ro = 26,
	qos_npu_m1ro = 27,
	qos_emmc = 28,
	qos_fspi0 = 29,
	qos_mmu0 = 30,
	qos_mmu1 = 31,
	qos_pmu_mcu = 32,
	qos_rkvdec = 33,
	qos_crypto = 34,
	qos_mmu2 = 35,
	qos_ufshc = 36,
	qos_vepu0 = 37,
	qos_isp_mro = 38,
	qos_isp_mwo = 39,
	qos_vicap_m0 = 40,
	qos_vpss_mro = 41,
	qos_vpss_mwo = 42,
	qos_hdcp0 = 43,
	qos_vop_m0 = 44,
	qos_vop_m1ro = 45,
	qos_ebc = 46,
	qos_rga0 = 47,
	qos_rga1 = 48,
	qos_jpeg = 49,
	qos_vdpp = 50,
	qos_dma2ddr = 51,
};

enum pmu_bus_id {
	pmu_bus_id_gpu = 0,
	pmu_bus_id_npu0 = 1,
	pmu_bus_id_npu1 = 2,
	pmu_bus_id_nputop = 3,
	pmu_bus_id_npusys = 4,
	pmu_bus_id_vpu = 5,
	pmu_bus_id_vdec = 6,
	pmu_bus_id_vepu0 = 7,
	pmu_bus_id_vepu1 = 8,
	pmu_bus_id_vi = 9,
	pmu_bus_id_usb = 10,
	pmu_bus_id_vo0 = 11,
	pmu_bus_id_vo1 = 12,
	pmu_bus_id_vop = 13,
	pmu_bus_id_vop_nocddrsch = 14,
	pmu_bus_id_php = 15,
	pmu_bus_id_audio = 16,
	pmu_bus_id_gmac = 17,
	pmu_bus_id_nvm = 18,
	pmu_bus_id_center_nocddrsch = 19,
	pmu_bus_id_center_nocmain = 20,
	pmu_bus_id_ddr = 21,
	pmu_bus_id_ddrsch0 = 22,
	pmu_bus_id_ddrsch1 = 23,
	pmu_bus_id_bus = 24,
	pmu_bus_id_secure = 25,
	pmu_bus_id_top = 26,
	pmu_bus_id_vo0vop_chn = 27,
	pmu_bus_id_cci = 28,
	pmu_bus_id_cci_nocddrsch = 29,
	pmu_bus_id_max,
};

enum pmu_pd_id {
	pmu_pd_npu = 0,
	pmu_pd_bus = 1,
	pmu_pd_secure = 2,
	pmu_pd_center = 3,
	pmu_pd_ddr = 4,
	pmu_pd_cci = 5,
	pmu_pd_nvm = 6,
	pmu_pd_sd_gmac = 7,
	pmu_pd_audio = 8,
	pmu_pd_php = 9,
	pmu_pd_subphp = 10,
	pmu_pd_vop = 11,
	pmu_pd_vop_smart = 12,
	pmu_pd_vop_clst = 13,
	pmu_pd_vo1 = 14,
	pmu_pd_vo0 = 15,
	pmu_pd_usb = 16,
	pmu_pd_vi = 17,
	pmu_pd_vepu0 = 18,
	pmu_pd_vepu1 = 19,
	pmu_pd_vdec = 20,
	pmu_pd_vpu = 21,
	pmu_pd_nputop = 22,
	pmu_pd_npu0 = 23,
	pmu_pd_npu1 = 24,
	pmu_pd_gpu = 25,
	pmu_pd_id_max,
};

enum pmu_vd_id {
	pmu_vd_npu = 0,
	pmu_vd_ddr = 1,
	pmu_vd_cci = 2,
	pmu_vd_gpu = 3,
};

enum pmu_bus_state {
	pmu_bus_active = 0,
	pmu_bus_idle = 1,
};

enum pmu_pd_state {
	pmu_pd_on = 0,
	pmu_pd_off = 1
};

enum pmu_scu_fsm_st {
	pmu_scu_fsm_normal = 0,
	pmu_scu_fsm_cpu_pwr_down = 1,
	pmu_scu_fsm_l2_flush = 2,
	pmu_scu_fsm_l2_idle = 3,
	pmu_scu_fsm_clust_idle = 4,
	pmu_scu_fsm_scu_pwr_down = 5,
	pmu_scu_fsm_sleep = 6,
	pmu_scu_fsm_wkup = 7,
	pmu_scu_fsm_scu_pwr_up = 8,
	pmu_scu_fsm_clust_resume = 9,
	pmu_scu_fsm_cpu_pwr_up = 10,
};

#define MAX_MEM_OS_REG_NUM		32
#define MEM_OS_REG_BASE			\
	(PMUSRAM_BASE + PMUSRAM_RSIZE - MAX_MEM_OS_REG_NUM * 4)

#define PSRAM_SP_TOP			MEM_OS_REG_BASE

#define PD_CTR_LOOP			5000
#define WFEI_CHECK_LOOP			5000
#define BUS_IDLE_LOOP			1000
#define NONBOOT_CPUS_OFF_LOOP		500000

#define REBOOT_FLAG			0x5242C300
#define BOOT_BROM_DOWNLOAD		0xef08a53c

#define BOOTROM_SUSPEND_MAGIC		0x02468ace
#define BOOTROM_RESUME_MAGIC		0x13579bdf
#define WARM_BOOT_MAGIC			0x76543210
#define VALID_GLB_RST_MSK		0xbfff

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

#define SLP_32K_EXT			BIT(24)
#define SLP_TIME_OUT_WKUP		BIT(25)
#define SLP_PMU_DBG			BIT(26)
#define SLP_ARCH_TIMER_RESET		BIT(27)

#define PM_INVALID_GPIO			0xffff
#define MAX_GPIO_POWER_CFG_CNT		10
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

#define pmu_bus_idle_st(id)	\
	(!!(mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST) & BIT(id)))

#define pmu_bus_idle_ack(id)	\
	(!!(mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ACK) & BIT(id)))

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
