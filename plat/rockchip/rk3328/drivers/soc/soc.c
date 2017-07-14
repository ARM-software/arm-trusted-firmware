/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <console.h>
#include <ddr_parameter.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <plat_private.h>
#include <platform_def.h>
#include <rk3328_def.h>
#include <soc.h>

/* Table of regions to map using the MMU. */
const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(UART2_BASE, UART2_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMU_BASE, PMU_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SGRF_BASE, SGRF_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GPIO0_BASE, GPIO0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GPIO1_BASE, GPIO1_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GPIO2_BASE, GPIO2_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GPIO3_BASE, GPIO3_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CRU_BASE, CRU_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GRF_BASE, GRF_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(FIREWALL_DDR_BASE, FIREWALL_DDR_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(FIREWALL_CFG_BASE, FIREWALL_CFG_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(STIME_BASE, STIME_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GIC400_BASE, GIC400_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMUSRAM_BASE, PMUSRAM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SHARE_MEM_BASE, SHARE_MEM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_GRF_BASE, DDR_GRF_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_UPCTL_BASE, DDR_UPCTL_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PWM_BASE, PWM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_PARAM_BASE, DDR_PARAM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(EFUSE8_BASE, EFUSE8_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(EFUSE32_BASE, EFUSE32_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_PHY_BASE, DDR_PHY_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SERVER_MSCH_BASE, SERVER_MSCH_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_MONITOR_BASE, DDR_MONITOR_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(VOP_BASE, VOP_SIZE,
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

void secure_timer_init(void)
{
	mmio_write_32(STIMER_CHN_BASE(1) + TIMER_LOADE_COUNT0, 0xffffffff);
	mmio_write_32(STIMER_CHN_BASE(1) + TIMER_LOADE_COUNT1, 0xffffffff);
	/* auto reload & enable the timer */
	mmio_write_32(STIMER_CHN_BASE(1) + TIMER_CONTROL_REG, TIMER_EN);
}

void sgrf_init(void)
{
	uint32_t i, val;
	struct param_ddr_usage usg;

	/* general secure regions */
	usg = ddr_region_usage_parse(DDR_PARAM_BASE,
				     PLAT_MAX_DDR_CAPACITY_MB);
	for (i = 0; i < usg.s_nr; i++) {
		/* enable secure */
		val = mmio_read_32(FIREWALL_DDR_BASE +
			      FIREWALL_DDR_FW_DDR_CON_REG);
		val |= BIT(7 - i);
		mmio_write_32(FIREWALL_DDR_BASE +
			      FIREWALL_DDR_FW_DDR_CON_REG, val);
		/* map top and base */
		mmio_write_32(FIREWALL_DDR_BASE +
			      FIREWALL_DDR_FW_DDR_RGN(7 - i),
			      RG_MAP_SECURE(usg.s_top[i], usg.s_base[i]));
	}

	/* set ddr rgn0_top and rga0_top as 0 */
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_RGN(0), 0x0);

	/* set all slave ip into no-secure, except stimer */
	mmio_write_32(FIREWALL_CFG_BASE + FIREWALL_CFG_FW_SYS_CON(0),
		      SGRF_SLV_S_ALL_NS);
	mmio_write_32(FIREWALL_CFG_BASE + FIREWALL_CFG_FW_SYS_CON(1),
		      SGRF_SLV_S_ALL_NS);
	mmio_write_32(FIREWALL_CFG_BASE + FIREWALL_CFG_FW_SYS_CON(2),
		      SGRF_SLV_S_ALL_NS | STIMER_S);
	mmio_write_32(FIREWALL_CFG_BASE + FIREWALL_CFG_FW_SYS_CON(3),
		      SGRF_SLV_S_ALL_NS);

	/* set all master ip into no-secure */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(2), 0xf0000000);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(3), SGRF_MST_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(4), SGRF_MST_S_ALL_NS);

	/* set DMAC into no-secure */
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(3), DMA_IRQ_BOOT_NS);
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(4), DMA_PERI_CH_NS_15_0);
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(5), DMA_PERI_CH_NS_19_16);
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(5), DMA_MANAGER_BOOT_NS);

	/* soft reset dma before use */
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(3), DMA_SOFTRST_REQ);
	udelay(5);
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(3), DMA_SOFTRST_RLS);
}

void plat_rockchip_soc_init(void)
{
	secure_timer_init();
	sgrf_init();

	NOTICE("BL31:Rockchip release version: v%d.%d\n",
	       MAJOR_VERSION, MINOR_VERSION);
}
