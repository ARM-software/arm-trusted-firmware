/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <pmu.h>

#include <plat_pm_helpers.h>
#include <plat_private.h>
#include <pm_pd_regs.h>
#include <soc.h>

#define WMSK_VAL		0xffff0000

static struct reg_region qos_reg_rgns[] = {
	[QOS_ISP0_MWO] = REG_REGION(0x08, 0x18, 4, 0xfdf40500, 0),
	[QOS_ISP0_MRO] = REG_REGION(0x08, 0x18, 4, 0xfdf40400, 0),
	[QOS_ISP1_MWO] = REG_REGION(0x08, 0x18, 4, 0xfdf41000, 0),
	[QOS_ISP1_MRO] = REG_REGION(0x08, 0x18, 4, 0xfdf41100, 0),
	[QOS_VICAP_M0] = REG_REGION(0x08, 0x18, 4, 0xfdf40600, 0),
	[QOS_VICAP_M1] = REG_REGION(0x08, 0x18, 4, 0xfdf40800, 0),
	[QOS_FISHEYE0] = REG_REGION(0x08, 0x18, 4, 0xfdf40000, 0),
	[QOS_FISHEYE1] = REG_REGION(0x08, 0x18, 4, 0xfdf40200, 0),
	[QOS_VOP_M0] = REG_REGION(0x08, 0x18, 4, 0xfdf82000, 0),
	[QOS_VOP_M1] = REG_REGION(0x08, 0x18, 4, 0xfdf82200, 0),
	[QOS_RKVDEC0] = REG_REGION(0x08, 0x18, 4, 0xfdf62000, 0),
	[QOS_RKVDEC1] = REG_REGION(0x08, 0x18, 4, 0xfdf63000, 0),
	[QOS_AV1] = REG_REGION(0x08, 0x18, 4, 0xfdf64000, 0),
	[QOS_RKVENC0_M0RO] = REG_REGION(0x08, 0x18, 4, 0xfdf60000, 0),
	[QOS_RKVENC0_M1RO] = REG_REGION(0x08, 0x18, 4, 0xfdf60200, 0),
	[QOS_RKVENC0_M2WO] = REG_REGION(0x08, 0x18, 4, 0xfdf60400, 0),
	[QOS_RKVENC1_M0RO] = REG_REGION(0x08, 0x18, 4, 0xfdf61000, 0),
	[QOS_RKVENC1_M1RO] = REG_REGION(0x08, 0x18, 4, 0xfdf61200, 0),
	[QOS_RKVENC1_M2WO] = REG_REGION(0x08, 0x18, 4, 0xfdf61400, 0),
	[QOS_DSU_M0] = REG_REGION(0x08, 0x18, 4, 0xfe008000, 0),
	[QOS_DSU_M1] = REG_REGION(0x08, 0x18, 4, 0xfe008800, 0),
	[QOS_DSU_MP] = REG_REGION(0x08, 0x18, 4, 0xfdf34200, 0),
	[QOS_DEBUG] = REG_REGION(0x08, 0x18, 4, 0xfdf34400, 0),
	[QOS_GPU_M0] = REG_REGION(0x08, 0x18, 4, 0xfdf35000, 0),
	[QOS_GPU_M1] = REG_REGION(0x08, 0x18, 4, 0xfdf35200, 0),
	[QOS_GPU_M2] = REG_REGION(0x08, 0x18, 4, 0xfdf35400, 0),
	[QOS_GPU_M3] = REG_REGION(0x08, 0x18, 4, 0xfdf35600, 0),
	[QOS_NPU1] = REG_REGION(0x08, 0x18, 4, 0xfdf70000, 0),
	[QOS_NPU0_MRO] = REG_REGION(0x08, 0x18, 4, 0xfdf72200, 0),
	[QOS_NPU2] = REG_REGION(0x08, 0x18, 4, 0xfdf71000, 0),
	[QOS_NPU0_MWR] = REG_REGION(0x08, 0x18, 4, 0xfdf72000, 0),
	[QOS_MCU_NPU] = REG_REGION(0x08, 0x18, 4, 0xfdf72400, 0),
	[QOS_JPEG_DEC] = REG_REGION(0x08, 0x18, 4, 0xfdf66200, 0),
	[QOS_JPEG_ENC0] = REG_REGION(0x08, 0x18, 4, 0xfdf66400, 0),
	[QOS_JPEG_ENC1] = REG_REGION(0x08, 0x18, 4, 0xfdf66600, 0),
	[QOS_JPEG_ENC2] = REG_REGION(0x08, 0x18, 4, 0xfdf66800, 0),
	[QOS_JPEG_ENC3] = REG_REGION(0x08, 0x18, 4, 0xfdf66a00, 0),
	[QOS_RGA2_MRO] = REG_REGION(0x08, 0x18, 4, 0xfdf66c00, 0),
	[QOS_RGA2_MWO] = REG_REGION(0x08, 0x18, 4, 0xfdf66e00, 0),
	[QOS_RGA3_0] = REG_REGION(0x08, 0x18, 4, 0xfdf67000, 0),
	[QOS_RGA3_1] = REG_REGION(0x08, 0x18, 4, 0xfdf36000, 0),
	[QOS_VDPU] = REG_REGION(0x08, 0x18, 4, 0xfdf67200, 0),
	[QOS_IEP] = REG_REGION(0x08, 0x18, 4, 0xfdf66000, 0),
	[QOS_HDCP0] = REG_REGION(0x08, 0x18, 4, 0xfdf80000, 0),
	[QOS_HDCP1] = REG_REGION(0x08, 0x18, 4, 0xfdf81000, 0),
	[QOS_HDMIRX] = REG_REGION(0x08, 0x18, 4, 0xfdf81200, 0),
	[QOS_GIC600_M0] = REG_REGION(0x08, 0x18, 4, 0xfdf3a000, 0),
	[QOS_GIC600_M1] = REG_REGION(0x08, 0x18, 4, 0xfdf3a200, 0),
	[QOS_MMU600PCIE_TCU] = REG_REGION(0x08, 0x18, 4, 0xfdf3a400, 0),
	[QOS_MMU600PHP_TBU] = REG_REGION(0x08, 0x18, 4, 0xfdf3a600, 0),
	[QOS_MMU600PHP_TCU] = REG_REGION(0x08, 0x18, 4, 0xfdf3a800, 0),
	[QOS_USB3_0] = REG_REGION(0x08, 0x18, 4, 0xfdf3e200, 0),
	[QOS_USB3_1] = REG_REGION(0x08, 0x18, 4, 0xfdf3e000, 0),
	[QOS_USBHOST_0] = REG_REGION(0x08, 0x18, 4, 0xfdf3e400, 0),
	[QOS_USBHOST_1] = REG_REGION(0x08, 0x18, 4, 0xfdf3e600, 0),
	[QOS_EMMC] = REG_REGION(0x08, 0x18, 4, 0xfdf38200, 0),
	[QOS_FSPI] = REG_REGION(0x08, 0x18, 4, 0xfdf38000, 0),
	[QOS_SDIO] = REG_REGION(0x08, 0x18, 4, 0xfdf39000, 0),
	[QOS_DECOM] = REG_REGION(0x08, 0x18, 4, 0xfdf32000, 0),
	[QOS_DMAC0] = REG_REGION(0x08, 0x18, 4, 0xfdf32200, 0),
	[QOS_DMAC1] = REG_REGION(0x08, 0x18, 4, 0xfdf32400, 0),
	[QOS_DMAC2] = REG_REGION(0x08, 0x18, 4, 0xfdf32600, 0),
	[QOS_GIC600M] = REG_REGION(0x08, 0x18, 4, 0xfdf32800, 0),
	[QOS_DMA2DDR] = REG_REGION(0x08, 0x18, 4, 0xfdf52000, 0),
	[QOS_MCU_DDR] = REG_REGION(0x08, 0x18, 4, 0xfdf52200, 0),
	[QOS_VAD] = REG_REGION(0x08, 0x18, 4, 0xfdf3b200, 0),
	[QOS_MCU_PMU] = REG_REGION(0x08, 0x18, 4, 0xfdf3b000, 0),
	[QOS_CRYPTOS] = REG_REGION(0x08, 0x18, 4, 0xfdf3d200, 0),
	[QOS_CRYPTONS] = REG_REGION(0x08, 0x18, 4, 0xfdf3d000, 0),
	[QOS_DCF] = REG_REGION(0x08, 0x18, 4, 0xfdf3d400, 0),
	[QOS_SDMMC] = REG_REGION(0x08, 0x18, 4, 0xfdf3d800, 0),
};

