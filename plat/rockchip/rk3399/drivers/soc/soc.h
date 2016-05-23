/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SOC_H__
#define __SOC_H__

#define GLB_SRST_FST_CFG_VAL	0xfdb9
#define GLB_SRST_SND_CFG_VAL	0xeca8

#define PMUCRU_PPLL_CON_OFFSET		0x000
#define PMUCRU_PPLL_CON_BASE_ADDR	(PMUCRU_BASE + PMUCRU_PPLL_CON_OFFSET)
#define PMUCRU_PPLL_CON_CONUT		0x06

#define PMUCRU_PPLL_CON(num)		(PMUCRU_PPLL_CON_BASE_ADDR + num * 4)
#define CRU_PLL_CON(pll_id, num)	(CRU_BASE + pll_id  * 0x20 + num * 4)
#define PLL_MODE_MSK			0x03
#define PLL_MODE_SHIFT			0x08
#define PLL_BYPASS_MSK			0x01
#define PLL_BYPASS_SHIFT		0x01
#define PLL_PWRDN_MSK			0x01
#define PLL_PWRDN_SHIFT			0x0
#define PLL_BYPASS			BIT(1)
#define PLL_PWRDN			BIT(0)

#define NO_PLL_BYPASS			(0x00)
#define NO_PLL_PWRDN			(0x00)

#define PLL_SLOW_MODE		BITS_WITH_WMASK(SLOW_MODE,\
						PLL_MODE_MSK, PLL_MODE_SHIFT)
#define PLL_BYPASS_MODE		BITS_WITH_WMASK(PLL_BYPASS,\
						PLL_BYPASS_MSK,\
						PLL_BYPASS_SHIFT)
#define PLL_NO_BYPASS_MODE	BITS_WITH_WMASK(NO_PLL_BYPASS,\
						PLL_BYPASS_MSK,\
						PLL_BYPASS_SHIFT)
#define PLL_NOMAL_MODE		BITS_WITH_WMASK(NORMAL_MODE,\
						PLL_MODE_MSK, PLL_MODE_SHIFT)

#define PLL_CON_COUNT			0x06
#define CRU_CLKSEL_COUNT		0x108
#define CRU_CLKSEL_OFFSET		0x300

#define PMUCRU_CLKSEL_CONUT		0x06
#define PMUCRU_CLKSEL_OFFSET		0x080
#define REG_SIZE			0x04
#define REG_SOC_WMSK			0xffff0000

#define CLK_GATE_MASK			0x01

enum plls_id {
	ALPLL_ID = 0,
	ABPLL_ID,
	DPLL_ID,
	CPLL_ID,
	GPLL_ID,
	NPLL_ID,
	VPLL_ID,
	PPLL_ID,
	END_PLL_ID,
};

enum pll_work_mode {
	SLOW_MODE = 0x00,
	NORMAL_MODE = 0x01,
	DEEP_SLOW_MODE = 0x02,
};

enum glb_sft_reset {
	PMU_RST_BY_FIRST_SFT,
	PMU_RST_BY_SECOND_SFT = BIT(2),
	PMU_RST_NOT_BY_SFT = BIT(3),
};

struct deepsleep_data_s {
	uint32_t plls_con[END_PLL_ID][PLL_CON_COUNT];
	uint32_t pmucru_clksel_con[PMUCRU_CLKSEL_CONUT];
	uint32_t cru_clksel_con[CRU_CLKSEL_COUNT];
};

#define CYCL_24M_CNT_US(us)	(24 * us)
#define CYCL_24M_CNT_MS(ms)	(ms * CYCL_24M_CNT_US(1000))

/**************************************************
 * secure timer
 **************************************************/

/* chanal0~5 */
#define STIMER0_CHN_BASE(n)	(STIME_BASE + 0x20 * (n))
/* chanal6~11 */
#define STIMER1_CHN_BASE(n)	(STIME_BASE + 0x8000 + 0x20 * (n))

 /* low 32 bits */
#define TIMER_END_COUNT0	0x00
 /* high 32 bits */
#define TIMER_END_COUNT1	0x04

#define TIMER_CURRENT_VALUE0	0x08
#define TIMER_CURRENT_VALUE1	0x0C

 /* low 32 bits */
#define TIMER_INIT_COUNT0	0x10
 /* high 32 bits */
#define TIMER_INIT_COUNT1	0x14

#define TIMER_INTSTATUS		0x18
#define TIMER_CONTROL_REG	0x1c

#define TIMER_EN			0x1

#define TIMER_FMODE		(0x0 << 1)
#define TIMER_RMODE		(0x1 << 1)

/**************************************************
 * cru reg, offset
 **************************************************/
#define CRU_SOFTRST_CON(n)	(0x400 + (n) * 4)

#define CRU_DMAC0_RST		BIT_WITH_WMSK(3)
 /* reset release*/
#define CRU_DMAC0_RST_RLS	WMSK_BIT(3)

#define CRU_DMAC1_RST		BIT_WITH_WMSK(4)
 /* reset release*/
#define CRU_DMAC1_RST_RLS	WMSK_BIT(4)

#define CRU_GLB_RST_CON		0x0510
#define CRU_GLB_SRST_FST	0x0500
#define CRU_GLB_SRST_SND	0x0504

