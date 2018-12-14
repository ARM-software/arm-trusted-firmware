/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <drivers/delay_timer.h>
#include <drivers/marvell/aro.h>
#include <lib/mmio.h>

#include <a8k_plat_def.h>

/* Notify bootloader on DRAM setup */
#define AP807_CPU_ARO_CTRL(cluster)	\
			(MVEBU_RFU_BASE + 0x82A8 + (0xA58 * (cluster)))

/* 0 - ARO clock is enabled, 1 - ARO clock is disabled */
#define AP807_CPU_ARO_CLK_EN_OFFSET	0
#define AP807_CPU_ARO_CLK_EN_MASK	(0x1 << AP807_CPU_ARO_CLK_EN_OFFSET)

/* 0 - ARO is the clock source, 1 - PLL is the clock source */
#define AP807_CPU_ARO_SEL_PLL_OFFSET	5
#define AP807_CPU_ARO_SEL_PLL_MASK	(0x1 << AP807_CPU_ARO_SEL_PLL_OFFSET)

/* AP807 clusters count */
#define AP807_CLUSTER_NUM		2

/* PLL frequency values */
#define PLL_FREQ_1200			0x2AE5F002 /* 1200 */
#define PLL_FREQ_2000			0x2FC9F002 /* 2000 */
#define PLL_FREQ_2200			0x2AC57001 /* 2200 */
#define PLL_FREQ_2400			0x2AE5F001 /* 2400 */

/* CPU PLL control registers */
#define AP807_CPU_PLL_CTRL(cluster)	\
			(MVEBU_RFU_BASE + 0x82E0 + (0x8 * (cluster)))

#define AP807_CPU_PLL_PARAM(cluster)	AP807_CPU_PLL_CTRL(cluster)
#define AP807_CPU_PLL_CFG(cluster)	(AP807_CPU_PLL_CTRL(cluster) + 0x4)
#define AP807_CPU_PLL_CFG_BYPASS_MODE	(0x1)
#define AP807_CPU_PLL_CFG_USE_REG_FILE	(0x1 << 9)

static void pll_set_freq(unsigned int freq_val)
{
	int i;

	for (i = 0 ; i < AP807_CLUSTER_NUM ; i++) {
		mmio_write_32(AP807_CPU_PLL_CFG(i),
			      AP807_CPU_PLL_CFG_USE_REG_FILE);
		mmio_write_32(AP807_CPU_PLL_CFG(i),
			      AP807_CPU_PLL_CFG_USE_REG_FILE |
			      AP807_CPU_PLL_CFG_BYPASS_MODE);
		mmio_write_32(AP807_CPU_PLL_PARAM(i), freq_val);
		mmio_write_32(AP807_CPU_PLL_CFG(i),
			      AP807_CPU_PLL_CFG_USE_REG_FILE);
	}
}

/* Switch to ARO from PLL in ap807 */
static void aro_to_pll(void)
{
	unsigned int reg;
	int i;

	for (i = 0 ; i < AP807_CLUSTER_NUM ; i++) {
		/* switch from ARO to PLL */
		reg = mmio_read_32(AP807_CPU_ARO_CTRL(i));
		reg |= AP807_CPU_ARO_SEL_PLL_MASK;
		mmio_write_32(AP807_CPU_ARO_CTRL(i), reg);

		mdelay(100);

		/* disable ARO clk driver */
		reg = mmio_read_32(AP807_CPU_ARO_CTRL(i));
		reg |= (AP807_CPU_ARO_CLK_EN_MASK);
		mmio_write_32(AP807_CPU_ARO_CTRL(i), reg);
	}
}

/* switch from ARO to PLL
 * in case of default frequency option, configure PLL registers
 * to be aligned with new default frequency.
 */
void ap807_clocks_init(unsigned int freq_option)
{
	/* Switch from ARO to PLL */
	aro_to_pll();

	/* Modifications in frequency table:
	 * 0x0: 764x: change to 2000 MHz.
	 * 0x2: 744x change to 1800 MHz, 764x change to 2200/2400.
	 * 0x3: 3900/744x/764x change to 1200 MHz.
	 */
	switch (freq_option) {
	case CPU_2000_DDR_1200_RCLK_1200:
		pll_set_freq(PLL_FREQ_2000);
		break;
	default:
		break;
	}
}
