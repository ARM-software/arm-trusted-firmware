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
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <platform.h>

#include <plat_private.h>
#include <platform_def.h>
#include <pmu.h>
#include <rv1126b_clk.h>
#include <secure.h>
#include <soc.h>

const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(RV1126B_DEV_RNG0_BASE, RV1126B_DEV_RNG0_SIZE,
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
};

static void print_glb_reset_status(void)
{
	uint32_t glb_rst_st = 0;
	uint32_t warm_boot = 0;

	/* clear CRU_GLB_RST_ST_NCLR if cold boot */
	if (mmio_read_32(PMUGRF_BASE + PMUGRF_OS_REG(11)) == WARM_BOOT_MAGIC) {
		glb_rst_st = mmio_read_32(TOPCRU_BASE + CRU_GLB_RST_ST_NCLR);
		warm_boot = 1;
	} else {
		mmio_write_32(PMUGRF_BASE + PMUGRF_OS_REG(11), WARM_BOOT_MAGIC);
	}

	mmio_write_32(TOPCRU_BASE + CRU_GLB_RST_ST_NCLR, 0xffff);

	/* save glb_rst_st in mem_os_reg0 */
	write_mem_os_reg(0, glb_rst_st);

	if (warm_boot != 0)
		INFO("soc warm boot, reset status: 0x%x\n", glb_rst_st);
	else
		INFO("soc cold boot\n");
}

static void system_reset_init(void)
{
	print_glb_reset_status();

	/*
	 * enable tsadc trigger global reset and select first reset.
	 * enable global reset and wdt trigger pmu reset.
	 * select first reset trigger pmu reset.
	 */
	mmio_write_32(TOPCRU_BASE + CRU_GLB_RST_CON(0), 0x3f);
	/* the CRU APB is 100M and clk of rst_con0~2 is 24M,
	 * so need 3 cycle of (24M + apb_cru_clk) to sync at most.
	 */
	udelay(1);
	mmio_write_32(TOPCRU_BASE + CRU_GLB_RST_CON(1), 0x1);
	udelay(1);
	mmio_write_32(TOPCRU_BASE + CRU_GLB_RST_CON(2), 0x1);
	udelay(1);

	/* enable wdt_s reset */
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_CON, BITS_WITH_WMASK(0x1, 0x1, 8));

	/* enable wdt_ns reset */
	mmio_write_32(SYSGRF_BASE + SYSGRF_BUSGRF_MISC, BITS_WITH_WMASK(0x1, 0x3, 12));

	/* reset width = 0xffff */
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(3), 0xffffffff);

	/* enable first/tsadc/wdt reset output */
	mmio_write_32(PMUSGRF_BASE + PMUSGRF_SOC_CON(0), BITS_WITH_WMASK(0x7, 0x7, 9));

	/* pmugrf/pmu_ioc hold */
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(4), 0xffff0120);

	/* pmusgrf hold */
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(5), 0xffff8000);

	/* select tsadc_shut_m0 iomux */
	mmio_write_32(PMU0IOC_BASE + 0x0, 0x00f00030);
}

void plat_rockchip_soc_init(void)
{
	rockchip_clock_init();
	system_reset_init();
	secure_init();
	rockchip_init_scmi_server();
}
