/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PMU_H__
#define __PMU_H__

/* Needed aligned 16 bytes for sp stack top */
#define PSRAM_SP_TOP		((PMUSRAM_BASE + PMUSRAM_RSIZE) & ~0xf)

/*****************************************************************************
 * pmu con,reg
 *****************************************************************************/
#define PMU_WKUP_CFG0_LO		0x00
#define PMU_WKUP_CFG0_HI		0x04
#define PMU_WKUP_CFG1_LO		0x08
#define PMU_WKUP_CFG1_HI		0x0c
#define PMU_WKUP_CFG2_LO		0x10

#define PMU_PWRDN_CON			0x18
#define PMU_PWRDN_ST			0x20

#define PMU_PWRMODE_CORE_LO		0x24
#define PMU_PWRMODE_CORE_HI		0x28
#define PMU_PWRMODE_COMMON_CON_LO	0x2c
#define PMU_PWRMODE_COMMON_CON_HI	0x30

#define PMU_SFT_CON			0x34
#define PMU_INT_ST			0x44
#define PMU_BUS_IDLE_REQ		0x64
#define PMU_BUS_IDLE_ST			0x6c

#define PMU_OSC_CNT_LO			0x74
#define PMU_OSC_CNT_HI			0x78
#define PMU_PLLLOCK_CNT_LO		0x7c
#define PMU_PLLLOCK_CNT_HI		0x80
#define PMU_PLLRST_CNT_LO		0x84
#define PMU_PLLRST_CNT_HI		0x88
#define PMU_STABLE_CNT_LO		0x8c
#define PMU_STABLE_CNT_HI		0x90
#define PMU_WAKEUP_RST_CLR_LO		0x9c
#define PMU_WAKEUP_RST_CLR_HI		0xa0

#define PMU_DDR_SREF_ST			0xa4

#define PMU_SYS_REG0_LO			0xa8
#define PMU_SYS_REG0_HI			0xac
#define PMU_SYS_REG1_LO			0xb0
#define PMU_SYS_REG1_HI			0xb4
#define PMU_SYS_REG2_LO			0xb8
#define PMU_SYS_REG2_HI			0xbc
#define PMU_SYS_REG3_LO			0xc0
#define PMU_SYS_REG3_HI			0xc4

#define PMU_SCU_PWRDN_CNT_LO		0xc8
#define PMU_SCU_PWRDN_CNT_HI		0xcc
#define PMU_SCU_PWRUP_CNT_LO		0xd0
#define PMU_SCU_PWRUP_CNT_HI		0xd4

#define PMU_TIMEOUT_CNT_LO		0xd8
#define PMU_TIMEOUT_CNT_HI		0xdc

#define PMU_CPUAPM_CON(cpu)		(0xe0 + (cpu) * 0x4)

#define CORES_PM_DISABLE	0x0
#define CLST_CPUS_MSK		0xf

#define PD_CTR_LOOP		500
#define PD_CHECK_LOOP		500
#define WFEI_CHECK_LOOP		500
#define BUS_IDLE_LOOP		1000

enum pmu_wkup_cfg2 {
	pmu_cluster_wkup_en = 0,
	pmu_gpio_wkup_en = 2,
	pmu_sdio_wkup_en = 3,
	pmu_sdmmc_wkup_en = 4,
	pmu_uart0_wkup_en = 5,
	pmu_timer_wkup_en = 6,
	pmu_usbdev_wkup_en = 7,
	pmu_sft_wkup_en = 8,
	pmu_timeout_wkup_en = 10,
};

enum pmu_powermode_core_lo {
	pmu_global_int_dis = 0,
	pmu_core_src_gt = 1,
	pmu_cpu0_pd = 3,
	pmu_clr_core = 5,
	pmu_scu_pd = 6,
	pmu_l2_idle = 8,
	pmu_l2_flush = 9,
	pmu_clr_bus2main = 10,
	pmu_clr_peri2msch = 11,
};

enum pmu_powermode_core_hi {
	pmu_apll_pd_en = 3,
	pmu_dpll_pd_en = 4,
	pmu_cpll_pd_en = 5,
	pmu_gpll_pd_en = 6,
	pmu_npll_pd_en = 7,
};

enum pmu_powermode_common_lo {
	pmu_mode_en = 0,
	pmu_ddr_pd_en = 1,
	pmu_wkup_rst = 3,
	pmu_pll_pd = 4,
	pmu_pmu_use_if = 6,
	pmu_alive_use_if = 7,
	pmu_osc_dis = 8,
	pmu_input_clamp = 9,
	pmu_sref_enter = 10,
	pmu_ddrc_gt = 11,
	pmu_ddrio_ret = 12,
	pmu_ddrio_ret_deq = 13,
	pmu_clr_pmu = 14,
	pmu_clr_peri_pmu = 15,
};

