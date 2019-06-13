/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <ddr_parameter.h>
#include <platform_def.h>
#include <pmu.h>
#include <px30_def.h>
#include <soc.h>
#include <rockchip_sip_svc.h>

/* Aggregate of all devices in the first GB */
#define PX30_DEV_RNG0_BASE	0xff000000
#define PX30_DEV_RNG0_SIZE	0x00ff0000

const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(PX30_DEV_RNG0_BASE, PX30_DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SHARE_MEM_BASE, SHARE_MEM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DDR_PARAM_BASE, DDR_PARAM_SIZE,
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

void clk_gate_con_save(uint32_t *clkgt_save)
{
	uint32_t i, j;

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++)
		clkgt_save[i] =
			mmio_read_32(CRU_BASE + CRU_CLKGATES_CON(i));
	j = i;
	for (i = 0; i < CRU_PMU_CLKGATE_CON_CNT; i++, j++)
		clkgt_save[j] =
			mmio_read_32(PMUCRU_BASE + CRU_PMU_CLKGATES_CON(i));
}

void clk_gate_con_restore(uint32_t *clkgt_save)
{
	uint32_t i, j;

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++)
		mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(i),
			      WITH_16BITS_WMSK(clkgt_save[i]));

	j = i;
	for (i = 0; i < CRU_PMU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(PMUCRU_BASE + CRU_PMU_CLKGATES_CON(i),
			      WITH_16BITS_WMSK(clkgt_save[j]));
}

void clk_gate_con_disable(void)
{
	uint32_t i;

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++)
		mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(i),
			      0xffff0000);

	for (i = 0; i < CRU_PMU_CLKGATE_CON_CNT; i++)
		mmio_write_32(PMUCRU_BASE + CRU_PMU_CLKGATES_CON(i),
			      0xffff0000);
}

void secure_timer_init(void)
{
	mmio_write_32(STIMER_CHN_BASE(1) + TIMER_CONTROL_REG,
		      TIMER_DIS);

	mmio_write_32(STIMER_CHN_BASE(1) + TIMER_LOAD_COUNT0, 0xffffffff);
	mmio_write_32(STIMER_CHN_BASE(1) + TIMER_LOAD_COUNT1, 0xffffffff);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER_CHN_BASE(1) + TIMER_CONTROL_REG,
		      TIMER_EN | TIMER_FMODE);
}

static void sgrf_init(void)
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
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(4), SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(5), SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6), SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(7), SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(8), 0x00030000);

	/* set master crypto to no-secure, dcf to secure */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(3), 0x000f0003);

	/* set DMAC into no-secure */
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(0), DMA_IRQ_BOOT_NS);
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(1), DMA_PERI_CH_NS_15_0);
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(2), DMA_PERI_CH_NS_19_16);
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(3), DMA_MANAGER_BOOT_NS);

	/* soft reset dma before use */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1), DMA_SOFTRST_REQ);
	udelay(5);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1), DMA_SOFTRST_RLS);
}

static void soc_reset_config_all(void)
{
	uint32_t tmp;

	/* tsadc and wdt can trigger a first rst */
	tmp = mmio_read_32(CRU_BASE + CRU_GLB_RST_CON);
	tmp |= CRU_GLB_RST_TSADC_FST | CRU_GLB_RST_WDT_FST;
	mmio_write_32(CRU_BASE + CRU_GLB_RST_CON, tmp);
	return;
	tmp = mmio_read_32(PMUGRF_BASE + PMUGRF_SOC_CON(3));
	tmp &= ~(PMUGRF_FAILSAFE_SHTDN_TSADC | PMUGRF_FAILSAFE_SHTDN_WDT);
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(3), tmp);

	/* wdt pin rst eable */
	mmio_write_32(GRF_BASE + GRF_SOC_CON(2),
		      BIT_WITH_WMSK(GRF_SOC_CON2_NSWDT_RST_EN));
}

void px30_soc_reset_config(void)
{
	uint32_t tmp;

	/* enable soc ip rst hold time cfg */
	tmp = mmio_read_32(CRU_BASE + CRU_GLB_RST_CON);
	tmp |= BIT(CRU_GLB_RST_TSADC_EXT) | BIT(CRU_GLB_RST_WDT_EXT);
	mmio_write_32(CRU_BASE + CRU_GLB_RST_CON, tmp);
	/* soc ip rst hold time, 24m */
	tmp = mmio_read_32(CRU_BASE + CRU_GLB_CNT_TH);
	tmp &= ~CRU_GLB_CNT_RST_MSK;
	tmp |= (CRU_GLB_CNT_RST_1MS / 2);
	mmio_write_32(CRU_BASE + CRU_GLB_CNT_TH, tmp);

	mmio_write_32(PMUSGRF_BASE + PMUSGRF_SOC_CON(0),
		      BIT_WITH_WMSK(PMUSGRF_RSTOUT_FST) |
		      BIT_WITH_WMSK(PMUSGRF_RSTOUT_TSADC) |
		      BIT_WITH_WMSK(PMUSGRF_RSTOUT_WDT));

	/* rst_out pulse time */
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(2),
		      PMUGRF_SOC_CON2_MAX_341US | PMUGRF_SOC_CON2_US_WMSK);

	soc_reset_config_all();
}

void plat_rockchip_soc_init(void)
{
	secure_timer_init();
	sgrf_init();
}
