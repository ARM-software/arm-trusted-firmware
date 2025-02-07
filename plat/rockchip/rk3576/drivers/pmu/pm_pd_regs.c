// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
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
#include <rk3576_clk.h>
#include <soc.h>

#define WMSK_VAL		0xffff0000

static struct reg_region qos_reg_rgns[] = {
	[qos_decom] = REG_REGION(0x08, 0x18, 4, 0x27f00000, 0),
	[qos_dmac0] = REG_REGION(0x08, 0x18, 4, 0x27f00080, 0),
	[qos_dmac1] = REG_REGION(0x08, 0x18, 4, 0x27f00100, 0),
	[qos_dmac2] = REG_REGION(0x08, 0x18, 4, 0x27f00180, 0),
	[qos_bus_mcu] = REG_REGION(0x08, 0x18, 4, 0x27f00200, 0),
	[qos_can0] = REG_REGION(0x08, 0x18, 4, 0x27f00280, 0),
	[qos_can1] = REG_REGION(0x08, 0x18, 4, 0x27f00300, 0),
	[qos_cci_m0] = REG_REGION(0x08, 0x18, 4, 0x27f01000, 0),
	[qos_cci_m1] = REG_REGION(0x08, 0x18, 4, 0x27f18880, 0),
	[qos_cci_m2] = REG_REGION(0x08, 0x18, 4, 0x27f18900, 0),
	[qos_dap_lite] = REG_REGION(0x08, 0x18, 4, 0x27f01080, 0),
	[qos_hdcp1] = REG_REGION(0x08, 0x18, 4, 0x27f02000, 0),
	[qos_ddr_mcu] = REG_REGION(0x08, 0x18, 4, 0x27f03000, 0),
	[qos_fspi1] = REG_REGION(0x08, 0x18, 4, 0x27f04000, 0),
	[qos_gmac0] = REG_REGION(0x08, 0x18, 4, 0x27f04080, 0),
	[qos_gmac1] = REG_REGION(0x08, 0x18, 4, 0x27f04100, 0),
	[qos_sdio] = REG_REGION(0x08, 0x18, 4, 0x27f04180, 0),
	[qos_sdmmc] = REG_REGION(0x08, 0x18, 4, 0x27f04200, 0),
	[qos_flexbus] = REG_REGION(0x08, 0x18, 4, 0x27f04280, 0),
	[qos_gpu] = REG_REGION(0x08, 0x18, 4, 0x27f05000, 0),
	[qos_vepu1] = REG_REGION(0x08, 0x18, 4, 0x27f06000, 0),
	[qos_npu_mcu] = REG_REGION(0x08, 0x18, 4, 0x27f08000, 0),
	[qos_npu_nsp0] = REG_REGION(0x08, 0x18, 4, 0x27f08080, 0),
	[qos_npu_nsp1] = REG_REGION(0x08, 0x18, 4, 0x27f08100, 0),
	[qos_npu_m0] = REG_REGION(0x08, 0x18, 4, 0x27f20000, 0),
	[qos_npu_m1] = REG_REGION(0x08, 0x18, 4, 0x27f21000, 0),
	[qos_npu_m0ro] = REG_REGION(0x08, 0x18, 4, 0x27f22080, 0),
	[qos_npu_m1ro] = REG_REGION(0x08, 0x18, 4, 0x27f22100, 0),
	[qos_emmc] = REG_REGION(0x08, 0x18, 4, 0x27f09000, 0),
	[qos_fspi0] = REG_REGION(0x08, 0x18, 4, 0x27f09080, 0),
	[qos_mmu0] = REG_REGION(0x08, 0x18, 4, 0x27f0a000, 0),
	[qos_mmu1] = REG_REGION(0x08, 0x18, 4, 0x27f0a080, 0),
	[qos_pmu_mcu] = REG_REGION(0x08, 0x18, 4, 0x27f0b000, 0),
	[qos_rkvdec] = REG_REGION(0x08, 0x18, 4, 0x27f0c000, 0),
	[qos_crypto] = REG_REGION(0x08, 0x18, 4, 0x27f0d000, 0),
	[qos_mmu2] = REG_REGION(0x08, 0x18, 4, 0x27f0e000, 0),
	[qos_ufshc] = REG_REGION(0x08, 0x18, 4, 0x27f0e080, 0),
	[qos_vepu0] = REG_REGION(0x08, 0x18, 4, 0x27f0f000, 0),
	[qos_isp_mro] = REG_REGION(0x08, 0x18, 4, 0x27f10000, 0),
	[qos_isp_mwo] = REG_REGION(0x08, 0x18, 4, 0x27f10080, 0),
	[qos_vicap_m0] = REG_REGION(0x08, 0x18, 4, 0x27f10100, 0),
	[qos_vpss_mro] = REG_REGION(0x08, 0x18, 4, 0x27f10180, 0),
	[qos_vpss_mwo] = REG_REGION(0x08, 0x18, 4, 0x27f10200, 0),
	[qos_hdcp0] = REG_REGION(0x08, 0x18, 4, 0x27f11000, 0),
	[qos_vop_m0] = REG_REGION(0x08, 0x18, 4, 0x27f12800, 0),
	[qos_vop_m1ro] = REG_REGION(0x08, 0x18, 4, 0x27f12880, 0),
	[qos_ebc] = REG_REGION(0x08, 0x18, 4, 0x27f13000, 0),
	[qos_rga0] = REG_REGION(0x08, 0x18, 4, 0x27f13080, 0),
	[qos_rga1] = REG_REGION(0x08, 0x18, 4, 0x27f13100, 0),
	[qos_jpeg] = REG_REGION(0x08, 0x18, 4, 0x27f13180, 0),
	[qos_vdpp] = REG_REGION(0x08, 0x18, 4, 0x27f13200, 0),
	[qos_dma2ddr] = REG_REGION(0x08, 0x18, 4, 0x27f15880, 0),
};