enum pmu_powermode_common_hi {
	pmu_clr_bus = 0,
	pmu_clr_mmc = 1,
	pmu_clr_msch = 2,
	pmu_clr_nandc = 3,
	pmu_clr_gmac = 4,
	pmu_clr_vo = 5,
	pmu_clr_vi = 6,
	pmu_clr_gpu = 7,
	pmu_clr_usb = 8,
	pmu_clr_vpu = 9,
	pmu_clr_crypto = 10,
	pmu_wakeup_begin_cfg = 11,
	pmu_peri_clk_src_gt = 12,
	pmu_bus_clk_src_gt = 13,
};

enum pmu_pd_id {
	PD_CPU0 = 0,
	PD_CPU1 = 1,
	PD_CPU2 = 2,
	PD_CPU3 = 3,
	PD_SCU = 4,
	PD_USB = 5,
	PD_DDR = 6,
	PD_SDCARD = 8,
	PD_CRYPTO = 9,
	PD_GMAC = 10,
	PD_MMC_NAND = 11,
	PD_VPU = 12,
	PD_VO = 13,
	PD_VI = 14,
	PD_GPU = 15,
	PD_END = 16,
};

enum pmu_bus_id {
	BUS_ID_BUS = 0,
	BUS_ID_BUS2MAIN = 1,
	BUS_ID_GPU = 2,
	BUS_ID_CORE = 3,
	BUS_ID_CRYPTO = 4,
	BUS_ID_MMC = 5,
	BUS_ID_GMAC = 6,
	BUS_ID_VO = 7,
	BUS_ID_VI = 8,
	BUS_ID_SDCARD = 9,
	BUS_ID_USB = 10,
	BUS_ID_MSCH = 11,
	BUS_ID_PERI = 12,
	BUS_ID_PMU = 13,
	BUS_ID_VPU = 14,
	BUS_ID_PERI2MSCH = 15,
};

enum pmu_pd_state {
	pmu_pd_on = 0,
	pmu_pd_off = 1
};

enum pmu_bus_state {
	bus_active = 0,
	bus_idle = 1,
};

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

/*****************************************************************************
 * pmu_sgrf
 *****************************************************************************/
#define PMUSGRF_SOC_CON(i)	((i) * 0x4)

/*****************************************************************************
 * pmu_grf
 *****************************************************************************/
#define GPIO0A_IOMUX		0x0
#define GPIO0B_IOMUX		0x4
#define GPIO0C_IOMUX		0x8
#define GPIO0A_PULL		0x10

#define GPIO0L_SMT		0x38
#define GPIO0H_SMT		0x3c

#define PMUGRF_SOC_CON(i)	(0x100 + (i) * 4)

#define PMUGRF_PVTM_CON0	0x180
#define PMUGRF_PVTM_CON1	0x184
#define PMUGRF_PVTM_ST0		0x190
#define PMUGRF_PVTM_ST1		0x194

#define PVTM_CALC_CNT		0x200

#define PMUGRF_OS_REG(n)	(0x200 + (n) * 4)

#define GPIO0A6_IOMUX_MSK	(0x3 << 12)
#define GPIO0A6_IOMUX_GPIO	(0x0 << 12)
#define GPIO0A6_IOMUX_RSTOUT	(0x1 << 12)
#define GPIO0A6_IOMUX_SHTDN	(0x2 << 12)

enum px30_pmugrf_pvtm_con0 {
	pgrf_pvtm_st = 0,
	pgrf_pvtm_en = 1,
	pgrf_pvtm_div = 2,
};

/*****************************************************************************
 * pmu_cru
 *****************************************************************************/
#define CRU_PMU_MODE			0x20
#define CRU_PMU_CLKSEL_CON		0x40
#define CRU_PMU_CLKSELS_CON(i)		(CRU_PMU_CLKSEL_CON + (i) * 4)
#define CRU_PMU_CLKSEL_CON_CNT		5
#define CRU_PMU_CLKGATE_CON		0x80
#define CRU_PMU_CLKGATES_CON(i)		(CRU_PMU_CLKGATE_CON + (i) * 4)
#define CRU_PMU_CLKGATE_CON_CNT		2
#define CRU_PMU_ATCS_CON		0xc0
#define CRU_PMU_ATCSS_CON(i)		(CRU_PMU_ATCS_CON + (i) * 4)
#define CRU_PMU_ATCS_CON_CNT		2

/*****************************************************************************
 * pmusgrf
 *****************************************************************************/
#define PMUSGRF_RSTOUT_EN (0x7 << 10)
#define PMUSGRF_RSTOUT_FST 10
#define PMUSGRF_RSTOUT_TSADC 11
#define PMUSGRF_RSTOUT_WDT 12

