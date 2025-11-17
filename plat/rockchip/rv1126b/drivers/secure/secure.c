// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#include <assert.h>

#include <lib/mmio.h>

#include <plat_private.h>
#include <platform_def.h>
#include <secure.h>
#include <soc.h>

/* unit: Mb */
static void ddr_fw_rgn_config(uint64_t base_mb, uint64_t top_mb, int rgn_id)
{
	mmio_write_32(FW_DDR_BASE + FIREWALL_DDR_RGN(rgn_id),
		      RG_MAP_SECURE(top_mb << MB_TO_128KB_SHFT, base_mb << MB_TO_128KB_SHFT));

	/* enable region */
	mmio_setbits_32(FW_DDR_BASE + FIREWALL_DDR_CON, BIT(rgn_id));
}

/* Unit: Kb */
static void sram_fw_rgn_config(uint64_t base_kb, uint64_t top_kb, int rgn_id)
{
	uint32_t sft = rgn_id * 8;
	uint32_t val = RG_MAP_SRAM_SECURE(top_kb, base_kb);

	mmio_write_32(SYSSGRF_BASE + SYSSGRF_FIREWALL_CON(10), BITS_WITH_WMASK(val, 0xff, sft));

	/* enable region */
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_FIREWALL_CON(11), BITS_WITH_WMASK(1, 0x1, rgn_id));
}

static void secure_region_init(void)
{
	uint32_t i;

	/* disable all region first */
	mmio_write_32(FW_DDR_BASE + FIREWALL_DDR_CON, 0x0);
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_FIREWALL_CON(11), BITS_WITH_WMASK(0, 0x3, 0));

	/* all other ns-master can't access all ddr-regions */
	for (i = 0; i < FIREWALL_DDR_MST_CNT; i++)
		mmio_write_32(FW_DDR_BASE + FIREWALL_DDR_MST(i), 0xffffffff);

	/* Use FW_DDR_RGN0_REG to config 0~1M of ddr space to secure */
	ddr_fw_rgn_config(TZRAM_BASE >> 20, (TZRAM_BASE + TZRAM_SIZE) >> 20, 0);

	/* Use FIREWALL_SYSMEM_RGN0 to config 48~64k of sram as secure */
	sram_fw_rgn_config(44, 64, 0);
}

void secure_timer_init(void)
{
	/* rktimer switch to 24M */
	mmio_write_32(TOPCRU_BASE + CRU_CLKSEL_CON(46), BITS_WITH_WMASK(1, 0x1, 11));

	/* clear hptimer int_st */
	mmio_write_32(HPTIMER_BASE + TIMER_HP_INTR_STATUS, BIT(2));
}

void fw_init(void)
{
	int i;

	for (i = 0; i < 10; i++)
		mmio_write_32(SYSSGRF_BASE + SYSSGRF_FIREWALL_CON(i), 0xffff0000);

	/* slave as NS, except dcf, otpmask */
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_FIREWALL_CON(4), 0x24002400);
	/* slave as NS, except umctl2, dfi */
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_FIREWALL_CON(6), 0xa000a000);
	/* slave as NS, except pmusram */
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_FIREWALL_CON(11), BITS_WITH_WMASK(0x1, 0x7f, 4));

	secure_region_init();

	/* master as NS */
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_APB_SECURE_CON, 0xffff0000);
	/* master as NS */
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_AHB_SECURE_CON, 0xffffffff);
	/* master as NS, except dcf */
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_AXI_SECURE_CON, 0xfffffff3);
}

void secure_init(void)
{
	secure_timer_init();
	fw_init();
}