#define CRU_CLKGATE_CON(n)	(0x300 + n * 4)
#define PCLK_GPIO2_GATE_SHIFT	3
#define PCLK_GPIO3_GATE_SHIFT	4
#define PCLK_GPIO4_GATE_SHIFT	5

/**************************************************
 * pmu cru reg, offset
 **************************************************/
#define CRU_PMU_RSTHOLD_CON(n)		(0x120 + n * 4)
/* reset hold*/
#define CRU_PMU_SGRF_RST_HOLD		BIT_WITH_WMSK(6)
/* reset hold release*/
#define CRU_PMU_SGRF_RST_RLS		WMSK_BIT(6)

#define CRU_PMU_WDTRST_MSK		(0x1 << 4)
#define CRU_PMU_WDTRST_EN		0x0

#define CRU_PMU_FIRST_SFTRST_MSK	(0x3 << 2)
#define CRU_PMU_FIRST_SFTRST_EN		0x0

#define CRU_PMU_CLKGATE_CON(n)		(0x100 + n * 4)
#define PCLK_GPIO0_GATE_SHIFT		3
#define PCLK_GPIO1_GATE_SHIFT		4

/**************************************************
 * sgrf reg, offset
 **************************************************/
#define SGRF_SOC_CON0_1(n)		(0xc000 + (n) * 4)
#define SGRF_SOC_CON3_7(n)		(0xe00c + ((n) - 3) * 4)
#define SGRF_SOC_CON8_15(n)		(0x8020 + ((n) - 8) * 4)
#define SGRF_PMU_SLV_CON0_1(n)		(0xc240 + ((n) - 0) * 4)
#define SGRF_SLV_SECURE_CON0_4(n)	(0xe3c0 + ((n) - 0) * 4)
#define SGRF_DDRRGN_CON0_16(n)		((n) * 4)
#define SGRF_DDRRGN_CON20_34(n)		(0x50 + ((n) - 20) * 4)

/* security config for master */
#define SGRF_SOC_CON_WMSK		0xffff0000
/* All of master in ns */
#define SGRF_SOC_ALLMST_NS		0xffff

/* security config for slave */
#define SGRF_SLV_S_WMSK			0xffff0000
#define SGRF_SLV_S_ALL_NS		0x0

/* security config pmu slave ip */
/* All of slaves  is ns */
#define SGRF_PMU_SLV_S_NS		BIT_WITH_WMSK(0)
/* slaves secure attr is configed */
#define SGRF_PMU_SLV_S_CFGED		WMSK_BIT(0)
#define SGRF_PMU_SLV_CRYPTO1_NS		WMSK_BIT(1)

#define SGRF_PMUSRAM_S			BIT(8)

#define SGRF_PMU_SLV_CON1_CFG		(SGRF_SLV_S_WMSK | \
					SGRF_PMUSRAM_S)
/* ddr region */
#define SGRF_DDR_RGN_BYPS	BIT_WITH_WMSK(9) /* All of ddr rgn  is ns */

/* The MST access the ddr rgn n with secure attribution */
#define SGRF_L_MST_S_DDR_RGN(n)	BIT_WITH_WMSK((n))
/* bits[16:8]*/
#define SGRF_H_MST_S_DDR_RGN(n)	BIT_WITH_WMSK((n) + 8)

/* dmac to periph s or ns*/
#define SGRF_DMAC_CFG_S		0xffff0000

#define DMAC1_RGN_NS			0xff000000
#define DMAC0_RGN_NS			0x00ff0000

#define DMAC0_BOOT_CFG_NS		0xfffffff8
#define DMAC0_BOOT_PERIPH_NS		0xffff0fff
#define DMAC0_BOOT_ADDR_NS		0xffff0000

#define DMAC1_BOOT_CFG_NS		0xffff0008
#define DMAC1_BOOT_PERIPH_L_NS		0xffff0fff
#define DMAC1_BOOT_ADDR_NS		0xffff0000
#define DMAC1_BOOT_PERIPH_H_NS		0xffffffff
#define DMAC1_BOOT_IRQ_NS		0xffffffff

#define CPU_BOOT_ADDR_WMASK	0xffff0000
#define CPU_BOOT_ADDR_ALIGN	16

/*
 * When system reset in running state, we want the cpus to be reboot
 * from maskrom (system reboot),
 * the pmusgrf reset-hold bits needs to be released.
 * When system wake up from system deep suspend, some soc will be reset
 * when waked up,
 * we want the bootcpu to be reboot from pmusram,
 * the pmusgrf reset-hold bits needs to be held.
 */
static inline void pmu_sgrf_rst_hld_release(void)
{
	mmio_write_32(PMUCRU_BASE + CRU_PMU_RSTHOLD_CON(1),
		      CRU_PMU_SGRF_RST_RLS);
}

static inline void pmu_sgrf_rst_hld(void)
{
	mmio_write_32(PMUCRU_BASE + CRU_PMU_RSTHOLD_CON(1),
		      CRU_PMU_SGRF_RST_HOLD);
}

/* funciton*/
void __dead2 soc_global_soft_reset(void);
void plls_resume(void);
void plls_suspend(void);

#endif /* __SOC_H__ */