static struct reg_region pd_crypto_reg_rgns[] = {
	/* SECURE CRU */
	REG_REGION(0x300, 0x30c, 4, SCRU_BASE, WMSK_VAL),
	REG_REGION(0x800, 0x80c, 4, SCRU_BASE, WMSK_VAL),
	REG_REGION(0xa00, 0xa0c, 4, SCRU_BASE, WMSK_VAL),
	REG_REGION(0xd00, 0xd20, 8, SCRU_BASE, 0),
	REG_REGION(0xd04, 0xd24, 8, SCRU_BASE, WMSK_VAL),

	/* S TIMER0 6 channel */
	REG_REGION(0x00, 0x04, 4, STIMER0_BASE + 0x00, 0),
	REG_REGION(0x10, 0x10, 4, STIMER0_BASE + 0x00, 0),
	REG_REGION(0x00, 0x04, 4, STIMER0_BASE + 0x20, 0),
	REG_REGION(0x10, 0x10, 4, STIMER0_BASE + 0x20, 0),
	REG_REGION(0x00, 0x04, 4, STIMER0_BASE + 0x40, 0),
	REG_REGION(0x10, 0x10, 4, STIMER0_BASE + 0x40, 0),
	REG_REGION(0x00, 0x04, 4, STIMER0_BASE + 0x60, 0),
	REG_REGION(0x10, 0x10, 4, STIMER0_BASE + 0x60, 0),
	REG_REGION(0x00, 0x04, 4, STIMER0_BASE + 0x80, 0),
	REG_REGION(0x10, 0x10, 4, STIMER0_BASE + 0x80, 0),
	REG_REGION(0x00, 0x04, 4, STIMER0_BASE + 0xa0, 0),
	REG_REGION(0x10, 0x10, 4, STIMER0_BASE + 0xa0, 0),

