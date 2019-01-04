/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
		mmio_write_32(SGRF_BASE + SGRF_DDRRGN_CON0_16(16),
			      SGRF_DDR_RGN_BYPS);
	else
		/* cancel bypass for whole ddr regions */
		mmio_write_32(SGRF_BASE + SGRF_DDRRGN_CON0_16(16),
			      SGRF_DDR_RGN_NO_BYPS);
}

/**
 * There are 8 + 1 regions for DDR secure control:
 * DDR_RGN_0 ~ DDR_RGN_7: Per DDR_RGNs grain size is 1MB
 * DDR_RGN_X - the memories of exclude DDR_RGN_0 ~ DDR_RGN_7
 *
 * DDR_RGN_0 - start address of the RGN0
 * DDR_RGN_8 - end address of the RGN0
 * DDR_RGN_1 - start address of the RGN1
 * DDR_RGN_9 - end address of the RGN1
 * ...
 * DDR_RGN_7 - start address of the RGN7
 * DDR_RGN_15 - end address of the RGN7
 * DDR_RGN_16 - bit 0 ~ 7 is bitmap for RGN0~7 secure,0: disable, 1: enable
 *              bit 8 is setting for RGNx, the rest of the memory and region
 *                which excludes RGN0~7, 0: disable, 1: enable
 *              bit 9, the global secure configuration via bypass, 0: disable
 *                bypass, 1: enable bypass
 *
 * @rgn - the DDR regions 0 ~ 7 which are can be configured.
 * The @st_mb and @ed_mb indicate the start and end addresses for which to set
 * the security, and the unit is megabyte. When the st_mb == 0, ed_mb == 0, the
 * address range 0x0 ~ 0xfffff is secure.
 *
 * For example, if we would like to set the range [0, 32MB) is security via
 * DDR_RGN0, then rgn == 0, st_mb == 0, ed_mb == 31.
 */
static void sgrf_ddr_rgn_config(uint32_t rgn,
				uintptr_t st, uintptr_t ed)
{
	uintptr_t st_mb, ed_mb;

	assert(rgn <= 7);
	assert(st < ed);

	/* check aligned 1MB */
	assert(st % SIZE_M(1) == 0);
	assert(ed % SIZE_M(1) == 0);

	st_mb = st / SIZE_M(1);
	ed_mb = ed / SIZE_M(1);

	/* set ddr region addr start */
	mmio_write_32(SGRF_BASE + SGRF_DDRRGN_CON0_16(rgn),
		      BITS_WITH_WMASK(st_mb, SGRF_DDR_RGN_0_16_WMSK, 0));

	/* set ddr region addr end */
	mmio_write_32(SGRF_BASE + SGRF_DDRRGN_CON0_16(rgn + 8),
		      BITS_WITH_WMASK((ed_mb - 1), SGRF_DDR_RGN_0_16_WMSK, 0));

	mmio_write_32(SGRF_BASE + SGRF_DDRRGN_CON0_16(16),
		      BIT_WITH_WMSK(rgn));
}

void secure_watchdog_gate(void)
{
	/**
	 * Disable CA53 and CM0 wdt pclk
	 * BIT[8]: ca53 wdt pclk, 0: enable 1: disable
	 * BIT[10]: cm0 wdt pclk, 0: enable 1: disable
	 */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(3),
		      BIT_WITH_WMSK(PCLK_WDT_CA53_GATE_SHIFT) |
		      BIT_WITH_WMSK(PCLK_WDT_CM0_GATE_SHIFT));
}

__pmusramfunc void secure_watchdog_ungate(void)
{
	/**
	 * Enable CA53 and CM0 wdt pclk
	 * BIT[8]: ca53 wdt pclk, 0: enable 1: disable
	 * BIT[10]: cm0 wdt pclk, 0: enable 1: disable
	 */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(3),
		      WMSK_BIT(PCLK_WDT_CA53_GATE_SHIFT) |
		      WMSK_BIT(PCLK_WDT_CM0_GATE_SHIFT));
}

__pmusramfunc void sram_secure_timer_init(void)
{
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_END_COUNT0, 0xffffffff);
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_END_COUNT1, 0xffffffff);

	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_INIT_COUNT0, 0x0);
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_INIT_COUNT0, 0x0);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_CONTROL_REG,
		      TIMER_EN | TIMER_FMODE);
}

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

void secure_sgrf_init(void)
{
	/* security config for master */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(5),
		      REG_SOC_WMSK | SGRF_SOC_ALLMST_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6),
		      REG_SOC_WMSK | SGRF_SOC_ALLMST_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(7),
		      REG_SOC_WMSK | SGRF_SOC_ALLMST_NS);

	/* security config for slave */
	mmio_write_32(SGRF_BASE + SGRF_PMU_SLV_CON0_1(0),
		      SGRF_PMU_SLV_S_CFGED |
		      SGRF_PMU_SLV_CRYPTO1_NS);
	mmio_write_32(SGRF_BASE + SGRF_PMU_SLV_CON0_1(1),
		      SGRF_SLV_S_WMSK | SGRF_PMUSRAM_S);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(0),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(1),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(2),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(3),
		      SGRF_SLV_S_WMSK | SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0_4(4),
		      SGRF_SLV_S_WMSK | SGRF_INTSRAM_S);
}

void secure_sgrf_ddr_rgn_init(void)
{
	sgrf_ddr_rgn_config(0, TZRAM_BASE, TZRAM_SIZE);
	sgrf_ddr_rgn_global_bypass(0);
}
