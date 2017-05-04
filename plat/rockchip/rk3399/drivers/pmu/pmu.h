/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PMU_H__
#define __PMU_H__

#include <pmu_bits.h>
#include <pmu_regs.h>
#include <soc.h>

/* Allocate sp reginon in pmusram */
#define PSRAM_SP_SIZE		0x80
#define PSRAM_SP_BOTTOM		(PSRAM_SP_TOP - PSRAM_SP_SIZE)

/*****************************************************************************
 * Common define for per soc pmu.h
 *****************************************************************************/
/* The ways of cores power domain contorlling */
enum cores_pm_ctr_mode {
	core_pwr_pd = 0,
	core_pwr_wfi = 1,
	core_pwr_wfi_int = 2
};

/*****************************************************************************
 * pmu con,reg
 *****************************************************************************/
#define PMU_WKUP_CFG(n)	((n) * 4)

#define PMU_CORE_PM_CON(cpu)		(0xc0 + (cpu * 4))

/* the shift of bits for cores status */
enum pmu_core_pwrst_shift {
	clstl_cpu_wfe = 2,
	clstl_cpu_wfi = 6,
	clstb_cpu_wfe = 12,
	clstb_cpu_wfi = 16
};

#define CKECK_WFE_MSK		0x1
#define CKECK_WFI_MSK		0x10
#define CKECK_WFEI_MSK		0x11

/* Specific features required  */
#define AP_PWROFF		0x0a

#define GPIO0A0_SMT_ENABLE	BITS_WITH_WMASK(1, 3, 0)
#define GPIO1A6_IOMUX		BITS_WITH_WMASK(0, 3, 12)

#define TSADC_INT_PIN		38
#define CORES_PM_DISABLE	0x0

#define PD_CTR_LOOP		500
#define CHK_CPU_LOOP		500
#define MAX_WAIT_COUNT		1000

#define	GRF_SOC_CON4		0x0e210

#define PMUGRF_GPIO0A_SMT	0x0120
#define PMUGRF_SOC_CON0		0x0180

#define CCI_FORCE_WAKEUP	WMSK_BIT(8)
#define EXTERNAL_32K		WMSK_BIT(0)

#define PLL_PD_HW		0xff
#define IOMUX_CLK_32K		0x00030002
#define NOC_AUTO_ENABLE		0x3fffffff

#define SAVE_QOS(array, NAME) \
	RK3399_CPU_AXI_SAVE_QOS(array, CPU_AXI_##NAME##_QOS_BASE)
#define RESTORE_QOS(array, NAME) \
	RK3399_CPU_AXI_RESTORE_QOS(array, CPU_AXI_##NAME##_QOS_BASE)

#define RK3399_CPU_AXI_SAVE_QOS(array, base) do { \
	array[0] = mmio_read_32(base + CPU_AXI_QOS_ID_COREID); \
	array[1] = mmio_read_32(base + CPU_AXI_QOS_REVISIONID); \
	array[2] = mmio_read_32(base + CPU_AXI_QOS_PRIORITY); \
	array[3] = mmio_read_32(base + CPU_AXI_QOS_MODE); \
	array[4] = mmio_read_32(base + CPU_AXI_QOS_BANDWIDTH); \
	array[5] = mmio_read_32(base + CPU_AXI_QOS_SATURATION); \
	array[6] = mmio_read_32(base + CPU_AXI_QOS_EXTCONTROL); \
} while (0)

#define RK3399_CPU_AXI_RESTORE_QOS(array, base) do { \
	mmio_write_32(base + CPU_AXI_QOS_ID_COREID, array[0]); \
	mmio_write_32(base + CPU_AXI_QOS_REVISIONID, array[1]); \
	mmio_write_32(base + CPU_AXI_QOS_PRIORITY, array[2]); \
	mmio_write_32(base + CPU_AXI_QOS_MODE, array[3]); \
	mmio_write_32(base + CPU_AXI_QOS_BANDWIDTH, array[4]); \
	mmio_write_32(base + CPU_AXI_QOS_SATURATION, array[5]); \
	mmio_write_32(base + CPU_AXI_QOS_EXTCONTROL, array[6]); \
} while (0)

struct pmu_slpdata_s {
	uint32_t cci_m0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t cci_m1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t dmac0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t dmac1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t dcf_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t crypto0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t crypto1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t pmu_cm0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t peri_cm1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t gic_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t sdmmc_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t gmac_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t emmc_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t usb_otg0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t usb_otg1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t usb_host0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t usb_host1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t gpu_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t video_m0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t video_m1_r_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t video_m1_w_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t rga_r_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t rga_w_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t vop_big_r[CPU_AXI_QOS_NUM_REGS];
	uint32_t vop_big_w[CPU_AXI_QOS_NUM_REGS];
	uint32_t vop_little[CPU_AXI_QOS_NUM_REGS];
	uint32_t iep_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t isp1_m0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t isp1_m1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t isp0_m0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t isp0_m1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t hdcp_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t perihp_nsp_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t perilp_nsp_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t perilpslv_nsp_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t sdio_qos[CPU_AXI_QOS_NUM_REGS];
};

extern uint32_t clst_warmboot_data[PLATFORM_CLUSTER_COUNT];

extern void sram_func_set_ddrctl_pll(uint32_t pll_src);

#endif /* __PMU_H__ */