	/* S TIMER1 6 channel */
	REG_REGION(0x00, 0x04, 4, STIMER1_BASE + 0x00, 0),
	REG_REGION(0x10, 0x10, 4, STIMER1_BASE + 0x00, 0),
	REG_REGION(0x00, 0x04, 4, STIMER1_BASE + 0x20, 0),
	REG_REGION(0x10, 0x10, 4, STIMER1_BASE + 0x20, 0),
	REG_REGION(0x00, 0x04, 4, STIMER1_BASE + 0x40, 0),
	REG_REGION(0x10, 0x10, 4, STIMER1_BASE + 0x40, 0),
	REG_REGION(0x00, 0x04, 4, STIMER1_BASE + 0x60, 0),
	REG_REGION(0x10, 0x10, 4, STIMER1_BASE + 0x60, 0),
	REG_REGION(0x00, 0x04, 4, STIMER1_BASE + 0x80, 0),
	REG_REGION(0x10, 0x10, 4, STIMER1_BASE + 0x80, 0),
	REG_REGION(0x00, 0x04, 4, STIMER1_BASE + 0xa0, 0),
	REG_REGION(0x10, 0x10, 4, STIMER1_BASE + 0xa0, 0),

	/* wdt_s */
	REG_REGION(0x04, 0x04, 4, WDT_S_BASE, 0),
	REG_REGION(0x00, 0x00, 4, WDT_S_BASE, 0),
};

static struct reg_region pd_dsu_reg_rgns[] = {
	/* dsucru */
	REG_REGION(0x040, 0x054, 4, DSUCRU_BASE, WMSK_VAL),
	REG_REGION(0x300, 0x31c, 4, DSUCRU_BASE, WMSK_VAL),
	REG_REGION(0x800, 0x80c, 4, DSUCRU_BASE, WMSK_VAL),
	REG_REGION(0xa00, 0xa0c, 4, DSUCRU_BASE, WMSK_VAL),
	REG_REGION(0xd00, 0xd20, 8, DSUCRU_BASE, 0),
	REG_REGION(0xd04, 0xd24, 8, DSUCRU_BASE, WMSK_VAL),
	REG_REGION(0xf00, 0xf00, 4, DSUCRU_BASE, WMSK_VAL),
	REG_REGION(0xf10, 0xf1c, 4, DSUCRU_BASE, 0),