static struct reg_region pd_bcore_reg_rgns[] = {
	/* bcore cru */
	/* REG_REGION(0x280, 0x280, 4, BIGCORE0CRU_BASE, WMSK_VAL), */
	REG_REGION(0x300, 0x30c, 4, BIGCORE_CRU_BASE, WMSK_VAL),
	REG_REGION(0x800, 0x804, 4, BIGCORE_CRU_BASE, WMSK_VAL),
	REG_REGION(0xa00, 0xa0c, 4, BIGCORE_CRU_BASE, WMSK_VAL),
	REG_REGION(0xcc0, 0xcc0, 4, BIGCORE_CRU_BASE, 0),
	REG_REGION(0xf28, 0xf28, 8, BIGCORE_CRU_BASE, 0),
	REG_REGION(0xf2c, 0xf2c, 8, BIGCORE_CRU_BASE, WMSK_VAL),

	/* bcore_grf */
	REG_REGION(0x34, 0x3c, 4, BIGCORE_GRF_BASE, WMSK_VAL),
	REG_REGION(0x44, 0x44, 4, BIGCORE_GRF_BASE, WMSK_VAL),
};

static struct reg_region pd_core_reg_rgns[] = {
	/* cci cru */
	REG_REGION(0x310, 0x310, 4, CCI_CRU_BASE, WMSK_VAL),
	REG_REGION(0x804, 0x808, 4, CCI_CRU_BASE, WMSK_VAL),
	REG_REGION(0xa04, 0xa08, 4, CCI_CRU_BASE, WMSK_VAL),
	REG_REGION(0xc50, 0xc58, 4, CCI_CRU_BASE, WMSK_VAL),
	REG_REGION(0xd00, 0xd00, 8, CCI_CRU_BASE, 0),
	REG_REGION(0xd04, 0xd04, 8, CCI_CRU_BASE, WMSK_VAL),
	/* Restore lpll registers after clksel_* registers. Because lpll
	 * may be turned off during restoring, which cause cci_cru to lost clock.
	 */
	REG_REGION(0x040, 0x044, 4, CCI_CRU_BASE, WMSK_VAL),
	REG_REGION(0x048, 0x048, 4, CCI_CRU_BASE, 0),
	REG_REGION(0x04c, 0x058, 4, CCI_CRU_BASE, WMSK_VAL),

