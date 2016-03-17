/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <arch_helpers.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <platform_def.h>
#include <plat_private.h>
#include <rk3399_def.h>
#include <soc.h>

/* The RockChip power domain tree descriptor */
const unsigned char rockchip_power_domain_tree_desc[] = {
	/* No of root nodes */
	PLATFORM_SYSTEM_COUNT,
	/* No of children for the root node */
	PLATFORM_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
	/* No of children for the second cluster node */
	PLATFORM_CLUSTER1_CORE_COUNT
};

void secure_timer_init(void)
{
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_END_COUNT0, 0xffffffff);
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_END_COUNT1, 0xffffffff);

	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_INIT_COUNT0, 0x0);
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_INIT_COUNT0, 0x0);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_CONTROL_REG,
		      TIMER_EN | TIMER_FMODE);
}

void sgrf_init(void)
{
	/* security config for master */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON3_7(5),
		      SGRF_SOC_CON_WMSK | SGRF_SOC_ALLMST_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON3_7(6),
		      SGRF_SOC_CON_WMSK | SGRF_SOC_ALLMST_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON3_7(7),
		      SGRF_SOC_CON_WMSK | SGRF_SOC_ALLMST_NS);

	/* security config for slave */
	mmio_write_32(SGRF_BASE + SGRF_PMU_SLV_CON0_1(0),
		      SGRF_PMU_SLV_S_CFGED |
		      SGRF_PMU_SLV_CRYPTO1_NS);
	mmio_write_32(SGRF_BASE + SGRF_PMU_SLV_CON0_1(1),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(0),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(1),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(2),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(3),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(4),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);

	/* security config for ddr memery */
	mmio_write_32(SGRF_BASE + SGRF_DDRRGN_CON0_16(16),
		      SGRF_DDR_RGN_BYPS);
}

static void dma_secure_cfg(void)
{
	/* rgn0 secure for dmac0 and dmac1 */
	mmio_write_32(SGRF_BASE + SGRF_DDRRGN_CON20_34(22),
		      SGRF_L_MST_S_DDR_RGN(0) | /* dmac0 */
		      SGRF_H_MST_S_DDR_RGN(0) /* dmac1 */
		      );

	/* set dmac0 boot, under secure state */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON8_15(8), SGRF_DMAC_CFG_S);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON8_15(9), SGRF_DMAC_CFG_S);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON8_15(10), SGRF_DMAC_CFG_S);

	/* dmac0 soft reset */
	mmio_write_32(CRU_BASE + CRU_SOFTRST_CON(10), CRU_DMAC0_RST);
	udelay(5);
	mmio_write_32(CRU_BASE + CRU_SOFTRST_CON(10), CRU_DMAC1_RST_RLS);

	/* set dmac1 boot, under secure state */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON8_15(11), SGRF_DMAC_CFG_S);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON8_15(12), SGRF_DMAC_CFG_S);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON8_15(13), SGRF_DMAC_CFG_S);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON8_15(14), SGRF_DMAC_CFG_S);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON8_15(15), SGRF_DMAC_CFG_S);

	/* dmac1 soft reset */
	mmio_write_32(CRU_BASE + CRU_SOFTRST_CON(10), CRU_DMAC1_RST);
	udelay(5);
	mmio_write_32(CRU_BASE + CRU_SOFTRST_CON(10), CRU_DMAC1_RST_RLS);
}

/* pll suspend */
struct deepsleep_data_s slp_data;

static void pll_suspend_prepare(uint32_t pll_id)
{
	int i;

	if (pll_id == PPLL_ID)
		for (i = 0; i < PLL_CON_COUNT; i++)
			slp_data.plls_con[pll_id][i] =
				mmio_read_32(PMUCRU_BASE + PMUCRU_PPLL_CON(i));
	else
		for (i = 0; i < PLL_CON_COUNT; i++)
			slp_data.plls_con[pll_id][i] =
				mmio_read_32(CRU_BASE + CRU_PLL_CON(pll_id, i));
}

static void set_pll_slow_mode(uint32_t pll_id)
{
	if (pll_id == PPLL_ID)
		mmio_write_32(PMUCRU_BASE + PMUCRU_PPLL_CON(3), PLL_SLOW_MODE);
	else
		mmio_write_32((CRU_BASE +
			      CRU_PLL_CON(pll_id, 3)), PLL_SLOW_MODE);
}

static void set_pll_normal_mode(uint32_t pll_id)
{
	if (pll_id == PPLL_ID)
		mmio_write_32(PMUCRU_BASE + PMUCRU_PPLL_CON(3), PLL_NOMAL_MODE);
	else
		mmio_write_32(CRU_BASE +
			      CRU_PLL_CON(pll_id, 3), PLL_NOMAL_MODE);
}

