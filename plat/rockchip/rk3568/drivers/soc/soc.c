/*
 * Copyright (c) 2023-2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <mmio.h>
#include <platform_def.h>

#include <plat_private.h>
#include <rk3568_clk.h>
#include <soc.h>

const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(RKFPGA_DEV_RNG0_BASE, RKFPGA_DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMUSRAM_BASE, PMUSRAM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
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

static void secure_timer_init(void)
{
	mmio_write_32(STIMER0_CHN_BASE(1) + TIMER_CONTROL_REG, TIMER_DIS);
	mmio_write_32(STIMER0_CHN_BASE(1) + TIMER_LOAD_COUNT0, 0xffffffff);
	mmio_write_32(STIMER0_CHN_BASE(1) + TIMER_LOAD_COUNT1, 0xffffffff);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER0_CHN_BASE(1) + TIMER_CONTROL_REG, TIMER_EN);
}

static void sgrf_init(void)
{
	mmio_write_32(SGRF_BASE + SGRF_FIREWALL_SLV_CON(0), 0xffff0000);
	mmio_write_32(SGRF_BASE + SGRF_FIREWALL_SLV_CON(1), 0xffff0000);
	mmio_write_32(SGRF_BASE + SGRF_FIREWALL_SLV_CON(2), 0xffff0000);
	mmio_write_32(SGRF_BASE + SGRF_FIREWALL_SLV_CON(3), 0xffff0000);
	mmio_write_32(SGRF_BASE + SGRF_FIREWALL_SLV_CON(4), 0xffff0000);
	mmio_write_32(SGRF_BASE + SGRF_FIREWALL_SLV_CON(5), 0xffff0000);
	mmio_write_32(SGRF_BASE + SGRF_FIREWALL_SLV_CON(6), 0xffff0000);
	mmio_write_32(SGRF_BASE + SGRF_FIREWALL_SLV_CON(7), 0xffff0000);
	mmio_write_32(SGRF_BASE + SGRF_FIREWALL_SLV_CON(8), 0xffff0000);

	mmio_write_32(DDRSGRF_BASE + FIREWALL_DDR_FW_DDR_CON_REG, 0xffff0000);
}

static void set_pll_slow_mode(uint32_t clk_pll)
{
	mmio_write_32(CRU_BASE + CRU_MODE_CON00, 0x03 << (16 + clk_pll * 2));
}

static void __dead2 soc_global_soft_reset(void)
{
	set_pll_slow_mode(CLK_CPLL);
	set_pll_slow_mode(CLK_GPLL);
	set_pll_slow_mode(CLK_NPLL);
	set_pll_slow_mode(CLK_VPLL);
	set_pll_slow_mode(CLK_USBPLL);
	set_pll_slow_mode(CLK_APLL);
	mmio_write_32(PMUCRU_BASE + PMUCRU_MODE_CON00, 0x000f0000);

	dsb();
	mmio_write_32(CRU_BASE + CRU_GLB_SRST_FST, GLB_SRST_FST_CFG_VAL);
	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to excute valid codes.
	 */
	while (1) {
		;
	}
}

static void rockchip_system_reset_init(void)
{
	mmio_write_32(GRF_BASE + 0x0508, 0x00100010);
	mmio_write_32(CRU_BASE + 0x00dc, 0x01030103);
}

void __dead2 rockchip_soc_soft_reset(void)
{
	soc_global_soft_reset();
}

void plat_rockchip_soc_init(void)
{
	rockchip_clock_init();
	secure_timer_init();
	sgrf_init();
	rockchip_system_reset_init();
	rockchip_init_scmi_server();
	NOTICE("BL31: Rockchip release version: v%d.%d\n",
		MAJOR_VERSION, MINOR_VERSION);
}