	/* lcore cru */
	/* REG_REGION(0x280, 0x280, 4, BIGCORE1CRU_BASE, WMSK_VAL), */
	REG_REGION(0x300, 0x30c, 4, LITTLE_CRU_BASE, WMSK_VAL),
	REG_REGION(0x800, 0x804, 4, LITTLE_CRU_BASE, WMSK_VAL),
	REG_REGION(0xa00, 0xa0c, 4, LITTLE_CRU_BASE, WMSK_VAL),
	REG_REGION(0xcc0, 0xcc0, 4, LITTLE_CRU_BASE, 0),
	REG_REGION(0xf38, 0xf38, 8, LITTLE_CRU_BASE, 0),
	REG_REGION(0xf3c, 0xf3c, 8, LITTLE_CRU_BASE, WMSK_VAL),

	/* bcore cru */
	/* REG_REGION(0x280, 0x280, 4, BIGCORE0CRU_BASE, WMSK_VAL), */
	REG_REGION(0x300, 0x30c, 4, BIGCORE_CRU_BASE, WMSK_VAL),
	REG_REGION(0x800, 0x804, 4, BIGCORE_CRU_BASE, WMSK_VAL),
	REG_REGION(0xa00, 0xa0c, 4, BIGCORE_CRU_BASE, WMSK_VAL),
	REG_REGION(0xcc0, 0xcc0, 4, BIGCORE_CRU_BASE, 0),
	REG_REGION(0xf28, 0xf28, 8, BIGCORE_CRU_BASE, 0),
	REG_REGION(0xf2c, 0xf2c, 8, BIGCORE_CRU_BASE, WMSK_VAL),

	/* cci grf */
	REG_REGION(0x00, 0x10, 4, CCI_GRF_BASE, WMSK_VAL),
	REG_REGION(0x54, 0x54, 4, CCI_GRF_BASE, WMSK_VAL),

	/* lcore_grf */
	REG_REGION(0x34, 0x3c, 4, LITCORE_GRF_BASE, WMSK_VAL),
	REG_REGION(0x44, 0x44, 4, LITCORE_GRF_BASE, WMSK_VAL),

	/* bcore_grf */
	REG_REGION(0x34, 0x3c, 4, BIGCORE_GRF_BASE, WMSK_VAL),
	REG_REGION(0x44, 0x44, 4, BIGCORE_GRF_BASE, WMSK_VAL),
};

static struct reg_region pd_php_reg_rgns[] = {
	/* php_grf */
	REG_REGION(0x004, 0x00c, 4, PHP_GRF_BASE, WMSK_VAL),
	REG_REGION(0x010, 0x018, 4, PHP_GRF_BASE, 0),
	REG_REGION(0x01c, 0x020, 4, PHP_GRF_BASE, WMSK_VAL),
	REG_REGION(0x048, 0x048, 4, PHP_GRF_BASE, 0),
};

static struct reg_region pd_usb2phy_reg_rgns[] = {
	/* usb */
	REG_REGION(0x00, 0x14, 4, USB2PHY0_GRF_BASE, WMSK_VAL),
	REG_REGION(0x40, 0x40, 4, USB2PHY0_GRF_BASE, WMSK_VAL),
	REG_REGION(0x44, 0x50, 4, USB2PHY0_GRF_BASE, 0),
	REG_REGION(0x00, 0x14, 4, USB2PHY1_GRF_BASE, WMSK_VAL),
	REG_REGION(0x08, 0x08, 4, USBDPPHY_GRF_BASE, WMSK_VAL),
};

#define PLL_LOCKED_TIMEOUT 600000U

static void pm_pll_wait_lock(uint32_t pll_base)
{
	int delay = PLL_LOCKED_TIMEOUT;

	if ((mmio_read_32(pll_base + CRU_PLL_CON(1)) & CRU_PLLCON1_PWRDOWN) != 0)
		return;

	while (delay-- >= 0) {
		if ((mmio_read_32(pll_base + CRU_PLL_CON(6)) & CRU_PLLCON6_LOCK_STATUS) != 0)
			break;
		udelay(1);
	}

	if (delay <= 0)
		ERROR("Can't wait pll(0x%x) lock\n", pll_base);
}

