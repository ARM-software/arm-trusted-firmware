/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <ddr_parameter.h>
#include <plat_private.h>
#include <secure.h>
#include <px30_def.h>

/**
 * There are 8 regions for DDR security control
 * @rgn - the DDR regions 0 ~ 7 which are can be configured.
 * @st - start address to set as secure
 * @sz - length of area to set as secure
 * The internal unit is megabytes, so memory areas need to be aligned
 * to megabyte borders.
 */
static void secure_ddr_region(uint32_t rgn,
			      uintptr_t st, size_t sz)
{
	uintptr_t ed = st + sz;
	uintptr_t st_mb, ed_mb;
	uint32_t val;

	assert(rgn <= 7);
	assert(st < ed);

	/* check aligned 1MB */
	assert(st % SIZE_M(1) == 0);
	assert(ed % SIZE_M(1) == 0);

	st_mb = st / SIZE_M(1);
	ed_mb = ed / SIZE_M(1);

	/* map top and base */
	mmio_write_32(FIREWALL_DDR_BASE +
		      FIREWALL_DDR_FW_DDR_RGN(rgn),
		      RG_MAP_SECURE(ed_mb, st_mb));

	/* enable secure */
	val = mmio_read_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_CON_REG);
	val |= BIT(rgn);
	mmio_write_32(FIREWALL_DDR_BASE +
		      FIREWALL_DDR_FW_DDR_CON_REG, val);
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

void sgrf_init(void)
{
#ifdef PLAT_RK_SECURE_DDR_MINILOADER
	uint32_t i;
	struct param_ddr_usage usg;

	/* general secure regions */
	usg = ddr_region_usage_parse(DDR_PARAM_BASE,
				     PLAT_MAX_DDR_CAPACITY_MB);

	/* region-0 for TF-A, region-1 for optional OP-TEE */
	assert(usg.s_nr < 7);

	for (i = 0; i < usg.s_nr; i++)
		secure_ddr_region(7 - i, usg.s_top[i], usg.s_base[i]);
#endif

	/* secure the trustzone ram */
	secure_ddr_region(0, TZRAM_BASE, TZRAM_SIZE);

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
