/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define SOC_H

enum plls_id {
	ABPLL_ID = 0,
	ALPLL_ID,
	DPLL_ID,
	CPLL_ID,
	GPLL_ID,
	NPLL_ID,
	END_PLL_ID,
};

/*****************************************************************************
 * secure timer
 *****************************************************************************/
#define TIMER_LOADE_COUNT0	0x00
#define TIMER_LOADE_COUNT1	0x04
#define TIMER_CURRENT_VALUE0	0x08
#define TIMER_CURRENT_VALUE1	0x0C
#define TIMER_CONTROL_REG	0x10
#define TIMER_INTSTATUS		0x18

#define TIMER_EN		0x1

#define STIMER1_BASE		(STIME_BASE + 0x20)

#define CYCL_24M_CNT_US(us)	(24 * us)
#define CYCL_24M_CNT_MS(ms)	(ms * CYCL_24M_CNT_US(1000))

/*****************************************************************************
 * sgrf reg, offset
 *****************************************************************************/
#define SGRF_SOC_CON(n)		(0x0 + (n) * 4)
#define SGRF_BUSDMAC_CON(n)	(0x100 + (n) * 4)

#define SGRF_SOC_CON_NS		0xffff0000

/*****************************************************************************
 * con6[2]pmusram is security.
 * con6[6]stimer is security.
 *****************************************************************************/
#define PMUSRAM_S_SHIFT		2
#define PMUSRAM_S		1
#define STIMER_S_SHIFT		6
#define STIMER_S		1
#define SGRF_SOC_CON7_BITS	((0xffffu << 16) | \
				 (PMUSRAM_S << PMUSRAM_S_SHIFT) | \
				 (STIMER_S << STIMER_S_SHIFT))

#define SGRF_BUSDMAC_CON0_NS	0xfffcfff8
#define SGRF_BUSDMAC_CON1_NS	0xffff0fff

/*
 * sgrf_soc_con1~2, mask and offset
 */
#define CPU_BOOT_ADDR_WMASK	0xffff0000
#define CPU_BOOT_ADDR_ALIGN	16

/*****************************************************************************
 * cru reg, offset
 *****************************************************************************/
#define CRU_SOFTRST_CON		0x300
#define CRU_SOFTRSTS_CON(n)	(CRU_SOFTRST_CON + ((n) * 4))
#define CRU_SOFTRSTS_CON_CNT	15

#define SOFTRST_DMA1		0x40004
#define SOFTRST_DMA2		0x10001

#define RST_DMA1_MSK		0x4
#define RST_DMA2_MSK		0x0

#define CRU_CLKSEL_CON		0x100
#define CRU_CLKSELS_CON(i)	(CRU_CLKSEL_CON + ((i) * 4))
#define CRU_CLKSEL_CON_CNT	56

#define CRU_CLKGATE_CON		0x200
#define CRU_CLKGATES_CON(i)	(CRU_CLKGATE_CON + ((i) * 4))
#define CRU_CLKGATES_CON_CNT	25

#define CRU_GLB_SRST_FST	0x280
#define CRU_GLB_SRST_SND	0x284
#define CRU_GLB_RST_CON		0x388

#define CRU_CONS_GATEID(i)	(16 * (i))
#define GATE_ID(reg, bit)	((reg * 16) + bit)

#define PMU_RST_BY_SECOND_SFT	(BIT(1) << 2)
#define PMU_RST_NOT_BY_SFT	(BIT(1) << 2)

/***************************************************************************
 * pll
 ***************************************************************************/
#define PLL_PWR_DN_MSK		(0x1 << 1)
#define PLL_PWR_DN		REG_WMSK_BITS(1, 1, 0x1)
#define PLL_PWR_ON		REG_WMSK_BITS(0, 1, 0x1)
#define PLL_RESET		REG_WMSK_BITS(1, 5, 0x1)
#define PLL_RESET_RESUME	REG_WMSK_BITS(0, 5, 0x1)
#define PLL_BYPASS_MSK		(0x1 << 0)
#define PLL_BYPASS_W_MSK	(PLL_BYPASS_MSK << 16)
#define PLL_BYPASS		REG_WMSK_BITS(1, 0, 0x1)
#define PLL_NO_BYPASS		REG_WMSK_BITS(0, 0, 0x1)
#define PLL_MODE_SHIFT		8
#define PLL_MODE_MSK		0x3
#define PLLS_MODE_WMASK		(PLL_MODE_MSK << (16 + PLL_MODE_SHIFT))
#define PLL_SLOW		0x0
#define PLL_NORM		0x1
#define PLL_DEEP		0x2
#define PLL_SLOW_BITS		REG_WMSK_BITS(PLL_SLOW, 8, 0x3)
#define PLL_NORM_BITS		REG_WMSK_BITS(PLL_NORM, 8, 0x3)
#define PLL_DEEP_BITS		REG_WMSK_BITS(PLL_DEEP, 8, 0x3)

#define PLL_CONS(id, i)		((id) * 0x10 + ((i) * 4))

#define REG_W_MSK(bits_shift, msk) \
		((msk) << ((bits_shift) + 16))
#define REG_VAL_CLRBITS(val, bits_shift, msk) \
		(val & (~(msk << bits_shift)))
#define REG_SET_BITS(bits, bits_shift, msk) \
		(((bits) & (msk)) << (bits_shift))
#define REG_WMSK_BITS(bits, bits_shift, msk) \
		(REG_W_MSK(bits_shift, msk) | \
		REG_SET_BITS(bits, bits_shift, msk))

#define regs_updata_bit_set(addr, shift) \
		regs_updata_bits((addr), 0x1, 0x1, (shift))
#define regs_updata_bit_clr(addr, shift) \
		regs_updata_bits((addr), 0x0, 0x1, (shift))

void regs_updata_bits(uintptr_t addr, uint32_t val,
		      uint32_t mask, uint32_t shift);
void soc_sleep_config(void);
void pm_plls_resume(void);

#endif /* SOC_H */