	/* bcore0cru */
	REG_REGION(0x000, 0x014, 4, BIGCORE0CRU_BASE, WMSK_VAL),
	REG_REGION(0x300, 0x304, 4, BIGCORE0CRU_BASE, WMSK_VAL),
	REG_REGION(0x800, 0x804, 4, BIGCORE0CRU_BASE, WMSK_VAL),
	REG_REGION(0xa00, 0xa04, 4, BIGCORE0CRU_BASE, WMSK_VAL),
	REG_REGION(0xcc0, 0xcc4, 4, BIGCORE0CRU_BASE, 0),
	REG_REGION(0xd00, 0xd00, 4, BIGCORE0CRU_BASE, 0),
	REG_REGION(0xd04, 0xd04, 4, BIGCORE0CRU_BASE, WMSK_VAL),

	/* bcore1cru */
	REG_REGION(0x020, 0x034, 4, BIGCORE1CRU_BASE, WMSK_VAL),
	REG_REGION(0x300, 0x304, 4, BIGCORE1CRU_BASE, WMSK_VAL),
	REG_REGION(0x800, 0x804, 4, BIGCORE1CRU_BASE, WMSK_VAL),
	REG_REGION(0xa00, 0xa04, 4, BIGCORE1CRU_BASE, WMSK_VAL),
	REG_REGION(0xcc0, 0xcc4, 4, BIGCORE1CRU_BASE, 0),
	REG_REGION(0xd00, 0xd00, 4, BIGCORE1CRU_BASE, 0),
	REG_REGION(0xd04, 0xd04, 4, BIGCORE1CRU_BASE, WMSK_VAL),

	/* dsugrf */
	REG_REGION(0x00, 0x18, 4, DSUGRF_BASE, WMSK_VAL),
	REG_REGION(0x20, 0x20, 4, DSUGRF_BASE, WMSK_VAL),
	REG_REGION(0x28, 0x30, 4, DSUGRF_BASE, WMSK_VAL),
	REG_REGION(0x38, 0x38, 4, DSUGRF_BASE, WMSK_VAL),

	/* lcore_grf */
	REG_REGION(0x20, 0x20, 4, LITCOREGRF_BASE, WMSK_VAL),
	REG_REGION(0x28, 0x30, 4, LITCOREGRF_BASE, WMSK_VAL),

	/* bcore0_grf */
	REG_REGION(0x20, 0x20, 4, BIGCORE0GRF_BASE, WMSK_VAL),
	REG_REGION(0x28, 0x30, 4, BIGCORE0GRF_BASE, WMSK_VAL),

	/* bcore1_grf */
	REG_REGION(0x20, 0x20, 4, BIGCORE1GRF_BASE, WMSK_VAL),
	REG_REGION(0x28, 0x28, 4, BIGCORE1GRF_BASE, WMSK_VAL),
};

static struct reg_region pd_php_reg_rgns[] = {
	/* php_grf */
	REG_REGION(0x000, 0x008, 4, PHPGRF_BASE, WMSK_VAL),
	REG_REGION(0x014, 0x024, 4, PHPGRF_BASE, WMSK_VAL),
	REG_REGION(0x028, 0x02c, 4, PHPGRF_BASE, 0),
	REG_REGION(0x030, 0x03c, 4, PHPGRF_BASE, WMSK_VAL),
	REG_REGION(0x05c, 0x060, 4, PHPGRF_BASE, WMSK_VAL),
	REG_REGION(0x064, 0x068, 4, PHPGRF_BASE, 0),
	REG_REGION(0x070, 0x070, 4, PHPGRF_BASE, WMSK_VAL),
	REG_REGION(0x074, 0x0d0, 4, PHPGRF_BASE, 0),
	REG_REGION(0x0d4, 0x0d4, 4, PHPGRF_BASE, WMSK_VAL),
	REG_REGION(0x0e0, 0x0e0, 4, PHPGRF_BASE, 0),
	REG_REGION(0x0e4, 0x0ec, 4, PHPGRF_BASE, WMSK_VAL),
	REG_REGION(0x100, 0x104, 4, PHPGRF_BASE, WMSK_VAL),
	REG_REGION(0x10c, 0x130, 4, PHPGRF_BASE, 0),
	REG_REGION(0x138, 0x138, 4, PHPGRF_BASE, WMSK_VAL),
	REG_REGION(0x144, 0x168, 4, PHPGRF_BASE, 0),
	REG_REGION(0x16c, 0x174, 4, PHPGRF_BASE, WMSK_VAL),

