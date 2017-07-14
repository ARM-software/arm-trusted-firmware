/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <plat_private.h>
#include <platform_def.h>
#include <rk3368_def.h>
#include <soc.h>

static uint32_t plls_con[END_PLL_ID][4];

/* Table of regions to map using the MMU. */
const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(CCI400_BASE, CCI400_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GIC400_BASE, GIC400_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(STIME_BASE, STIME_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SGRF_BASE, SGRF_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMUSRAM_BASE, PMUSRAM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMU_BASE, PMU_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(UART_DBG_BASE, UART_DBG_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CRU_BASE, CRU_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_PCTL_BASE, DDR_PCTL_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_PHY_BASE, DDR_PHY_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GRF_BASE, GRF_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SERVICE_BUS_BASE, SERVICE_BUS_SISE,
			MT_DEVICE | MT_RW | MT_SECURE),
	{ 0 }
};

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
	mmio_write_32(STIMER1_BASE + TIMER_LOADE_COUNT0, 0xffffffff);
	mmio_write_32(STIMER1_BASE + TIMER_LOADE_COUNT1, 0xffffffff);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER1_BASE + TIMER_CONTROL_REG, TIMER_EN);
}

void sgrf_init(void)
{
	/* setting all configurable ip into no-secure */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(5), SGRF_SOC_CON_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6), SGRF_SOC_CON7_BITS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(7), SGRF_SOC_CON_NS);

	/* secure dma to no sesure */
	mmio_write_32(SGRF_BASE + SGRF_BUSDMAC_CON(0), SGRF_BUSDMAC_CON0_NS);
	mmio_write_32(SGRF_BASE + SGRF_BUSDMAC_CON(1), SGRF_BUSDMAC_CON1_NS);
	dsb();

	/* rst dma1 */
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(1),
		      RST_DMA1_MSK | (RST_DMA1_MSK << 16));
	/* rst dma2 */
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(4),
		      RST_DMA2_MSK | (RST_DMA2_MSK << 16));

	dsb();

	/* release dma1 rst*/
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(1), (RST_DMA1_MSK << 16));
	/* release dma2 rst*/
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(4), (RST_DMA2_MSK << 16));
}

void plat_rockchip_soc_init(void)
{
	secure_timer_init();
	sgrf_init();
}

void regs_updata_bits(uintptr_t addr, uint32_t val,
		      uint32_t mask, uint32_t shift)
{
	uint32_t tmp, orig;

	orig = mmio_read_32(addr);

	tmp = orig & ~(mask << shift);
	tmp |= (val & mask) << shift;

	if (tmp != orig)
		mmio_write_32(addr, tmp);
	dsb();
}

static void plls_suspend(uint32_t pll_id)
{
	plls_con[pll_id][0] = mmio_read_32(CRU_BASE + PLL_CONS((pll_id), 0));
	plls_con[pll_id][1] = mmio_read_32(CRU_BASE + PLL_CONS((pll_id), 1));
	plls_con[pll_id][2] = mmio_read_32(CRU_BASE + PLL_CONS((pll_id), 2));
	plls_con[pll_id][3] = mmio_read_32(CRU_BASE + PLL_CONS((pll_id), 3));

	mmio_write_32(CRU_BASE + PLL_CONS((pll_id), 3), PLL_SLOW_BITS);
	mmio_write_32(CRU_BASE + PLL_CONS((pll_id), 3), PLL_BYPASS);
}

static void pm_plls_suspend(void)
{
	plls_suspend(NPLL_ID);
	plls_suspend(CPLL_ID);
	plls_suspend(GPLL_ID);
	plls_suspend(ABPLL_ID);
	plls_suspend(ALPLL_ID);
}

static inline void plls_resume(void)
{
	mmio_write_32(CRU_BASE + PLL_CONS(ABPLL_ID, 3),
		      plls_con[ABPLL_ID][3] | PLL_BYPASS_W_MSK);
	mmio_write_32(CRU_BASE + PLL_CONS(ALPLL_ID, 3),
		      plls_con[ALPLL_ID][3] | PLL_BYPASS_W_MSK);
	mmio_write_32(CRU_BASE + PLL_CONS(GPLL_ID, 3),
		      plls_con[GPLL_ID][3] | PLL_BYPASS_W_MSK);
	mmio_write_32(CRU_BASE + PLL_CONS(CPLL_ID, 3),
		      plls_con[CPLL_ID][3] | PLL_BYPASS_W_MSK);
	mmio_write_32(CRU_BASE + PLL_CONS(NPLL_ID, 3),
		      plls_con[NPLL_ID][3] | PLL_BYPASS_W_MSK);
}

void soc_sleep_config(void)
{
	int i = 0;

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++)
		mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(i), 0xffff0000);
	pm_plls_suspend();

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++)
		mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(i), 0xffff0000);
}

void pm_plls_resume(void)
{
	plls_resume();

	mmio_write_32(CRU_BASE + PLL_CONS(ABPLL_ID, 3),
		      plls_con[ABPLL_ID][3] | PLLS_MODE_WMASK);
	mmio_write_32(CRU_BASE + PLL_CONS(ALPLL_ID, 3),
		      plls_con[ALPLL_ID][3] | PLLS_MODE_WMASK);
	mmio_write_32(CRU_BASE + PLL_CONS(GPLL_ID, 3),
		      plls_con[GPLL_ID][3] | PLLS_MODE_WMASK);
	mmio_write_32(CRU_BASE + PLL_CONS(CPLL_ID, 3),
		      plls_con[CPLL_ID][3] | PLLS_MODE_WMASK);
	mmio_write_32(CRU_BASE + PLL_CONS(NPLL_ID, 3),
		      plls_con[NPLL_ID][3] | PLLS_MODE_WMASK);
}

void __dead2 rockchip_soc_soft_reset(void)
{
	uint32_t temp_val;

	mmio_write_32(CRU_BASE + PLL_CONS((GPLL_ID), 3), PLL_SLOW_BITS);
	mmio_write_32(CRU_BASE + PLL_CONS((CPLL_ID), 3), PLL_SLOW_BITS);
	mmio_write_32(CRU_BASE + PLL_CONS((NPLL_ID), 3), PLL_SLOW_BITS);
	mmio_write_32(CRU_BASE + PLL_CONS((ABPLL_ID), 3), PLL_SLOW_BITS);
	mmio_write_32(CRU_BASE + PLL_CONS((ALPLL_ID), 3), PLL_SLOW_BITS);

	temp_val = mmio_read_32(CRU_BASE + CRU_GLB_RST_CON) |
		   PMU_RST_BY_SECOND_SFT;

	mmio_write_32(CRU_BASE + CRU_GLB_RST_CON, temp_val);
	mmio_write_32(CRU_BASE + CRU_GLB_SRST_SND, 0xeca8);

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to excute valid codes.
	 */
	while (1)
	;
}