void qos_save(void)
{
	uint32_t pmu_pd_st = mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST);

	if ((pmu_pd_st & BIT(pmu_pd_nvm)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_emmc], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_fspi0], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_sd_gmac)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_fspi1], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_gmac0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_gmac1], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_sdio], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_sdmmc], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_flexbus], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_php)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_mmu0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_mmu1], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_vop)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_vop_m0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_vop_m1ro], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_vo1)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_hdcp1], 1);

	if ((pmu_pd_st & BIT(pmu_pd_vo0)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_hdcp0], 1);

	if ((pmu_pd_st & BIT(pmu_pd_usb)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_mmu2], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_ufshc], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_vi)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_isp_mro], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_isp_mwo], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_vicap_m0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_vpss_mro], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_vpss_mwo], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_vepu0)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_vepu0], 1);

	if ((pmu_pd_st & BIT(pmu_pd_vepu1)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_vepu1], 1);

	if ((pmu_pd_st & BIT(pmu_pd_vdec)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_rkvdec], 1);

	if ((pmu_pd_st & BIT(pmu_pd_vpu)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_ebc], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_rga0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_rga1], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_jpeg], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_vdpp], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_nputop)) == 0) {
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_npu_mcu], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_npu_nsp0], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_npu_nsp1], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_npu_m0ro], 1);
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_npu_m1ro], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_npu0)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_npu_m0], 1);

	if ((pmu_pd_st & BIT(pmu_pd_npu1)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_npu_m1], 1);

	if ((pmu_pd_st & BIT(pmu_pd_gpu)) == 0)
		rockchip_reg_rgn_save(&qos_reg_rgns[qos_gpu], 1);
}

void qos_restore(void)
{
	uint32_t pmu_pd_st = mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST);

	if ((pmu_pd_st & BIT(pmu_pd_nvm)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_emmc], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_fspi0], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_sd_gmac)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_fspi1], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_gmac0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_gmac1], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_sdio], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_sdmmc], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_flexbus], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_php)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_mmu0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_mmu1], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_vop)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_vop_m0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_vop_m1ro], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_vo1)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_hdcp1], 1);

	if ((pmu_pd_st & BIT(pmu_pd_vo0)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_hdcp0], 1);

	if ((pmu_pd_st & BIT(pmu_pd_usb)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_mmu2], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_ufshc], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_vi)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_isp_mro], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_isp_mwo], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_vicap_m0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_vpss_mro], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_vpss_mwo], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_vepu0)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_vepu0], 1);

	if ((pmu_pd_st & BIT(pmu_pd_vepu1)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_vepu1], 1);

	if ((pmu_pd_st & BIT(pmu_pd_vdec)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_rkvdec], 1);

	if ((pmu_pd_st & BIT(pmu_pd_vpu)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_ebc], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_rga0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_rga1], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_jpeg], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_vdpp], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_nputop)) == 0) {
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_npu_mcu], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_npu_nsp0], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_npu_nsp1], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_npu_m0ro], 1);
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_npu_m1ro], 1);
	}

	if ((pmu_pd_st & BIT(pmu_pd_npu0)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_npu_m0], 1);

	if ((pmu_pd_st & BIT(pmu_pd_npu1)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_npu_m1], 1);

	if ((pmu_pd_st & BIT(pmu_pd_gpu)) == 0)
		rockchip_reg_rgn_restore(&qos_reg_rgns[qos_gpu], 1);
}

void pd_usb2phy_save(void)
{
	rockchip_reg_rgn_save(pd_usb2phy_reg_rgns, ARRAY_SIZE(pd_usb2phy_reg_rgns));
}

void pd_usb2phy_restore(void)
{
	rockchip_reg_rgn_restore(pd_usb2phy_reg_rgns, ARRAY_SIZE(pd_usb2phy_reg_rgns));
}

static uint32_t b_cru_mode, l_cru_mode;
static uint32_t bcore_need_restore;

void pd_bcore_save(void)
{
	pvtplls_cpub_suspend();

	b_cru_mode = mmio_read_32(BIGCORE_CRU_BASE + 0x280);
	rockchip_reg_rgn_save(pd_bcore_reg_rgns, ARRAY_SIZE(pd_bcore_reg_rgns));

	bcore_need_restore = 1;
}

