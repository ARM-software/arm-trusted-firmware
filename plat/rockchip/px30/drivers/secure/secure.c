/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ddr_parameter.h>
#include <secure.h>
#include <px30_def.h>

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
