/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define SOC_H

enum plls_id {
	APLL_ID = 0,
	DPLL_ID,
	CPLL_ID,
	GPLL_ID,
	NPLL_ID,
	END_PLL_ID,
};


#define CYCL_24M_CNT_US(us)	(24 * (us))
#define CYCL_24M_CNT_MS(ms)	((ms) * CYCL_24M_CNT_US(1000))

/*****************************************************************************
 * grf regs
 *****************************************************************************/
#define GRF_UOC0_CON0		0x320
#define GRF_UOC1_CON0		0x334
#define GRF_UOC2_CON0		0x348
#define GRF_SIDDQ		BIT(13)

/*****************************************************************************
 * cru reg, offset
 *****************************************************************************/
#define CRU_SOFTRST_CON		0x1b8
#define CRU_SOFTRSTS_CON(n)	(CRU_SOFTRST_CON + ((n) * 4))
#define CRU_SOFTRSTS_CON_CNT	11

#define RST_DMA1_MSK		0x4
#define RST_DMA2_MSK		0x1

#define CRU_CLKSEL_CON		0x60
#define CRU_CLKSELS_CON(i)	(CRU_CLKSEL_CON + ((i) * 4))
#define CRU_CLKSELS_CON_CNT	42

#define CRU_CLKGATE_CON		0x160
#define CRU_CLKGATES_CON(i)	(CRU_CLKGATE_CON + ((i) * 4))
#define CRU_CLKGATES_CON_CNT	18

#define CRU_GLB_SRST_FST	0x1b0
#define CRU_GLB_SRST_SND	0x1b4
#define CRU_GLB_RST_CON		0x1f0

#define CRU_CONS_GATEID(i)	(16 * (i))
#define GATE_ID(reg, bit)	(((reg) * 16) + (bit))

#define PMU_RST_MASK		0x3
#define PMU_RST_BY_FIRST_SFT	(0 << 2)
#define PMU_RST_BY_SECOND_SFT	(1 << 2)
#define PMU_RST_NOT_BY_SFT	(2 << 2)

/***************************************************************************
 * pll
 ***************************************************************************/
#define PLL_CON_COUNT		4
#define PLL_CONS(id, i)		((id) * 0x10 + ((i) * 4))
#define PLL_PWR_DN_MSK		BIT(1)
#define PLL_PWR_DN		REG_WMSK_BITS(1, 1, 0x1)
#define PLL_PWR_ON		REG_WMSK_BITS(0, 1, 0x1)
#define PLL_RESET		REG_WMSK_BITS(1, 5, 0x1)
#define PLL_RESET_RESUME	REG_WMSK_BITS(0, 5, 0x1)
#define PLL_BYPASS_MSK		BIT(0)
#define PLL_BYPASS_W_MSK	(PLL_BYPASS_MSK << 16)
#define PLL_BYPASS		REG_WMSK_BITS(1, 0, 0x1)
#define PLL_NO_BYPASS		REG_WMSK_BITS(0, 0, 0x1)

#define PLL_MODE_CON		0x50

struct deepsleep_data_s {
	uint32_t pll_con[END_PLL_ID][PLL_CON_COUNT];
	uint32_t pll_mode;
	uint32_t cru_sel_con[CRU_CLKSELS_CON_CNT];
	uint32_t cru_gate_con[CRU_CLKGATES_CON_CNT];
};

#define REG_W_MSK(bits_shift, msk) \
		((msk) << ((bits_shift) + 16))
#define REG_VAL_CLRBITS(val, bits_shift, msk) \
		((val) & (~((msk) << bits_shift)))
#define REG_SET_BITS(bits, bits_shift, msk) \
		(((bits) & (msk)) << (bits_shift))
#define REG_WMSK_BITS(bits, bits_shift, msk) \
		(REG_W_MSK(bits_shift, msk) | \
		REG_SET_BITS(bits, bits_shift, msk))
#define REG_SOC_WMSK		0xffff0000

#define regs_update_bit_set(addr, shift) \
		regs_update_bits((addr), 0x1, 0x1, (shift))
#define regs_update_bit_clr(addr, shift) \
		regs_update_bits((addr), 0x0, 0x1, (shift))

void regs_update_bits(uintptr_t addr, uint32_t val,
		      uint32_t mask, uint32_t shift);
void clk_plls_suspend(void);
void clk_plls_resume(void);
void clk_gate_con_save(void);
void clk_gate_con_disable(void);
void clk_gate_con_restore(void);
void clk_sel_con_save(void);
void clk_sel_con_restore(void);
#endif /* SOC_H */
