/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <lib/mmio.h>

#include <platform_def.h>

#include <secure.h>
#include <soc.h>

static void secure_fw_master_init(void)
{
	uint32_t i;

	/* ddr_mcu can access all ddr-regions */
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_MST(1), 0x0000ffff);
	/* dcf/crypto_s can access all ddr-regions */
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_MST(14), 0x00000000);
	/* dsu_mp_sec can access all ddr-regions.
	 * DSU access memory [f000_0000~ff00_0000] through MP in firewall_ddr.
	 */
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_MST(36), 0xffff0000);

	/* all other ns-master can't access all ddr-regions */
	for (i = 0; i < FIREWALL_DDR_MST_CNT; i++) {
		if (i == 1 || i == 14 || i == 36)
			continue;

		mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_MST(i), 0xffffffff);
	}

	/* mcu_pmu can access all sram-regions */
	mmio_write_32(FIREWALL_SYSMEM_BASE + FIREWALL_SYSMEM_MST(19), 0x000000ff);
	/* dsu mp-sec can access all sram-regions */
	mmio_write_32(FIREWALL_SYSMEM_BASE + FIREWALL_SYSMEM_MST(38), 0x000000ff);
	/* nsp_dsu2main_sec can access all sram-regions */
	mmio_write_32(FIREWALL_SYSMEM_BASE + FIREWALL_SYSMEM_MST(41), 0x00000000);

	/* all ns-master can't access all sram-regions */
	for (i = 0; i < FIREWALL_SYSMEM_MST_CNT; i++) {
		if (i == 19 || i == 38 || i == 41)
			continue;

		mmio_write_32(FIREWALL_SYSMEM_BASE + FIREWALL_SYSMEM_MST(i),
			      0x00ff00ff);
	}

	/* dsu-ns can't access all ddr-regions, dsu-s can access all ddr-regions */
	mmio_write_32(FIREWALL_DSU_BASE + FIREWALL_DSU_MST(0), 0xffffffff);
	mmio_write_32(FIREWALL_DSU_BASE + FIREWALL_DSU_MST(1), 0x00000000);
	dsb();
	isb();
}

/* unit: Mb */
static void dsu_fw_rgn_config(uint64_t base_mb, uint64_t top_mb, int rgn_id)
{
	int i;

	if (rgn_id >= FIREWALL_DSU_RGN_CNT || rgn_id < 0) {
		ERROR("%s regions-id:%d is invalid!\n", __func__, rgn_id);
		panic();
	}

	mmio_write_32(FIREWALL_DSU_BASE + FIREWALL_DSU_RGN(rgn_id),
		      RG_MAP_SECURE(top_mb, base_mb));

	for (i = 0; i < DDR_CHN_CNT; i++)
		mmio_setbits_32(FIREWALL_DSU_BASE + FIREWALL_DSU_CON(i),
				BIT(rgn_id));
}

/* unit: Mb */
static void ddr_fw_rgn_config(uint64_t base_mb, uint64_t top_mb, int rgn_id)
{
	if (rgn_id >= FIREWALL_DDR_RGN_CNT || rgn_id < 0) {
		ERROR("%s regions-id:%d is invalid!\n", __func__, rgn_id);
		panic();
	}

	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_RGN(rgn_id),
		      RG_MAP_SECURE(top_mb, base_mb));

	/* enable region */
	mmio_setbits_32(FIREWALL_DDR_BASE + FIREWALL_DDR_CON,
			BIT(rgn_id));
}

/* Unit: Kb */
static void sram_fw_rgn_config(uint64_t base_kb, uint64_t top_kb, int rgn_id)
{
	if (rgn_id >= FIREWALL_SYSMEM_RGN_CNT || rgn_id < 0) {
		ERROR("%s regions-id:%d is invalid!\n", __func__, rgn_id);
		panic();
	}

	mmio_write_32(FIREWALL_SYSMEM_BASE + FIREWALL_SYSMEM_RGN(rgn_id),
		      RG_MAP_SRAM_SECURE(top_kb, base_kb));

	/* enable region */
	mmio_setbits_32(FIREWALL_SYSMEM_BASE + FIREWALL_SYSMEM_CON, BIT(rgn_id));
}