	/* php_cru */
	REG_REGION(0x200, 0x218, 4, PHP_CRU_BASE, WMSK_VAL),
	REG_REGION(0x800, 0x800, 4, PHP_CRU_BASE, WMSK_VAL),
	REG_REGION(0xa00, 0xa00, 4, PHP_CRU_BASE, WMSK_VAL),

	/* pcie3phy_grf_cmn_con0 */
	REG_REGION(0x00, 0x00, 4, PCIE3PHYGRF_BASE, WMSK_VAL),
};

void qos_save(void)
{
	uint32_t pmu_pd_st0 = mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST(0));

	if ((pmu_pd_st0 & BIT(PD_GPU)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_GPU_M0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_GPU_M1], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_GPU_M2], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_GPU_M3], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_NPU1)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_NPU1], 1);
	if ((pmu_pd_st0 & BIT(PD_NPU2)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_NPU2], 1);
	if ((pmu_pd_st0 & BIT(PD_NPUTOP)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_NPU0_MRO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_NPU0_MWR], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_MCU_NPU], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_RKVDEC1)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RKVDEC1], 1);
	if ((pmu_pd_st0 & BIT(PD_RKVDEC0)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RKVDEC0], 1);

	if ((pmu_pd_st0 & BIT(PD_VENC1)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RKVENC1_M0RO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RKVENC1_M1RO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RKVENC1_M2WO], 1);
	}
	if ((pmu_pd_st0 & BIT(PD_VENC0)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RKVENC0_M0RO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RKVENC0_M1RO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RKVENC0_M2WO], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_RGA30)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RGA3_0], 1);
	if ((pmu_pd_st0 & BIT(PD_AV1)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_AV1], 1);
	if ((pmu_pd_st0 & BIT(PD_VDPU)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_JPEG_DEC], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_JPEG_ENC0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_JPEG_ENC1], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_JPEG_ENC2], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_JPEG_ENC3], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RGA2_MRO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RGA2_MWO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_VDPU], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_IEP], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_VO0)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_HDCP0], 1);
	if ((pmu_pd_st0 & BIT(PD_VO1)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_HDCP1], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_HDMIRX], 1);
	}
	if ((pmu_pd_st0 & BIT(PD_VOP)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_VOP_M0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_VOP_M1], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_FEC)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_FISHEYE0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_FISHEYE1], 1);
	}
	if ((pmu_pd_st0 & BIT(PD_ISP1)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_ISP1_MWO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_ISP1_MRO], 1);
	}
	if ((pmu_pd_st0 & BIT(PD_VI)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_ISP0_MWO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_ISP0_MRO], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_VICAP_M0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_VICAP_M1], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_RGA31)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_RGA3_1], 1);

	if ((pmu_pd_st0 & BIT(PD_USB)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_USB3_0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_USB3_1], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_USBHOST_0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_USBHOST_1], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_PHP)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_GIC600_M0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_GIC600_M1], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_MMU600PCIE_TCU], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_MMU600PHP_TBU], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_MMU600PHP_TCU], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_SDIO)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_SDIO], 1);
	if ((pmu_pd_st0 & BIT(PD_NVM0)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_FSPI], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_EMMC], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_SDMMC)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_SDMMC], 1);

	if ((pmu_pd_st0 & BIT(PD_CRYPTO)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_CRYPTONS], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_CRYPTOS], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[QOS_DCF], 1);
	}

	/* PD_DSU */
	rockchip_reg_rgn_save(&qos_reg_rgns[QOS_DSU_M0], 1);
	rockchip_reg_rgn_save(&qos_reg_rgns[QOS_DSU_M1], 1);
	rockchip_reg_rgn_save(&qos_reg_rgns[QOS_DSU_MP], 1);
	rockchip_reg_rgn_save(&qos_reg_rgns[QOS_DEBUG], 1);
}

