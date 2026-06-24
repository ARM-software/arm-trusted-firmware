// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
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

#include <plat_pm_helpers.h>
#include <plat_private.h>
#include <platform_def.h>
#include <pm_pd_regs.h>
#include <pmu.h>
#include <soc.h>

#define WMSK_VAL		0xffff0000

static struct reg_region pd_aiisp_reg_rgns[] = {
	/* qos */
	REG_REGION(0x08, 0x18, 4, 0x203b0000, 0)
};

static struct reg_region pd_vdo_reg_rgns[] = {
	/* vdo_cru */
	REG_REGION(0x300, 0x300, 4, VDOCRU_BASE, WMSK_VAL),

	/* vdo_grf */
	REG_REGION(0x000, 0x014, 4, VDOGRF_BASE, WMSK_VAL),

	/* qos */
	REG_REGION(0x08, 0x18, 4, 0x20380000, 0),
	REG_REGION(0x08, 0x18, 4, 0x20380100, 0),
	REG_REGION(0x08, 0x18, 4, 0x20380200, 0),
	REG_REGION(0x08, 0x18, 4, 0x20380300, 0),
	REG_REGION(0x08, 0x18, 4, 0x20380400, 0),

	/* shaping */
	REG_REGION(0x08, 0x08, 4, 0x20380080, 0),
	REG_REGION(0x08, 0x08, 4, 0x20380180, 0),
	REG_REGION(0x08, 0x08, 4, 0x20380280, 0),
	REG_REGION(0x08, 0x08, 4, 0x20380380, 0),
	REG_REGION(0x08, 0x08, 4, 0x20380480, 0),
};

static struct reg_region vd_npu_reg_rgns[] = {
	/* npu pvtpll*/
	REG_REGION(0x24, 0x24, 4, PVTPLL_NPU_BASE, WMSK_VAL),
	REG_REGION(0x20, 0x20, 4, PVTPLL_NPU_BASE, WMSK_VAL),

	/* npu_cru */
	REG_REGION(0x300, 0x300, 4, NPUCRU_BASE, WMSK_VAL),

	/* npu_grf */
	REG_REGION(0x000, 0x004, 4, NPUGRF_BASE, WMSK_VAL),
	REG_REGION(0x008, 0x00c, 4, NPUGRF_BASE, 0),
	REG_REGION(0x010, 0x018, 4, NPUGRF_BASE, WMSK_VAL),

	/* qos */
	REG_REGION(0x08, 0x18, 4, 0x20340000, 0),

	/* shaping */
	REG_REGION(0x08, 0x08, 4, 0x20340080, 0),
};

static struct reg_region vd_core_reg_rgns[] = {
	/* cpu pvtpll */
	REG_REGION(0x24, 0x24, 4, PVTPLL_CORE_BASE, WMSK_VAL),
	REG_REGION(0x20, 0x20, 4, PVTPLL_CORE_BASE, WMSK_VAL),

	/* core_cru */
	REG_REGION(0x300, 0x308, 4, CORECRU_BASE, WMSK_VAL),
	REG_REGION(0xd00, 0xd00, 4, CORECRU_BASE, 0),
	REG_REGION(0xd04, 0xd04, 4, CORECRU_BASE, WMSK_VAL),

	/* core_grf */
	REG_REGION(0x000, 0x004, 4, COREGRF_BASE, WMSK_VAL),

	/* qos */
	REG_REGION(0x08, 0x18, 4, 0x20310000, 0),

	/* shaping */
	REG_REGION(0x08, 0x08, 4, 0x20310080, 0),
};

static uint32_t npu_clk_gt_save, vdo_clk_gt_save[VDO_CRU_CLKGATE_CON_CNT];

void pd_aiisp_save(void)
{
	rockchip_reg_rgn_save(pd_aiisp_reg_rgns, ARRAY_SIZE(pd_aiisp_reg_rgns));
}

void pd_aiisp_restore(void)
{
	rockchip_reg_rgn_restore(pd_aiisp_reg_rgns, ARRAY_SIZE(pd_aiisp_reg_rgns));
}

void pd_vdo_save(void)
{
	int i;

	for (i = 0; i < VDO_CRU_CLKGATE_CON_CNT; i++) {
		vdo_clk_gt_save[i] = mmio_read_32(VDOCRU_BASE + VDO_CRU_CLKGATE_CON(i));
		mmio_write_32(VDOCRU_BASE + VDO_CRU_CLKGATE_CON(i), 0xffff0000);
	}

	rockchip_reg_rgn_save(pd_vdo_reg_rgns, ARRAY_SIZE(pd_vdo_reg_rgns));
}

void pd_vdo_restore(void)
{
	int i;

	rockchip_reg_rgn_restore(pd_vdo_reg_rgns, ARRAY_SIZE(pd_vdo_reg_rgns));

	for (i = 0; i < VDO_CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(VDOCRU_BASE + VDO_CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(vdo_clk_gt_save[i]));
}

void vd_npu_save(void)
{
	npu_clk_gt_save = mmio_read_32(NPUCRU_BASE + NPU_CRU_CLKGATE_CON0);

	mmio_write_32(NPUCRU_BASE + NPU_CRU_CLKGATE_CON0, 0xffff0000);

	rockchip_reg_rgn_save(vd_npu_reg_rgns, ARRAY_SIZE(vd_npu_reg_rgns));
}

void vd_npu_restore(void)
{
	rockchip_reg_rgn_restore(vd_npu_reg_rgns, ARRAY_SIZE(vd_npu_reg_rgns));

	mmio_write_32(NPUCRU_BASE + NPU_CRU_CLKGATE_CON0, WITH_16BITS_WMSK(npu_clk_gt_save));
}

void vd_core_save(void)
{
	rockchip_reg_rgn_save(vd_core_reg_rgns, ARRAY_SIZE(vd_core_reg_rgns));
}

void vd_core_restore(void)
{
	/* slow mode */
	mmio_write_32(TOPCRU_BASE + 0x280, 0x003f0000);

	rockchip_reg_rgn_restore(vd_core_reg_rgns, ARRAY_SIZE(vd_core_reg_rgns));

	/* switch to pll */
	mmio_write_32(TOPCRU_BASE + 0x280, 0x003f0015);
}

void pm_reg_rgns_init(void)
{
	rockchip_alloc_region_mem(pd_aiisp_reg_rgns, ARRAY_SIZE(pd_aiisp_reg_rgns));
	rockchip_alloc_region_mem(pd_vdo_reg_rgns, ARRAY_SIZE(pd_vdo_reg_rgns));
	rockchip_alloc_region_mem(vd_npu_reg_rgns, ARRAY_SIZE(vd_npu_reg_rgns));
	rockchip_alloc_region_mem(vd_core_reg_rgns, ARRAY_SIZE(vd_core_reg_rgns));
}