#define PMUGRF_SOC_CON2_US_WMSK  (0x1fff << 16)
#define PMUGRF_SOC_CON2_MAX_341US  0x1fff
#define PMUGRF_SOC_CON2_200US  0x12c0

#define PMUGRF_FAILSAFE_SHTDN_TSADC BIT(0)
#define PMUGRF_FAILSAFE_SHTDN_WDT BIT(1)

/*****************************************************************************
 * QOS
 *****************************************************************************/
#define CPU_AXI_QOS_ID_COREID		0x00
#define CPU_AXI_QOS_REVISIONID		0x04
#define CPU_AXI_QOS_PRIORITY		0x08
#define CPU_AXI_QOS_MODE		0x0c
#define CPU_AXI_QOS_BANDWIDTH		0x10
#define CPU_AXI_QOS_SATURATION		0x14
#define CPU_AXI_QOS_EXTCONTROL		0x18
#define CPU_AXI_QOS_NUM_REGS		0x07

#define CPU_AXI_CPU_QOS_BASE		0xff508000
#define CPU_AXI_GPU_QOS_BASE		0xff520000
#define CPU_AXI_ISP_128M_QOS_BASE	0xff548000
#define CPU_AXI_ISP_RD_QOS_BASE		0xff548080
#define CPU_AXI_ISP_WR_QOS_BASE		0xff548100
#define CPU_AXI_ISP_M1_QOS_BASE		0xff548180
#define CPU_AXI_VIP_QOS_BASE		0xff548200
#define CPU_AXI_RGA_RD_QOS_BASE		0xff550000
#define CPU_AXI_RGA_WR_QOS_BASE		0xff550080
#define CPU_AXI_VOP_M0_QOS_BASE		0xff550100
#define CPU_AXI_VOP_M1_QOS_BASE		0xff550180
#define CPU_AXI_VPU_QOS_BASE		0xff558000
#define CPU_AXI_VPU_R128_QOS_BASE	0xff558080
#define CPU_AXI_DCF_QOS_BASE		0xff500000
#define CPU_AXI_DMAC_QOS_BASE		0xff500080
#define CPU_AXI_CRYPTO_QOS_BASE		0xff510000
#define CPU_AXI_GMAC_QOS_BASE		0xff518000
#define CPU_AXI_EMMC_QOS_BASE		0xff538000
#define CPU_AXI_NAND_QOS_BASE		0xff538080
#define CPU_AXI_SDIO_QOS_BASE		0xff538100
#define CPU_AXI_SFC_QOS_BASE		0xff538180
#define CPU_AXI_SDMMC_QOS_BASE		0xff52c000
#define CPU_AXI_USB_HOST_QOS_BASE	0xff540000
#define CPU_AXI_USB_OTG_QOS_BASE	0xff540080

#define PX30_CPU_AXI_SAVE_QOS(array, base) do { \
	array[0] = mmio_read_32(base + CPU_AXI_QOS_ID_COREID); \
	array[1] = mmio_read_32(base + CPU_AXI_QOS_REVISIONID); \
	array[2] = mmio_read_32(base + CPU_AXI_QOS_PRIORITY); \
	array[3] = mmio_read_32(base + CPU_AXI_QOS_MODE); \
	array[4] = mmio_read_32(base + CPU_AXI_QOS_BANDWIDTH); \
	array[5] = mmio_read_32(base + CPU_AXI_QOS_SATURATION); \
	array[6] = mmio_read_32(base + CPU_AXI_QOS_EXTCONTROL); \
} while (0)

#define PX30_CPU_AXI_RESTORE_QOS(array, base) do { \
	mmio_write_32(base + CPU_AXI_QOS_ID_COREID, array[0]); \
	mmio_write_32(base + CPU_AXI_QOS_REVISIONID, array[1]); \
	mmio_write_32(base + CPU_AXI_QOS_PRIORITY, array[2]); \
	mmio_write_32(base + CPU_AXI_QOS_MODE, array[3]); \
	mmio_write_32(base + CPU_AXI_QOS_BANDWIDTH, array[4]); \
	mmio_write_32(base + CPU_AXI_QOS_SATURATION, array[5]); \
	mmio_write_32(base + CPU_AXI_QOS_EXTCONTROL, array[6]); \
} while (0)

#define SAVE_QOS(array, NAME) \
	PX30_CPU_AXI_SAVE_QOS(array, CPU_AXI_##NAME##_QOS_BASE)
#define RESTORE_QOS(array, NAME) \
	PX30_CPU_AXI_RESTORE_QOS(array, CPU_AXI_##NAME##_QOS_BASE)

#endif /* __PMU_H__ */
