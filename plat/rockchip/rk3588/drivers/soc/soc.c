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
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <platform.h>
#include <platform_def.h>
#include <pmu.h>

#include <plat_private.h>
#include <rk3588_clk.h>
#include <secure.h>
#include <soc.h>

#define RK3588_DEV_RNG0_BASE	0xf0000000
#define RK3588_DEV_RNG0_SIZE	0x0ffff000

const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(RK3588_DEV_RNG0_BASE, RK3588_DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_SHARE_MEM, DDR_SHARE_SIZE,
			MT_DEVICE | MT_RW | MT_NS),
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

void timer_hp_init(void)
{
	if ((mmio_read_32(TIMER_HP_BASE + TIMER_HP_CTRL) & 0x1) != 0)
		return;

	mmio_write_32(TIMER_HP_BASE + TIMER_HP_CTRL, 0x0);
	dsb();
	mmio_write_32(TIMER_HP_BASE + TIMER_HP_LOAD_COUNT0, 0xffffffff);
	mmio_write_32(TIMER_HP_BASE + TIMER_HP_LOAD_COUNT1, 0xffffffff);
	mmio_write_32(TIMER_HP_BASE + TIMER_HP_INT_EN, 0);
	dsb();
	mmio_write_32(TIMER_HP_BASE + TIMER_HP_CTRL, 0x1);
}

static void system_reset_init(void)
{
	/* enable wdt_ns0~4 trigger global reset and select first reset.
	 * enable tsadc trigger global reset and select first reset.
	 * enable global reset and wdt trigger pmu reset.
	 * select first reset trigger pmu reset.s
	 */
	mmio_write_32(CRU_BASE + CRU_GLB_RST_CON, 0xffdf);

	/* enable wdt_s, wdt_ns reset */
	mmio_write_32(BUSSGRF_BASE + SGRF_SOC_CON(2), 0x0c000c00);

	/* reset width = 0xffff */
	mmio_write_32(PMU1GRF_BASE + PMU1GRF_SOC_CON(1), 0xffffffff);

	/* enable first/tsadc/wdt reset output */
	mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(0), 0x00070007);

	/* pmu1_grf pmu1_ioc hold */
	mmio_write_32(PMU1GRF_BASE + PMU1GRF_SOC_CON(7), 0x30003000);

	/* pmu1sgrf hold */
	mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(14), 0x00200020);

	/* select tsadc_shut_m0 ionmux*/
	mmio_write_32(PMU0IOC_BASE + 0x0, 0x00f00020);
}

void plat_rockchip_soc_init(void)
{
	rockchip_clock_init();
	secure_timer_init();
	timer_hp_init();
	system_reset_init();
	sgrf_init();
	rockchip_init_scmi_server();
}
