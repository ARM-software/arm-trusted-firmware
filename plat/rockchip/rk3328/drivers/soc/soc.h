/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define SOC_H

/******************************* stimer ***************************************/
#define TIMER_LOADE_COUNT0	0x00
#define TIMER_LOADE_COUNT1	0x04
#define TIMER_CURRENT_VALUE0	0x08
#define TIMER_CURRENT_VALUE1	0x0C
#define TIMER_CONTROL_REG	0x10
#define TIMER_INTSTATUS		0x18
#define TIMER_EN		0x1

/**************************** read/write **************************************/
#ifndef BITS_WMSK
#define BITS_WMSK(msk, shift)	((msk) << (shift + REG_MSK_SHIFT))
#endif

/**************************** cru *********************************************/
enum plls_id {
	APLL_ID = 0,
	DPLL_ID,
	CPLL_ID,
	GPLL_ID,
	RESERVE,
	NPLL_ID,
	MAX_PLL,
};

#define CRU_CRU_MODE		0x0080
#define CRU_CRU_MISC		0x0084
#define CRU_GLB_SRST_FST	0x009c
#define CRU_GLB_SRST_FST_VALUE	0xfdb9
#define PLL_CONS(id, i)		(0x020 * (id) + ((i) * 4))
#define CRU_CLKSEL_CON(i)	(0x100 + ((i) * 4))
#define CRU_CLKSEL_NUMS		53
#define CRU_CLKGATE_CON(i)	(0x200 + ((i) * 4))
#define CRU_CLKGATE_NUMS	29
#define CRU_SOFTRSTS_CON(n)	(0x300 + ((n) * 4))
#define CRU_SOFTRSTS_NUMS	12
#define CRU_PLL_CON_NUMS	5

/* PLLn_CON1 */
#define PLL_IS_LOCKED		BIT(10)
/* PLLn_CON0 */
#define PLL_BYPASS		BITS_WITH_WMASK(1, 0x1, 15)
#define PLL_NO_BYPASS		BITS_WITH_WMASK(0, 0x1, 15)

/* CRU_MODE */
static inline uint32_t rk3328_pll_mode(uint32_t pll_id, uint32_t mode)
{
	uint32_t shift = pll_id * 4U;

	if (pll_id == NPLL_ID) {
		shift = 1U;
	}

	return BITS_WITH_WMASK(mode, 1U, shift);
}

static inline uint32_t rk3328_pll_mode_slow(uint32_t pll_id)
{
	return rk3328_pll_mode(pll_id, 0U);
}

static inline uint32_t rk3328_pll_mode_normal(uint32_t pll_id)
{
	return rk3328_pll_mode(pll_id, 1U);
}

static inline bool rk3328_pll_is_mode_normal(uint32_t mode_val, uint32_t pll_id)
{
	uint32_t mode_normal = rk3328_pll_mode_normal(pll_id);
	uint32_t mask = 0xffff;

	return mode_val & mode_normal & mask;
}

#define CRU_GATEID_CONS(ID)	(0x200 + (ID / 16) * 4)
#define CRU_CONS_GATEID(i)	(16 * (i))
#define GATE_ID(reg, bit)	((reg * 16) + bit)

#define PLL_LOCKED_TIMEOUT 600000U

#define STIMER_CHN_BASE(n)	(STIME_BASE + 0x20 * (n))
/************************** config regs ***************************************/
#define FIREWALL_CFG_FW_SYS_CON(n)	(0x000 + (n) * 4)
#define FIREWALL_DDR_FW_DDR_RGN(n)	(0x000 + (n) * 4)
#define FIREWALL_DDR_FW_DDR_MST(n)	(0x020 + (n) * 4)
#define FIREWALL_DDR_FW_DDR_CON_REG	(0x040)
#define GRF_SOC_CON(n)			(0x400 + (n) * 4)
#define GRF_SOC_STATUS(n)		(0x480 + (n) * 4)
#define GRF_CPU_STATUS(n)		(0x520 + (n) * 4)
#define GRF_OS_REG(n)			(0x5c8 + (n) * 4)
#define DDRGRF_SOC_CON(n)		(0x000 + (n) * 4)
#define DDRGRF_SOC_STATUS(n)		(0x100 + (n) * 4)
#define SGRF_SOC_CON(n)			(0x000 + (n) * 4)
#define SGRF_DMAC_CON(n)		(0x100 + (n) * 4)
#define SGRF_HDCP_KEY_CON(n)		(0x280 + (n) * 4)

#define DDR_PCTL2_PWRCTL		0x30
/************************** regs func *****************************************/
#define STIMER_S			BIT(23)
#define SGRF_SLV_S_ALL_NS		0x0
#define SGRF_MST_S_ALL_NS		0xffffffff
#define DMA_IRQ_BOOT_NS			0xffffffff
#define DMA_MANAGER_BOOT_NS		0x80008000
#define DMA_PERI_CH_NS_15_0		0xffffffff
#define DMA_PERI_CH_NS_19_16		0x000f000f
#define DMA_SOFTRST_REQ			0x01000100
#define DMA_SOFTRST_RLS			0x01000000

#define SELFREF_EN			BIT(0)
/************************** cpu ***********************************************/
#define CPU_BOOT_ADDR_WMASK		0xffff0000
#define CPU_BOOT_ADDR_ALIGN		16

/************************** ddr secure region *********************************/
#define PLAT_MAX_DDR_CAPACITY_MB	4096
#define RG_MAP_SECURE(top, base)	((((top) - 1) << 16) | (base))

/************************** gpio2_d2 ******************************************/
#define SWPORTA_DR		0x00
#define SWPORTA_DDR		0x04
#define GPIO2_D2		BIT(26)
#define GPIO2_D2_GPIO_MODE	0x30
#define GRF_GPIO2D_IOMUX	0x34

#endif /* SOC_H */
