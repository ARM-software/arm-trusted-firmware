/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <plat_private.h>
#include <rk3288_def.h>
#include <soc.h>
#include <secure.h>

/* sleep data for pll suspend */
static struct deepsleep_data_s slp_data;

/* Table of regions to map using the MMU. */
const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(GIC400_BASE, GIC400_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(STIME_BASE, STIME_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SGRF_BASE, SGRF_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TZPC_BASE, TZPC_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMUSRAM_BASE, PMUSRAM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SRAM_BASE, SRAM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMU_BASE, PMU_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(UART0_BASE, UART0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(UART1_BASE, UART1_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(UART2_BASE, UART2_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(UART3_BASE, UART3_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(UART4_BASE, UART4_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CRU_BASE, CRU_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GRF_BASE, GRF_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_PCTL0_BASE, DDR_PCTL0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_PHY0_BASE, DDR_PHY0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_PCTL1_BASE, DDR_PCTL1_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_PHY1_BASE, DDR_PHY1_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SERVICE_BUS_BASE, SERVICE_BUS_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CORE_AXI_BUS_BASE, CORE_AXI_BUS_SIZE,
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
};

void plat_rockchip_soc_init(void)
{
	secure_timer_init();
	secure_sgrf_init();
	/*
	 * We cannot enable ddr security at this point, as the kernel
	 * seems to have an issue with it even living in the same 128MB
	 * memory block. Only when moving the kernel to the second
	 * 128MB block does it not conflict, but then we'd loose this
	 * memory area for use. Late maybe enable
	 * secure_sgrf_ddr_rgn_init();
	 */
}

void regs_update_bits(uintptr_t addr, uint32_t val,
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

static void pll_save(uint32_t pll_id)
{
	uint32_t *pll = slp_data.pll_con[pll_id];

	pll[0] = mmio_read_32(CRU_BASE + PLL_CONS((pll_id), 0));
	pll[1] = mmio_read_32(CRU_BASE + PLL_CONS((pll_id), 1));
	pll[2] = mmio_read_32(CRU_BASE + PLL_CONS((pll_id), 2));
	pll[3] = mmio_read_32(CRU_BASE + PLL_CONS((pll_id), 3));
}

void clk_plls_suspend(void)
{
	pll_save(NPLL_ID);
	pll_save(CPLL_ID);
	pll_save(GPLL_ID);
	pll_save(APLL_ID);
	slp_data.pll_mode = mmio_read_32(CRU_BASE + PLL_MODE_CON);

	/*
	 * Switch PLLs other than DPLL (for SDRAM) to slow mode to
	 * avoid crashes on resume. The Mask ROM on the system will
	 * put APLL, CPLL, and GPLL into slow mode at resume time
	 * anyway (which is why we restore them), but we might not
	 * even make it to the Mask ROM if this isn't done at suspend
	 * time.
	 *
	 * NOTE: only APLL truly matters here, but we'll do them all.
	 */
	mmio_write_32(CRU_BASE + PLL_MODE_CON, 0xf3030000);
}

void clk_plls_resume(void)
{
	/* restore pll-modes */
	mmio_write_32(CRU_BASE + PLL_MODE_CON,
		      slp_data.pll_mode | REG_SOC_WMSK);
}

void clk_gate_con_save(void)
{
	uint32_t i = 0;

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++)
		slp_data.cru_gate_con[i] =
			mmio_read_32(CRU_BASE + CRU_CLKGATES_CON(i));
}

void clk_gate_con_disable(void)
{
	uint32_t i;

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++)
		mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(i), REG_SOC_WMSK);
}

void clk_gate_con_restore(void)
{
	uint32_t i;

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++)
		mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(i),
			      REG_SOC_WMSK | slp_data.cru_gate_con[i]);
}

void clk_sel_con_save(void)
{
	uint32_t i = 0;

	for (i = 0; i < CRU_CLKSELS_CON_CNT; i++)
		slp_data.cru_sel_con[i] =
			mmio_read_32(CRU_BASE + CRU_CLKSELS_CON(i));
}

void clk_sel_con_restore(void)
{
	uint32_t i, val;

	for (i = 0; i < CRU_CLKSELS_CON_CNT; i++) {
		/* fractional dividers don't have write-masks */
		if ((i >= 7 && i <= 9) ||
		    (i >= 17 && i <= 20) ||
		    (i == 23) || (i == 41))
			val = slp_data.cru_sel_con[i];
		else
			val = slp_data.cru_sel_con[i] | REG_SOC_WMSK;

		mmio_write_32(CRU_BASE + CRU_CLKSELS_CON(i), val);
	}
}

void __dead2 rockchip_soc_soft_reset(void)
{
	uint32_t temp_val;

	/*
	 * Switch PLLs other than DPLL (for SDRAM) to slow mode to
	 * avoid crashes on resume. The Mask ROM on the system will
	 * put APLL, CPLL, and GPLL into slow mode at resume time
	 * anyway (which is why we restore them), but we might not
	 * even make it to the Mask ROM if this isn't done at suspend
	 * time.
	 *
	 * NOTE: only APLL truly matters here, but we'll do them all.
	 */
	mmio_write_32(CRU_BASE + PLL_MODE_CON, 0xf3030000);

	temp_val = mmio_read_32(CRU_BASE + CRU_GLB_RST_CON);
	temp_val &= ~PMU_RST_MASK;
	temp_val |= PMU_RST_BY_SECOND_SFT;
	mmio_write_32(CRU_BASE + CRU_GLB_RST_CON, temp_val);
	mmio_write_32(CRU_BASE + CRU_GLB_SRST_SND, 0xeca8);

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to excute valid codes.
	 */
	while (1)
		;
}
