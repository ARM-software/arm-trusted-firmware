/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>

#include <plat_private.h>
#include <secure.h>
#include <soc.h>

static void sgrf_ddr_rgn_global_bypass(uint32_t bypass)
{
	if (bypass)
		/* set bypass (non-secure regions) for whole ddr regions */
		mmio_write_32(SGRF_BASE + SGRF_SOC_CON(21),
			      SGRF_DDR_RGN_BYPS);
	else
		/* cancel bypass for whole ddr regions */
		mmio_write_32(SGRF_BASE + SGRF_SOC_CON(21),
			      SGRF_DDR_RGN_NO_BYPS);
}

/**
 * There are 8 + 1 regions for DDR secure control:
 * DDR_RGN_0 ~ DDR_RGN_7: Per DDR_RGNs grain size is 1MB
 * DDR_RGN_X - the memories of exclude DDR_RGN_0 ~ DDR_RGN_7
 *
 * SGRF_SOC_CON6 - start address of RGN_0 + control
 * SGRF_SOC_CON7 - end address of RGN_0
 * ...
 * SGRF_SOC_CON20 - start address of the RGN_7 + control
 * SGRF_SOC_CON21 - end address of the RGN_7 + RGN_X control
 *
 * @rgn - the DDR regions 0 ~ 7 which are can be configured.
 * @st - start address to set as secure
 * @sz - length of area to set as secure
 * The @st_mb and @ed_mb indicate the start and end addresses for which to set
 * the security, and the unit is megabyte. When the st_mb == 0, ed_mb == 0, the
 * address range 0x0 ~ 0xfffff is secure.
 *
 * For example, if we would like to set the range [0, 32MB) is security via
 * DDR_RGN0, then rgn == 0, st_mb == 0, ed_mb == 31.
 */
static void sgrf_ddr_rgn_config(uint32_t rgn, uintptr_t st, size_t sz)
{
	uintptr_t ed = st + sz;
	uintptr_t st_mb, ed_mb;

	assert(rgn <= 7);
	assert(st < ed);

	/* check aligned 1MB */
	assert(st % SIZE_M(1) == 0);
	assert(ed % SIZE_M(1) == 0);

	st_mb = st / SIZE_M(1);
	ed_mb = ed / SIZE_M(1);

	/* set ddr region addr start */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6 + (rgn * 2)),
		      BITS_WITH_WMASK(st_mb, SGRF_DDR_RGN_ADDR_WMSK, 0));

	/* set ddr region addr end */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6 + (rgn * 2) + 1),
		      BITS_WITH_WMASK((ed_mb - 1), SGRF_DDR_RGN_ADDR_WMSK, 0));

	/* select region security */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6 + (rgn * 2)),
		      SGRF_DDR_RGN_SECURE_SEL);

	/* enable region security */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6 + (rgn * 2)),
		      SGRF_DDR_RGN_SECURE_EN);
}

void secure_watchdog_gate(void)
{
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(0), SGRF_PCLK_WDT_GATE);
}

void secure_watchdog_ungate(void)
{
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(0), SGRF_PCLK_WDT_UNGATE);
}

__pmusramfunc void sram_secure_timer_init(void)
{
	mmio_write_32(STIMER1_BASE + TIMER_CONTROL_REG, 0);

	mmio_write_32(STIMER1_BASE + TIMER_LOAD_COUNT0, 0xffffffff);
	mmio_write_32(STIMER1_BASE + TIMER_LOAD_COUNT1, 0xffffffff);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER1_BASE + TIMER_CONTROL_REG, TIMER_EN);
}

void secure_gic_init(void)
{
	/* (re-)enable non-secure access to the gic*/
	mmio_write_32(CORE_AXI_BUS_BASE + CORE_AXI_SECURITY0,
		      AXI_SECURITY0_GIC);
}

void secure_timer_init(void)
{
	mmio_write_32(STIMER1_BASE + TIMER_CONTROL_REG, 0);

	mmio_write_32(STIMER1_BASE + TIMER_LOAD_COUNT0, 0xffffffff);
	mmio_write_32(STIMER1_BASE + TIMER_LOAD_COUNT1, 0xffffffff);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER1_BASE + TIMER_CONTROL_REG, TIMER_EN);
}

void secure_sgrf_init(void)
{
	/*
	 * We use the first sram part to talk to the bootrom,
	 * so make it secure.
	 */
	mmio_write_32(TZPC_BASE + TZPC_R0SIZE, TZPC_SRAM_SECURE_4K(1));

	secure_gic_init();

	/* set all master ip to non-secure */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(2), SGRF_SOC_CON2_MST_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(3), SGRF_SOC_CON3_MST_NS);

	/* setting all configurable ip into non-secure */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(4),
		      SGRF_SOC_CON4_SECURE_WMSK /*TODO:|SGRF_STIMER_SECURE*/);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(5), SGRF_SOC_CON5_SECURE_WMSK);

	/* secure dma to non-secure */
	mmio_write_32(TZPC_BASE + TZPC_DECPROT1SET, 0xff);
	mmio_write_32(TZPC_BASE + TZPC_DECPROT2SET, 0xff);
	mmio_write_32(SGRF_BASE + SGRF_BUSDMAC_CON(1), 0x3800);
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

void secure_sgrf_ddr_rgn_init(void)
{
	sgrf_ddr_rgn_config(0, TZRAM_BASE, TZRAM_SIZE);
	sgrf_ddr_rgn_global_bypass(0);
}