void qos_restore(void)
{
	uint32_t pmu_pd_st0 = mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST(0));

	if ((pmu_pd_st0 & BIT(PD_GPU)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_GPU_M0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_GPU_M1], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_GPU_M2], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_GPU_M3], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_NPU1)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_NPU1], 1);
	if ((pmu_pd_st0 & BIT(PD_NPU2)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_NPU2], 1);
	if ((pmu_pd_st0 & BIT(PD_NPUTOP)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_NPU0_MRO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_NPU0_MWR], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_MCU_NPU], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_RKVDEC1)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RKVDEC1], 1);
	if ((pmu_pd_st0 & BIT(PD_RKVDEC0)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RKVDEC0], 1);

	if ((pmu_pd_st0 & BIT(PD_VENC1)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RKVENC1_M0RO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RKVENC1_M1RO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RKVENC1_M2WO], 1);
	}
	if ((pmu_pd_st0 & BIT(PD_VENC0)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RKVENC0_M0RO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RKVENC0_M1RO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RKVENC0_M2WO], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_RGA30)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RGA3_0], 1);
	if ((pmu_pd_st0 & BIT(PD_AV1)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_AV1], 1);
	if ((pmu_pd_st0 & BIT(PD_VDPU)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_JPEG_DEC], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_JPEG_ENC0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_JPEG_ENC1], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_JPEG_ENC2], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_JPEG_ENC3], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RGA2_MRO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RGA2_MWO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_VDPU], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_IEP], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_VO0)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_HDCP0], 1);
	if ((pmu_pd_st0 & BIT(PD_VO1)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_HDCP1], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_HDMIRX], 1);
	}
	if ((pmu_pd_st0 & BIT(PD_VOP)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_VOP_M0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_VOP_M1], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_FEC)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_FISHEYE0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_FISHEYE1], 1);
	}
	if ((pmu_pd_st0 & BIT(PD_ISP1)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_ISP1_MWO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_ISP1_MRO], 1);
	}
	if ((pmu_pd_st0 & BIT(PD_VI)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_ISP0_MWO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_ISP0_MRO], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_VICAP_M0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_VICAP_M1], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_RGA31)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_RGA3_1], 1);

	if ((pmu_pd_st0 & BIT(PD_USB)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_USB3_0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_USB3_1], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_USBHOST_0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_USBHOST_1], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_PHP)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_GIC600_M0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_GIC600_M1], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_MMU600PCIE_TCU], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_MMU600PHP_TBU], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_MMU600PHP_TCU], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_SDIO)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_SDIO], 1);
	if ((pmu_pd_st0 & BIT(PD_NVM0)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_FSPI], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_EMMC], 1);
	}

	if ((pmu_pd_st0 & BIT(PD_SDMMC)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_SDMMC], 1);

	if ((pmu_pd_st0 & BIT(PD_CRYPTO)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_CRYPTONS], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_CRYPTOS], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_DCF], 1);
	}

	/* PD_DSU */
	rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_DSU_M0], 1);
	rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_DSU_M1], 1);
	rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_DSU_MP], 1);
	rockchip_reg_rgn_restore(&qos_reg_rgns[QOS_DEBUG], 1);
}

void pd_crypto_save(void)
{
	rockchip_reg_rgn_save(pd_crypto_reg_rgns, ARRAY_SIZE(pd_crypto_reg_rgns));
}

void pd_crypto_restore(void)
{
	rockchip_reg_rgn_restore(pd_crypto_reg_rgns, ARRAY_SIZE(pd_crypto_reg_rgns));
}

static uint32_t b0_cru_mode;
static uint32_t b1_cru_mode;
static uint32_t dsu_cru_mode;
static uint32_t bcore0_cru_sel_con2, bcore1_cru_sel_con2;

void pd_dsu_core_save(void)
{
	b0_cru_mode = mmio_read_32(BIGCORE0CRU_BASE + 0x280);
	b1_cru_mode = mmio_read_32(BIGCORE1CRU_BASE + 0x280);
	dsu_cru_mode = mmio_read_32(DSUCRU_BASE + 0x280);
	bcore0_cru_sel_con2 = mmio_read_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(2));
	bcore1_cru_sel_con2 = mmio_read_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(2));

	rockchip_reg_rgn_save(pd_dsu_reg_rgns, ARRAY_SIZE(pd_dsu_reg_rgns));
}

