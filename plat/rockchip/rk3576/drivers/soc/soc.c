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
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <platform.h>
#include <platform_def.h>
#include <pmu.h>

#include <plat_private.h>
#include <rk3576_clk.h>
#include <secure.h>
#include <soc.h>

const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(RK3576_DEV_RNG0_BASE, RK3576_DEV_RNG0_SIZE,
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

static void clear_glb_reset_status(void)
{
	uint32_t cru_sel55 = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(55));

	/* switch pclk_bus_root to 24M before writing CRU_GLB_RST_ST */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(55),
		      BITS_WITH_WMASK(2, 0x3, 2));
	dsb();

	mmio_write_32(CRU_BASE + CRU_GLB_RST_ST, 0xffff);
	dsb();

	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(55),
		      WITH_16BITS_WMSK(cru_sel55));
}

static void print_glb_reset_status(void)
{
	uint32_t glb_rst_st = 0, warm_boot = 0;

	/* clear CRU_GLB_RST_ST_NCLR if cold boot */
	if (mmio_read_32(PMU0_GRF_BASE + PMU0GRF_OS_REG(17)) == WARM_BOOT_MAGIC) {
		/* ignore npu_wdt*/
		glb_rst_st = mmio_read_32(CRU_BASE + CRU_GLB_RST_ST_NCLR) & VALID_GLB_RST_MSK;
		warm_boot = 1;
	} else {
		mmio_write_32(PMU0_GRF_BASE + PMU0GRF_OS_REG(17), WARM_BOOT_MAGIC);
	}

	clear_glb_reset_status();

	/* save glb_rst_st in mem_os_reg31 */
	write_mem_os_reg(31, glb_rst_st);

	if (warm_boot != 0)
		INFO("soc warm boot, reset status: 0x%x\n", glb_rst_st);
	else
		INFO("soc cold boot\n");
}

static void system_reset_init(void)
{
	/*
	 * enable tsadc trigger global reset and select first reset.
	 * enable global reset and wdt trigger pmu reset.
	 * select first reset trigger pmu reset.
	 */
	mmio_write_32(CRU_BASE + CRU_GLB_RST_CON, 0xffdf);

	/* enable wdt_s reset */
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(0), 0x20002000);

	/* enable wdt_ns reset */
	mmio_write_32(SYS_GRF_BASE + SYSGRF_SOC_CON(4), 0x01000100);

	/* reset width = 0xffff */
	mmio_write_32(PMU1_GRF_BASE + PMU1GRF_SOC_CON(6), 0xffffffff);

	/* enable first/tsadc/wdt reset output */
	mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(0), 0x00070007);

	/* pmu0sgrf pmu0_ioc hold */
	mmio_write_32(PMU0SGRF_BASE + PMU1SGRF_SOC_CON(0), 0xffff1800);

	/* pmu1sgrf pmu1_grf hold */
	mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(16), 0xffff8800);

	/* select tsadc_shut_m0 ionmux*/
	mmio_write_32(PMU0_IOC_BASE + 0x0, 0x00f00090);

	print_glb_reset_status();
}

void plat_rockchip_soc_init(void)
{
	rockchip_clock_init();
	system_reset_init();
	secure_init();
	rockchip_init_scmi_server();

	/* release cpu1~cpu7 */
	mmio_write_32(CCI_GRF_BASE + CCIGRF_CON(4), 0xffffffff);
	mmio_write_32(LITCORE_GRF_BASE + COREGRF_CPU_CON(1),
		      BITS_WITH_WMASK(0x77, 0xff, 4));
}