static void secure_region_init(void)
{
	uint32_t i;

	/* disable all region first except region0 */
	mmio_clrbits_32(FIREWALL_DDR_BASE + FIREWALL_DDR_CON, 0xfffe);
	for (i = 0; i < FIREWALL_DSU_CON_CNT; i++)
		mmio_clrbits_32(FIREWALL_DSU_BASE + FIREWALL_DSU_CON(i), 0xfffe);
	mmio_clrbits_32(FIREWALL_SYSMEM_BASE + FIREWALL_SYSMEM_CON, 0xfe);

	secure_fw_master_init();

	/* Use FW_DDR_RGN0_REG to config 0~1M space to secure */
	dsu_fw_rgn_config(0, 1, 0);
	ddr_fw_rgn_config(0, 1, 0);

	/* Use FIREWALL_SYSMEM_RGN0 to config SRAM_ENTRY code(0~4k of sram) to secure */
	sram_fw_rgn_config(0, 4, 0);
	/* For 0xffff0000~0xffffffff, use FIREWALL_SYSMEM_RGN7 to config
	 * 960~1024k of sram to secure.
	 */
	sram_fw_rgn_config(960, 1024, 7);
}

void secure_timer_init(void)
{
	/* gpu's cntvalue comes from stimer1 channel_5 */
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_CONTROL_REG,
		      TIMER_DIS);

	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_LOAD_COUNT0, 0xffffffff);
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_LOAD_COUNT1, 0xffffffff);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_CONTROL_REG,
		      TIMER_EN | TIMER_FMODE);
}

void sgrf_init(void)
{
	uint32_t i;

	secure_region_init();

	/* config master ddr_mcu_prot|dcf_wr|dcf_rd as secure */
	mmio_write_32(BUSSGRF_BASE + SGRF_SOC_CON(14), 0x001f0011);
	mmio_write_32(BUSSGRF_BASE + SGRF_SOC_CON(15), 0xffffffff);
	mmio_write_32(BUSSGRF_BASE + SGRF_SOC_CON(16), 0x03ff03ff);

	/* config slave mailbox_mcu_ddr as secure */
	mmio_write_32(BUSSGRF_BASE + SGRF_FIREWALL_CON(4), 0xffff2000);
	/* config slave int256mux4_mcu_ddr|int256mux4_mcu_pmu as secure */
	mmio_write_32(BUSSGRF_BASE + SGRF_FIREWALL_CON(5), 0xffff0060);
	/* config slave ddrgrf*|dma2ddr|ddrphy*_cru|umctl* as secure */
	mmio_write_32(BUSSGRF_BASE + SGRF_FIREWALL_CON(24), 0xffff0fbf);
	/* config slave ddrphy*|ddr_stanby*|ddr_mcu_timer|ddr_mcu_wdt as secure */
	mmio_write_32(BUSSGRF_BASE + SGRF_FIREWALL_CON(25), 0xffff03ff);

	/* config all other slave as ns */
	for (i = 0; i < SGRF_FIREWALL_CON_CNT; i++) {
		if (i == 4 || i == 5 || i == 24 || i == 25)
			continue;

		mmio_write_32(BUSSGRF_BASE + SGRF_FIREWALL_CON(i), 0xffff0000);
	}

	/* config vad_hprot non-secure, pmu_mcu_hprot as secure */
	mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(0), 0x00180010);
	/* config pmu1, pmu0, pmu_sram as secure */
	mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(1), 0xefbe6020);
	/* config remap_pmu_mem, h_pmu_mem as secure */
	mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(2), 0x01f900c0);

	/* disable dp encryption */
	mmio_write_32(BUSSGRF_BASE + SGRF_SOC_CON(13), 0x00180018);

	/* select grf config for pcie ats */
	mmio_write_32(BUSSGRF_BASE + SGRF_SOC_CON(17), 0x11111111);
	mmio_write_32(BUSSGRF_BASE + SGRF_SOC_CON(18), 0x11111111);
	mmio_write_32(BUSSGRF_BASE + SGRF_SOC_CON(19), 0x00110011);
}