void pd_dsu_core_restore(void)
{
	/* switch bcore0/1 pclk root to 24M */
	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(2),
		      BITS_WITH_WMASK(2, 0x3, 0));
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(2),
		      BITS_WITH_WMASK(2, 0x3, 0));

	/* slow mode */
	mmio_write_32(BIGCORE0CRU_BASE + 0x280, 0x00030000);
	mmio_write_32(BIGCORE1CRU_BASE + 0x280, 0x00030000);
	mmio_write_32(DSUCRU_BASE + 0x280, 0x00030000);

	rockchip_reg_rgn_restore(pd_dsu_reg_rgns, ARRAY_SIZE(pd_dsu_reg_rgns));

	/* trigger dsu/lcore/bcore mem_cfg */
	mmio_write_32(DSUGRF_BASE + 0x18, BITS_WITH_WMASK(1, 0x1, 14));
	mmio_write_32(LITCOREGRF_BASE + 0x30, BITS_WITH_WMASK(1, 0x1, 5));
	mmio_write_32(BIGCORE0GRF_BASE + 0x30, BITS_WITH_WMASK(1, 0x1, 5));
	mmio_write_32(BIGCORE1GRF_BASE + 0x30, BITS_WITH_WMASK(1, 0x1, 5));
	udelay(1);
	mmio_write_32(DSUGRF_BASE + 0x18, BITS_WITH_WMASK(0, 0x1, 14));
	mmio_write_32(LITCOREGRF_BASE + 0x30, BITS_WITH_WMASK(0, 0x1, 5));
	mmio_write_32(BIGCORE0GRF_BASE + 0x30, BITS_WITH_WMASK(0, 0x1, 5));
	mmio_write_32(BIGCORE1GRF_BASE + 0x30, BITS_WITH_WMASK(0, 0x1, 5));

	/* wait lock */
	pm_pll_wait_lock(BIGCORE0CRU_BASE + 0x00);
	pm_pll_wait_lock(BIGCORE1CRU_BASE + 0x20);
	pm_pll_wait_lock(DSUCRU_BASE + 0x40);

	/* restore mode */
	mmio_write_32(BIGCORE0CRU_BASE + 0x280, WITH_16BITS_WMSK(b0_cru_mode));
	mmio_write_32(BIGCORE1CRU_BASE + 0x280, WITH_16BITS_WMSK(b1_cru_mode));
	mmio_write_32(DSUCRU_BASE + 0x280, WITH_16BITS_WMSK(dsu_cru_mode));

	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(2),
		      WITH_16BITS_WMSK(bcore0_cru_sel_con2));
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(2),
		      WITH_16BITS_WMSK(bcore1_cru_sel_con2));
}

static uint32_t php_ppll_con0;

void pd_php_save(void)
{
	php_ppll_con0 = mmio_read_32(PHP_CRU_BASE + 0x200);

	/* php_ppll bypass */
	mmio_write_32(PHP_CRU_BASE + 0x200, BITS_WITH_WMASK(1u, 1u, 15));
	dsb();
	isb();
	rockchip_reg_rgn_save(pd_php_reg_rgns, ARRAY_SIZE(pd_php_reg_rgns));
}

void pd_php_restore(void)
{
	rockchip_reg_rgn_restore(pd_php_reg_rgns, ARRAY_SIZE(pd_php_reg_rgns));

	pm_pll_wait_lock(PHP_CRU_BASE + 0x200);

	/* restore php_ppll bypass */
	mmio_write_32(PHP_CRU_BASE + 0x200, WITH_16BITS_WMSK(php_ppll_con0));
}

void pm_reg_rgns_init(void)
{
	rockchip_alloc_region_mem(qos_reg_rgns, ARRAY_SIZE(qos_reg_rgns));
	rockchip_alloc_region_mem(pd_crypto_reg_rgns, ARRAY_SIZE(pd_crypto_reg_rgns));
	rockchip_alloc_region_mem(pd_dsu_reg_rgns, ARRAY_SIZE(pd_dsu_reg_rgns));
	rockchip_alloc_region_mem(pd_php_reg_rgns, ARRAY_SIZE(pd_php_reg_rgns));
}