void pd_bcore_restore(void)
{
	if (bcore_need_restore == 0)
		return;

	/* slow mode */
	mmio_write_32(BIGCORE_CRU_BASE + 0x280, 0x00030000);

	rockchip_reg_rgn_restore(pd_bcore_reg_rgns, ARRAY_SIZE(pd_bcore_reg_rgns));

	/* trigger lcore/bcore mem_cfg */
	mmio_write_32(BIGCORE_GRF_BASE + 0x38, BITS_WITH_WMASK(1, 0x1, 1));
	udelay(1);
	mmio_write_32(BIGCORE_GRF_BASE + 0x38, BITS_WITH_WMASK(0, 0x1, 1));

	/* restore mode */
	mmio_write_32(BIGCORE_CRU_BASE + 0x280, WITH_16BITS_WMSK(b_cru_mode));

	pvtplls_cpub_resume();

	bcore_need_restore = 0;
}

void pd_core_save(void)
{
	pvtplls_suspend();

	b_cru_mode = mmio_read_32(BIGCORE_CRU_BASE + 0x280);
	l_cru_mode = mmio_read_32(LITTLE_CRU_BASE + 0x280);

	rockchip_reg_rgn_save(&qos_reg_rgns[qos_cci_m0], 1);
	rockchip_reg_rgn_save(&qos_reg_rgns[qos_cci_m1], 1);
	rockchip_reg_rgn_save(&qos_reg_rgns[qos_cci_m2], 1);
	rockchip_reg_rgn_save(&qos_reg_rgns[qos_dap_lite], 1);

	rockchip_reg_rgn_save(pd_core_reg_rgns, ARRAY_SIZE(pd_core_reg_rgns));
}

void pd_core_restore(void)
{
	/* slow mode */
	mmio_write_32(BIGCORE_CRU_BASE + 0x280, 0x00030000);
	mmio_write_32(LITTLE_CRU_BASE + 0x280, 0x00030000);

	rockchip_reg_rgn_restore(pd_core_reg_rgns, ARRAY_SIZE(pd_core_reg_rgns));

	/* trigger lcore/bcore mem_cfg */
	mmio_write_32(LITCORE_GRF_BASE + 0x38, BITS_WITH_WMASK(1, 0x1, 1));
	mmio_write_32(BIGCORE_GRF_BASE + 0x38, BITS_WITH_WMASK(1, 0x1, 1));
	udelay(1);
	mmio_write_32(LITCORE_GRF_BASE + 0x38, BITS_WITH_WMASK(0, 0x1, 1));
	mmio_write_32(BIGCORE_GRF_BASE + 0x38, BITS_WITH_WMASK(0, 0x1, 1));

	/* wait lock */
	pm_pll_wait_lock(CCI_CRU_BASE + 0x40);

	/* restore mode */
	mmio_write_32(BIGCORE_CRU_BASE + 0x280, WITH_16BITS_WMSK(b_cru_mode));
	mmio_write_32(LITTLE_CRU_BASE + 0x280, WITH_16BITS_WMSK(l_cru_mode));

	rockchip_reg_rgn_restore(&qos_reg_rgns[qos_cci_m0], 1);
	rockchip_reg_rgn_restore(&qos_reg_rgns[qos_cci_m1], 1);
	rockchip_reg_rgn_restore(&qos_reg_rgns[qos_cci_m2], 1);
	rockchip_reg_rgn_restore(&qos_reg_rgns[qos_dap_lite], 1);

	pvtplls_resume();
}

void pd_php_save(void)
{
	rockchip_reg_rgn_save(pd_php_reg_rgns, ARRAY_SIZE(pd_php_reg_rgns));
}

void pd_php_restore(void)
{
	rockchip_reg_rgn_restore(pd_php_reg_rgns, ARRAY_SIZE(pd_php_reg_rgns));
}

void pm_reg_rgns_init(void)
{
	rockchip_alloc_region_mem(qos_reg_rgns, ARRAY_SIZE(qos_reg_rgns));
	rockchip_alloc_region_mem(pd_bcore_reg_rgns, ARRAY_SIZE(pd_bcore_reg_rgns));
	rockchip_alloc_region_mem(pd_core_reg_rgns, ARRAY_SIZE(pd_core_reg_rgns));
	rockchip_alloc_region_mem(pd_php_reg_rgns, ARRAY_SIZE(pd_php_reg_rgns));
	rockchip_alloc_region_mem(pd_usb2phy_reg_rgns, ARRAY_SIZE(pd_usb2phy_reg_rgns));
}
