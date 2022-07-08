/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <lib/mmio.h>
#include <platform_def.h>

#define IMX_CCM_IP_BASE				(IMX_CCM_BASE + 0xa000)
#define DRAM_SEL_CFG				(IMX_CCM_BASE + 0x9800)
#define CCM_IP_CLK_ROOT_GEN_TAGET(i)		(IMX_CCM_IP_BASE + 0x80 * (i) + 0x00)
#define CCM_IP_CLK_ROOT_GEN_TAGET_SET(i)	(IMX_CCM_IP_BASE + 0x80 * (i) + 0x04)
#define CCM_IP_CLK_ROOT_GEN_TAGET_CLR(i)	(IMX_CCM_IP_BASE + 0x80 * (i) + 0x08)
#define PLL_FREQ_800M	U(0x00ece580)
#define PLL_FREQ_400M	U(0x00ec6984)
#define PLL_FREQ_167M	U(0x00f5a406)

void ddr_pll_bypass_100mts(void)
{
	/* change the clock source of dram_alt_clk_root to source 2 --100MHz */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(0), (0x7 << 24) | (0x7 << 16));
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(0), (0x2 << 24));

	/* change the clock source of dram_apb_clk_root to source 2 --40MHz/2 */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(1), (0x7 << 24) | (0x7 << 16));
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(1), (0x2 << 24) | (0x1 << 16));

	/* configure pll bypass mode */
	mmio_write_32(DRAM_SEL_CFG + 0x4, BIT(24));
}

void ddr_pll_bypass_400mts(void)
{
	/* change the clock source of dram_alt_clk_root to source 1 --400MHz */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(0), (0x7 << 24) | (0x7 << 16));
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(0), (0x1 << 24) | (0x1 << 16));

	/* change the clock source of dram_apb_clk_root to source 3 --160MHz/2 */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(1), (0x7 << 24) | (0x7 << 16));
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(1), (0x3 << 24) | (0x1 << 16));

	/* configure pll bypass mode */
	mmio_write_32(DRAM_SEL_CFG + 0x4, BIT(24));
}

void ddr_pll_unbypass(void)
{
	mmio_write_32(DRAM_SEL_CFG + 0x8, BIT(24));
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(1), (0x7 << 24) | (0x7 << 16));
	/* to source 4 --800MHz/5 */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(1), (0x4 << 24) | (0x4 << 16));
}

#if defined(PLAT_imx8mq)
void dram_pll_init(unsigned int drate)
{
	/* bypass the PLL */
	mmio_setbits_32(HW_DRAM_PLL_CFG0, 0x30);

	switch (drate) {
	case 3200:
		mmio_write_32(HW_DRAM_PLL_CFG2, PLL_FREQ_800M);
		break;
	case 1600:
		mmio_write_32(HW_DRAM_PLL_CFG2, PLL_FREQ_400M);
		break;
	case 667:
		mmio_write_32(HW_DRAM_PLL_CFG2, PLL_FREQ_167M);
		break;
	default:
		break;
	}

	/* unbypass the PLL */
	mmio_clrbits_32(HW_DRAM_PLL_CFG0, 0x30);
	while (!(mmio_read_32(HW_DRAM_PLL_CFG0) & (1 << 31))) {
		;
	}
}
#else
void dram_pll_init(unsigned int drate)
{
	/* bypass the PLL */
	mmio_setbits_32(DRAM_PLL_CTRL, (1 << 16));
	mmio_clrbits_32(DRAM_PLL_CTRL, (1 << 9));

	switch (drate) {
	case 2400:
		mmio_write_32(DRAM_PLL_CTRL + 0x4, (300 << 12) | (3 << 4) | 2);
		break;
	case 1600:
		mmio_write_32(DRAM_PLL_CTRL + 0x4, (400 << 12) | (3 << 4) | 3);
		break;
	case 1066:
		mmio_write_32(DRAM_PLL_CTRL + 0x4, (266 << 12) | (3 << 4) | 3);
		break;
	case 667:
		mmio_write_32(DRAM_PLL_CTRL + 0x4, (334 << 12) | (3 << 4) | 4);
		break;
	default:
		break;
	}

	mmio_setbits_32(DRAM_PLL_CTRL, BIT(9));
	/* wait for PLL locked */
	while (!(mmio_read_32(DRAM_PLL_CTRL) & BIT(31))) {
		;
	}

	/* unbypass the PLL */
	mmio_clrbits_32(DRAM_PLL_CTRL, BIT(16));
}
#endif

/* change the dram clock frequency */
void dram_clock_switch(unsigned int target_drate, bool bypass_mode)
{
	if (bypass_mode) {
		switch (target_drate) {
		case 400:
			ddr_pll_bypass_400mts();
			break;
		case 100:
			ddr_pll_bypass_100mts();
			break;
		default:
			ddr_pll_unbypass();
			break;
		}
	} else {
		dram_pll_init(target_drate);
	}
}