static void set_pll_bypass(uint32_t pll_id)
{
	if (pll_id == PPLL_ID)
		mmio_write_32(PMUCRU_BASE +
			      PMUCRU_PPLL_CON(3), PLL_BYPASS_MODE);
	else
		mmio_write_32(CRU_BASE +
			      CRU_PLL_CON(pll_id, 3), PLL_BYPASS_MODE);
}

static void _pll_suspend(uint32_t pll_id)
{
	set_pll_slow_mode(pll_id);
	set_pll_bypass(pll_id);
}

void plls_suspend(void)
{
	uint32_t i, pll_id;

	for (pll_id = ALPLL_ID; pll_id < END_PLL_ID; pll_id++)
		pll_suspend_prepare(pll_id);

	for (i = 0; i < CRU_CLKSEL_COUNT; i++)
		slp_data.cru_clksel_con[i] =
			mmio_read_32(CRU_BASE +
				     CRU_CLKSEL_OFFSET + i * REG_SIZE);

	for (i = 0; i < PMUCRU_CLKSEL_CONUT; i++)
		slp_data.pmucru_clksel_con[i] =
			mmio_read_32(PMUCRU_BASE +
				     PMUCRU_CLKSEL_OFFSET + i * REG_SIZE);

	_pll_suspend(CPLL_ID);
	_pll_suspend(NPLL_ID);
	_pll_suspend(VPLL_ID);
	_pll_suspend(PPLL_ID);
	_pll_suspend(GPLL_ID);
	_pll_suspend(ABPLL_ID);
	_pll_suspend(ALPLL_ID);
}

static void set_plls_nobypass(uint32_t pll_id)
{
	if (pll_id == PPLL_ID)
		mmio_write_32(PMUCRU_BASE + PMUCRU_PPLL_CON(3),
			      PLL_NO_BYPASS_MODE);
	else
		mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 3),
			      PLL_NO_BYPASS_MODE);
}

static void plls_resume_prepare(void)
{
	int i;

	for (i = 0; i < CRU_CLKSEL_COUNT; i++)
		mmio_write_32((CRU_BASE + CRU_CLKSEL_OFFSET + i * REG_SIZE),
			      REG_SOC_WMSK | slp_data.cru_clksel_con[i]);
	for (i = 0; i < PMUCRU_CLKSEL_CONUT; i++)
		mmio_write_32((PMUCRU_BASE +
			      PMUCRU_CLKSEL_OFFSET + i * REG_SIZE),
			      REG_SOC_WMSK | slp_data.pmucru_clksel_con[i]);
}

void plls_resume(void)
{
	int pll_id;

	plls_resume_prepare();
	for (pll_id = ALPLL_ID; pll_id < END_PLL_ID; pll_id++) {
		set_plls_nobypass(pll_id);
		set_pll_normal_mode(pll_id);
	}
}

void soc_global_soft_reset_init(void)
{
	mmio_write_32(PMUCRU_BASE + CRU_PMU_RSTHOLD_CON(1),
		      CRU_PMU_SGRF_RST_RLS);
	mmio_write_32(PMUCRU_BASE + CRU_PMU_RSTHOLD_CON(1),
		      CRU_PMU_GPIO0_RST_HOLD);
	mmio_write_32(PMUCRU_BASE + CRU_PMU_RSTHOLD_CON(1),
		      CRU_PMU_GPIO0_RST_HOLD);
}

void  __dead2 soc_global_soft_reset(void)
{
	uint32_t temp_val;

	set_pll_slow_mode(VPLL_ID);
	set_pll_slow_mode(NPLL_ID);
	set_pll_slow_mode(GPLL_ID);
	set_pll_slow_mode(CPLL_ID);
	set_pll_slow_mode(PPLL_ID);
	set_pll_slow_mode(ABPLL_ID);
	set_pll_slow_mode(ALPLL_ID);
	temp_val = mmio_read_32(CRU_BASE + CRU_GLB_RST_CON) |
		   PMU_RST_BY_FIRST_SFT;
	mmio_write_32(CRU_BASE + CRU_GLB_RST_CON, temp_val);
	mmio_write_32(CRU_BASE + CRU_GLB_SRST_FST, GLB_SRST_FST_CFG_VAL);

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to excute valid codes.
	 */
	while (1)
	;
}

void plat_rockchip_soc_init(void)
{
	secure_timer_init();
	dma_secure_cfg();
	sgrf_init();
	soc_global_soft_reset_init();
}
