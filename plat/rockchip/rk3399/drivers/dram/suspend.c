/*
 * Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>

#include <dram.h>
#include <plat_private.h>
#include <pmu.h>
#include <pmu_bits.h>
#include <pmu_regs.h>
#include <rk3399_def.h>
#include <secure.h>
#include <soc.h>
#include <suspend.h>

#define PMUGRF_OS_REG0			0x300
#define PMUGRF_OS_REG1			0x304
#define PMUGRF_OS_REG2			0x308
#define PMUGRF_OS_REG3			0x30c

#define CRU_SFTRST_DDR_CTRL(ch, n)	((0x1 << (8 + 16 + (ch) * 4)) | \
					 ((n) << (8 + (ch) * 4)))
#define CRU_SFTRST_DDR_PHY(ch, n)	((0x1 << (9 + 16 + (ch) * 4)) | \
					 ((n) << (9 + (ch) * 4)))

#define FBDIV_ENC(n)			((n) << 16)
#define FBDIV_DEC(n)			(((n) >> 16) & 0xfff)
#define POSTDIV2_ENC(n)			((n) << 12)
#define POSTDIV2_DEC(n)			(((n) >> 12) & 0x7)
#define POSTDIV1_ENC(n)			((n) << 8)
#define POSTDIV1_DEC(n)			(((n) >> 8) & 0x7)
#define REFDIV_ENC(n)			(n)
#define REFDIV_DEC(n)			((n) & 0x3f)

/* PMU CRU */
#define PMUCRU_RSTNHOLD_CON0		0x120
#define PMUCRU_RSTNHOLD_CON1		0x124

#define PRESET_GPIO0_HOLD(n)		(((n) << 7) | WMSK_BIT(7))
#define PRESET_GPIO1_HOLD(n)		(((n) << 8) | WMSK_BIT(8))

#define SYS_COUNTER_FREQ_IN_MHZ		(SYS_COUNTER_FREQ_IN_TICKS / 1000000)

__pmusramdata uint32_t dpll_data[PLL_CON_COUNT];
__pmusramdata uint32_t cru_clksel_con6;
__pmusramdata uint8_t pmu_enable_watchdog0;

/*
 * Copy @num registers from @src to @dst
 */
static __pmusramfunc void sram_regcpy(uintptr_t dst, uintptr_t src,
		uint32_t num)
{
	while (num--) {
		mmio_write_32(dst, mmio_read_32(src));
		dst += sizeof(uint32_t);
		src += sizeof(uint32_t);
	}
}

/*
 * Copy @num registers from @src to @dst
 * This is intentionally a copy of the sram_regcpy function. PMUSRAM functions
 * cannot be called from code running in DRAM.
 */
static void dram_regcpy(uintptr_t dst, uintptr_t src, uint32_t num)
{
	while (num--) {
		mmio_write_32(dst, mmio_read_32(src));
		dst += sizeof(uint32_t);
		src += sizeof(uint32_t);
	}
}

static __pmusramfunc uint32_t sram_get_timer_value(void)
{
	/*
	 * Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntpct_el0() to simulate the down counter.
	 */
	return (uint32_t)(~read_cntpct_el0());
}

static __pmusramfunc void sram_udelay(uint32_t usec)
{
	uint32_t start, cnt, delta, total_ticks;

	/* counter is decreasing */
	start = sram_get_timer_value();
	total_ticks = usec * SYS_COUNTER_FREQ_IN_MHZ;
	do {
		cnt = sram_get_timer_value();
		if (cnt > start) {
			delta = UINT32_MAX - cnt;
			delta += start;
		} else
			delta = start - cnt;
	} while (delta <= total_ticks);
}

static __pmusramfunc void configure_sgrf(void)
{
	/*
	 * SGRF_DDR_RGN_DPLL_CLK and SGRF_DDR_RGN_RTC_CLK:
	 * IC ECO bug, need to set this register.
	 *
	 * SGRF_DDR_RGN_BYPS:
	 * After the PD_CENTER suspend/resume, the DDR region
	 * related registers in the SGRF will be reset, we
	 * need to re-initialize them.
	 */
	mmio_write_32(SGRF_BASE + SGRF_DDRRGN_CON0_16(16),
		      SGRF_DDR_RGN_DPLL_CLK |
		      SGRF_DDR_RGN_RTC_CLK |
		      SGRF_DDR_RGN_BYPS);
}

static __pmusramfunc void rkclk_ddr_reset(uint32_t channel, uint32_t ctl,
		uint32_t phy)
{
	channel &= 0x1;
	ctl &= 0x1;
	phy &= 0x1;
	mmio_write_32(CRU_BASE + CRU_SOFTRST_CON(4),
		      CRU_SFTRST_DDR_CTRL(channel, ctl) |
		      CRU_SFTRST_DDR_PHY(channel, phy));
}

static __pmusramfunc void phy_pctrl_reset(uint32_t ch)
{
	rkclk_ddr_reset(ch, 1, 1);
	sram_udelay(10);
	rkclk_ddr_reset(ch, 1, 0);
	sram_udelay(10);
	rkclk_ddr_reset(ch, 0, 0);
	sram_udelay(10);
}

static __pmusramfunc void set_cs_training_index(uint32_t ch, uint32_t rank)
{
	uint32_t byte;

	/* PHY_8/136/264/392 phy_per_cs_training_index_X 1bit offset_24 */
	for (byte = 0; byte < 4; byte++)
		mmio_clrsetbits_32(PHY_REG(ch, 8 + (128 * byte)), 0x1 << 24,
				   rank << 24);
}

static __pmusramfunc void select_per_cs_training_index(uint32_t ch,
		uint32_t rank)
{
	/* PHY_84 PHY_PER_CS_TRAINING_EN_0 1bit offset_16 */
	if ((mmio_read_32(PHY_REG(ch, 84)) >> 16) & 1)
		set_cs_training_index(ch, rank);
}

static __pmusramfunc void override_write_leveling_value(uint32_t ch)
{
	uint32_t byte;

	for (byte = 0; byte < 4; byte++) {
		/*
		 * PHY_8/136/264/392
		 * phy_per_cs_training_multicast_en_X 1bit offset_16
		 */
		mmio_clrsetbits_32(PHY_REG(ch, 8 + (128 * byte)), 0x1 << 16,
				   1 << 16);
		mmio_clrsetbits_32(PHY_REG(ch, 63 + (128 * byte)),
				   0xffffu << 16,
				   0x200 << 16);
	}

	/* CTL_200 ctrlupd_req 1bit offset_8 */
	mmio_clrsetbits_32(CTL_REG(ch, 200), 0x1 << 8, 0x1 << 8);
}

static __pmusramfunc int data_training(uint32_t ch,
		struct rk3399_sdram_params *sdram_params,
		uint32_t training_flag)
{
	uint32_t obs_0, obs_1, obs_2, obs_3, obs_err = 0;
	uint32_t rank = sdram_params->ch[ch].rank;
	uint32_t rank_mask;
	uint32_t i, tmp;

	if (sdram_params->dramtype == LPDDR4)
		rank_mask = (rank == 1) ? 0x5 : 0xf;
	else
		rank_mask = (rank == 1) ? 0x1 : 0x3;

	/* PHY_927 PHY_PAD_DQS_DRIVE  RPULL offset_22 */
	mmio_setbits_32(PHY_REG(ch, 927), (1 << 22));

	if (training_flag == PI_FULL_TRAINING) {
		if (sdram_params->dramtype == LPDDR4) {
			training_flag = PI_WRITE_LEVELING |
					PI_READ_GATE_TRAINING |
					PI_READ_LEVELING |
					PI_WDQ_LEVELING;
		} else if (sdram_params->dramtype == LPDDR3) {
			training_flag = PI_CA_TRAINING | PI_WRITE_LEVELING |
					PI_READ_GATE_TRAINING;
		} else if (sdram_params->dramtype == DDR3) {
			training_flag = PI_WRITE_LEVELING |
					PI_READ_GATE_TRAINING |
					PI_READ_LEVELING;
		}
	}

	/* ca training(LPDDR4,LPDDR3 support) */
	if ((training_flag & PI_CA_TRAINING) == PI_CA_TRAINING) {
		for (i = 0; i < 4; i++) {
			if (!(rank_mask & (1 << i)))
				continue;

			select_per_cs_training_index(ch, i);
			/* PI_100 PI_CALVL_EN:RW:8:2 */
			mmio_clrsetbits_32(PI_REG(ch, 100), 0x3 << 8, 0x2 << 8);

			/* PI_92 PI_CALVL_REQ:WR:16:1,PI_CALVL_CS:RW:24:2 */
			mmio_clrsetbits_32(PI_REG(ch, 92),
					   (0x1 << 16) | (0x3 << 24),
					   (0x1 << 16) | (i << 24));
			while (1) {
				/* PI_174 PI_INT_STATUS:RD:8:18 */
				tmp = mmio_read_32(PI_REG(ch, 174)) >> 8;

				/*
				 * check status obs
				 * PHY_532/660/788 phy_adr_calvl_obs1_:0:32
				 */
				obs_0 = mmio_read_32(PHY_REG(ch, 532));
				obs_1 = mmio_read_32(PHY_REG(ch, 660));
				obs_2 = mmio_read_32(PHY_REG(ch, 788));
				if (((obs_0 >> 30) & 0x3) ||
				    ((obs_1 >> 30) & 0x3) ||
				    ((obs_2 >> 30) & 0x3))
					obs_err = 1;
				if ((((tmp >> 11) & 0x1) == 0x1) &&
				    (((tmp >> 13) & 0x1) == 0x1) &&
				    (((tmp >> 5) & 0x1) == 0x0) &&
				    (obs_err == 0))
					break;
				else if ((((tmp >> 5) & 0x1) == 0x1) ||
					 (obs_err == 1))
					return -1;
			}
			/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
			mmio_write_32(PI_REG(ch, 175), 0x00003f7c);
		}
		mmio_clrbits_32(PI_REG(ch, 100), 0x3 << 8);
	}

	/* write leveling(LPDDR4,LPDDR3,DDR3 support) */
	if ((training_flag & PI_WRITE_LEVELING) == PI_WRITE_LEVELING) {
		for (i = 0; i < rank; i++) {
			select_per_cs_training_index(ch, i);
			/* PI_60 PI_WRLVL_EN:RW:8:2 */
			mmio_clrsetbits_32(PI_REG(ch, 60), 0x3 << 8, 0x2 << 8);
			/* PI_59 PI_WRLVL_REQ:WR:8:1,PI_WRLVL_CS:RW:16:2 */
			mmio_clrsetbits_32(PI_REG(ch, 59),
					   (0x1 << 8) | (0x3 << 16),
					   (0x1 << 8) | (i << 16));

			while (1) {
				/* PI_174 PI_INT_STATUS:RD:8:18 */
				tmp = mmio_read_32(PI_REG(ch, 174)) >> 8;

				/*
				 * check status obs, if error maybe can not
				 * get leveling done PHY_40/168/296/424
				 * phy_wrlvl_status_obs_X:0:13
				 */
				obs_0 = mmio_read_32(PHY_REG(ch, 40));
				obs_1 = mmio_read_32(PHY_REG(ch, 168));
				obs_2 = mmio_read_32(PHY_REG(ch, 296));
				obs_3 = mmio_read_32(PHY_REG(ch, 424));
				if (((obs_0 >> 12) & 0x1) ||
				    ((obs_1 >> 12) & 0x1) ||
				    ((obs_2 >> 12) & 0x1) ||
				    ((obs_3 >> 12) & 0x1))
					obs_err = 1;
				if ((((tmp >> 10) & 0x1) == 0x1) &&
				    (((tmp >> 13) & 0x1) == 0x1) &&
				    (((tmp >> 4) & 0x1) == 0x0) &&
				    (obs_err == 0))
					break;
				else if ((((tmp >> 4) & 0x1) == 0x1) ||
					 (obs_err == 1))
					return -1;
			}

			/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
			mmio_write_32(PI_REG(ch, 175), 0x00003f7c);
		}
		override_write_leveling_value(ch);
		mmio_clrbits_32(PI_REG(ch, 60), 0x3 << 8);
	}

	/* read gate training(LPDDR4,LPDDR3,DDR3 support) */
	if ((training_flag & PI_READ_GATE_TRAINING) == PI_READ_GATE_TRAINING) {
		for (i = 0; i < rank; i++) {
			select_per_cs_training_index(ch, i);
			/* PI_80 PI_RDLVL_GATE_EN:RW:24:2 */
			mmio_clrsetbits_32(PI_REG(ch, 80), 0x3 << 24,
					   0x2 << 24);
			/*
			 * PI_74 PI_RDLVL_GATE_REQ:WR:16:1
			 * PI_RDLVL_CS:RW:24:2
			 */
			mmio_clrsetbits_32(PI_REG(ch, 74),
					   (0x1 << 16) | (0x3 << 24),
					   (0x1 << 16) | (i << 24));

			while (1) {
				/* PI_174 PI_INT_STATUS:RD:8:18 */
				tmp = mmio_read_32(PI_REG(ch, 174)) >> 8;

				/*
				 * check status obs
				 * PHY_43/171/299/427
				 *     PHY_GTLVL_STATUS_OBS_x:16:8
				 */
				obs_0 = mmio_read_32(PHY_REG(ch, 43));
				obs_1 = mmio_read_32(PHY_REG(ch, 171));
				obs_2 = mmio_read_32(PHY_REG(ch, 299));
				obs_3 = mmio_read_32(PHY_REG(ch, 427));
				if (((obs_0 >> (16 + 6)) & 0x3) ||
				    ((obs_1 >> (16 + 6)) & 0x3) ||
				    ((obs_2 >> (16 + 6)) & 0x3) ||
				    ((obs_3 >> (16 + 6)) & 0x3))
					obs_err = 1;
				if ((((tmp >> 9) & 0x1) == 0x1) &&
				    (((tmp >> 13) & 0x1) == 0x1) &&
				    (((tmp >> 3) & 0x1) == 0x0) &&
				    (obs_err == 0))
					break;
				else if ((((tmp >> 3) & 0x1) == 0x1) ||
					 (obs_err == 1))
					return -1;
			}
			/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
			mmio_write_32(PI_REG(ch, 175), 0x00003f7c);
		}
		mmio_clrbits_32(PI_REG(ch, 80), 0x3 << 24);
	}

	/* read leveling(LPDDR4,LPDDR3,DDR3 support) */
	if ((training_flag & PI_READ_LEVELING) == PI_READ_LEVELING) {
		for (i = 0; i < rank; i++) {
			select_per_cs_training_index(ch, i);
			/* PI_80 PI_RDLVL_EN:RW:16:2 */
			mmio_clrsetbits_32(PI_REG(ch, 80), 0x3 << 16,
					   0x2 << 16);
			/* PI_74 PI_RDLVL_REQ:WR:8:1,PI_RDLVL_CS:RW:24:2 */
			mmio_clrsetbits_32(PI_REG(ch, 74),
					   (0x1 << 8) | (0x3 << 24),
					   (0x1 << 8) | (i << 24));
			while (1) {
				/* PI_174 PI_INT_STATUS:RD:8:18 */
				tmp = mmio_read_32(PI_REG(ch, 174)) >> 8;

				/*
				 * make sure status obs not report error bit
				 * PHY_46/174/302/430
				 *     phy_rdlvl_status_obs_X:16:8
				 */
				if ((((tmp >> 8) & 0x1) == 0x1) &&
				    (((tmp >> 13) & 0x1) == 0x1) &&
				    (((tmp >> 2) & 0x1) == 0x0))
					break;
				else if (((tmp >> 2) & 0x1) == 0x1)
					return -1;
			}
			/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
			mmio_write_32(PI_REG(ch, 175), 0x00003f7c);
		}
		mmio_clrbits_32(PI_REG(ch, 80), 0x3 << 16);
	}

	/* wdq leveling(LPDDR4 support) */
	if ((training_flag & PI_WDQ_LEVELING) == PI_WDQ_LEVELING) {
		for (i = 0; i < 4; i++) {
			if (!(rank_mask & (1 << i)))
				continue;

			select_per_cs_training_index(ch, i);
			/*
			 * disable PI_WDQLVL_VREF_EN before wdq leveling?
			 * PI_181 PI_WDQLVL_VREF_EN:RW:8:1
			 */
			mmio_clrbits_32(PI_REG(ch, 181), 0x1 << 8);
			/* PI_124 PI_WDQLVL_EN:RW:16:2 */
			mmio_clrsetbits_32(PI_REG(ch, 124), 0x3 << 16,
					   0x2 << 16);
			/* PI_121 PI_WDQLVL_REQ:WR:8:1,PI_WDQLVL_CS:RW:16:2 */
			mmio_clrsetbits_32(PI_REG(ch, 121),
					   (0x1 << 8) | (0x3 << 16),
					   (0x1 << 8) | (i << 16));
			while (1) {
				/* PI_174 PI_INT_STATUS:RD:8:18 */
				tmp = mmio_read_32(PI_REG(ch, 174)) >> 8;
				if ((((tmp >> 12) & 0x1) == 0x1) &&
				    (((tmp >> 13) & 0x1) == 0x1) &&
				    (((tmp >> 6) & 0x1) == 0x0))
					break;
				else if (((tmp >> 6) & 0x1) == 0x1)
					return -1;
			}
			/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
			mmio_write_32(PI_REG(ch, 175), 0x00003f7c);
		}
		mmio_clrbits_32(PI_REG(ch, 124), 0x3 << 16);
	}

	/* PHY_927 PHY_PAD_DQS_DRIVE  RPULL offset_22 */
	mmio_clrbits_32(PHY_REG(ch, 927), (1 << 22));

	return 0;
}

static __pmusramfunc void set_ddrconfig(
		struct rk3399_sdram_params *sdram_params,
		unsigned char channel, uint32_t ddrconfig)
{
	/* only need to set ddrconfig */
	struct rk3399_sdram_channel *ch = &sdram_params->ch[channel];
	unsigned int cs0_cap = 0;
	unsigned int cs1_cap = 0;

	cs0_cap = (1 << (ch->cs0_row + ch->col + ch->bk + ch->bw - 20));
	if (ch->rank > 1)
		cs1_cap = cs0_cap >> (ch->cs0_row - ch->cs1_row);
	if (ch->row_3_4) {
		cs0_cap = cs0_cap * 3 / 4;
		cs1_cap = cs1_cap * 3 / 4;
	}

	mmio_write_32(MSCH_BASE(channel) + MSCH_DEVICECONF,
		      ddrconfig | (ddrconfig << 6));
	mmio_write_32(MSCH_BASE(channel) + MSCH_DEVICESIZE,
		      ((cs0_cap / 32) & 0xff) | (((cs1_cap / 32) & 0xff) << 8));
}

static __pmusramfunc void dram_all_config(
		struct rk3399_sdram_params *sdram_params)
{
	unsigned int i;

	for (i = 0; i < 2; i++) {
		struct rk3399_sdram_channel *info = &sdram_params->ch[i];
		struct rk3399_msch_timings *noc = &info->noc_timings;

		if (sdram_params->ch[i].col == 0)
			continue;

		mmio_write_32(MSCH_BASE(i) + MSCH_DDRTIMINGA0,
			      noc->ddrtiminga0.d32);
		mmio_write_32(MSCH_BASE(i) + MSCH_DDRTIMINGB0,
			      noc->ddrtimingb0.d32);
		mmio_write_32(MSCH_BASE(i) + MSCH_DDRTIMINGC0,
			      noc->ddrtimingc0.d32);
		mmio_write_32(MSCH_BASE(i) + MSCH_DEVTODEV0,
			      noc->devtodev0.d32);
		mmio_write_32(MSCH_BASE(i) + MSCH_DDRMODE, noc->ddrmode.d32);

		/* rank 1 memory clock disable (dfi_dram_clk_disable = 1) */
		if (sdram_params->ch[i].rank == 1)
			mmio_setbits_32(CTL_REG(i, 276), 1 << 17);
	}

	DDR_STRIDE(sdram_params->stride);

	/* reboot hold register set */
	mmio_write_32(PMUCRU_BASE + CRU_PMU_RSTHOLD_CON(1),
		      CRU_PMU_SGRF_RST_RLS |
		      PRESET_GPIO0_HOLD(1) |
		      PRESET_GPIO1_HOLD(1));
	mmio_clrsetbits_32(CRU_BASE + CRU_GLB_RST_CON, 0x3, 0x3);
}

static __pmusramfunc void pctl_cfg(uint32_t ch,
		struct rk3399_sdram_params *sdram_params)
{
	const uint32_t *params_ctl = sdram_params->pctl_regs.denali_ctl;
	const uint32_t *params_pi = sdram_params->pi_regs.denali_pi;
	const struct rk3399_ddr_publ_regs *phy_regs = &sdram_params->phy_regs;
	uint32_t tmp, tmp1, tmp2, i;

	/*
	 * Workaround controller bug:
	 * Do not program DRAM_CLASS until NO_PHY_IND_TRAIN_INT is programmed
	 */
	sram_regcpy(CTL_REG(ch, 1), (uintptr_t)&params_ctl[1],
		    CTL_REG_NUM - 1);
	mmio_write_32(CTL_REG(ch, 0), params_ctl[0]);
	sram_regcpy(PI_REG(ch, 0), (uintptr_t)&params_pi[0],
		    PI_REG_NUM);

	sram_regcpy(PHY_REG(ch, 910), (uintptr_t)&phy_regs->phy896[910 - 896],
		    3);

	mmio_clrsetbits_32(CTL_REG(ch, 68), PWRUP_SREFRESH_EXIT,
				PWRUP_SREFRESH_EXIT);

	/* PHY_DLL_RST_EN */
	mmio_clrsetbits_32(PHY_REG(ch, 957), 0x3 << 24, 1 << 24);
	dmbst();

	mmio_setbits_32(PI_REG(ch, 0), START);
	mmio_setbits_32(CTL_REG(ch, 0), START);

	/* wait lock */
	while (1) {
		tmp = mmio_read_32(PHY_REG(ch, 920));
		tmp1 = mmio_read_32(PHY_REG(ch, 921));
		tmp2 = mmio_read_32(PHY_REG(ch, 922));
		if ((((tmp >> 16) & 0x1) == 0x1) &&
		     (((tmp1 >> 16) & 0x1) == 0x1) &&
		     (((tmp1 >> 0) & 0x1) == 0x1) &&
		     (((tmp2 >> 0) & 0x1) == 0x1))
			break;
		/* if PLL bypass,don't need wait lock */
		if (mmio_read_32(PHY_REG(ch, 911)) & 0x1)
			break;
	}

	sram_regcpy(PHY_REG(ch, 896), (uintptr_t)&phy_regs->phy896[0], 63);

	for (i = 0; i < 4; i++)
		sram_regcpy(PHY_REG(ch, 128 * i),
			    (uintptr_t)&phy_regs->phy0[0], 91);

	for (i = 0; i < 3; i++)
		sram_regcpy(PHY_REG(ch, 512 + 128 * i),
				(uintptr_t)&phy_regs->phy512[i][0], 38);
}

static __pmusramfunc int dram_switch_to_next_index(
		struct rk3399_sdram_params *sdram_params)
{
	uint32_t ch, ch_count;
	uint32_t fn = ((mmio_read_32(CTL_REG(0, 111)) >> 16) + 1) & 0x1;

	mmio_write_32(CIC_BASE + CIC_CTRL0,
		      (((0x3 << 4) | (1 << 2) | 1) << 16) |
		      (fn << 4) | (1 << 2) | 1);
	while (!(mmio_read_32(CIC_BASE + CIC_STATUS0) & (1 << 2)))
		;

	mmio_write_32(CIC_BASE + CIC_CTRL0, 0x20002);
	while (!(mmio_read_32(CIC_BASE + CIC_STATUS0) & (1 << 0)))
		;

	ch_count = sdram_params->num_channels;

	/* LPDDR4 f2 cann't do training, all training will fail */
	for (ch = 0; ch < ch_count; ch++) {
		/*
		 * Without this disabled for LPDDR4 we end up writing 0's
		 * in place of real data in an interesting pattern.
		 */
		if (sdram_params->dramtype != LPDDR4) {
			mmio_clrsetbits_32(PHY_REG(ch, 896), (0x3 << 8) | 1,
					fn << 8);
		}

		/* data_training failed */
		if (data_training(ch, sdram_params, PI_FULL_TRAINING))
			return -1;
	}

	return 0;
}

/*
 * Needs to be done for both channels at once in case of a shared reset signal
 * between channels.
 */
static __pmusramfunc int pctl_start(uint32_t channel_mask,
		struct rk3399_sdram_params *sdram_params)
{
	uint32_t count;
	uint32_t byte;

	mmio_setbits_32(CTL_REG(0, 68), PWRUP_SREFRESH_EXIT);
	mmio_setbits_32(CTL_REG(1, 68), PWRUP_SREFRESH_EXIT);

	/* need de-access IO retention before controller START */
	if (channel_mask & (1 << 0))
		mmio_setbits_32(PMU_BASE + PMU_PWRMODE_CON, (1 << 19));
	if (channel_mask & (1 << 1))
		mmio_setbits_32(PMU_BASE + PMU_PWRMODE_CON, (1 << 23));

	/* PHY_DLL_RST_EN */
	if (channel_mask & (1 << 0))
		mmio_clrsetbits_32(PHY_REG(0, 957), 0x3 << 24,
				   0x2 << 24);
	if (channel_mask & (1 << 1))
		mmio_clrsetbits_32(PHY_REG(1, 957), 0x3 << 24,
				   0x2 << 24);

	/* check ERROR bit */
	if (channel_mask & (1 << 0)) {
		count = 0;
		while (!(mmio_read_32(CTL_REG(0, 203)) & (1 << 3))) {
			/* CKE is low, loop 10ms */
			if (count > 100)
				return -1;

			sram_udelay(100);
			count++;
		}

		mmio_clrbits_32(CTL_REG(0, 68), PWRUP_SREFRESH_EXIT);

		/* Restore the PHY_RX_CAL_DQS value */
		for (byte = 0; byte < 4; byte++)
			mmio_clrsetbits_32(PHY_REG(0, 57 + 128 * byte),
					   0xfff << 16,
					   sdram_params->rx_cal_dqs[0][byte]);
	}
	if (channel_mask & (1 << 1)) {
		count = 0;
		while (!(mmio_read_32(CTL_REG(1, 203)) & (1 << 3))) {
			/* CKE is low, loop 10ms */
			if (count > 100)
				return -1;

			sram_udelay(100);
			count++;
		}

		mmio_clrbits_32(CTL_REG(1, 68), PWRUP_SREFRESH_EXIT);

		/* Restore the PHY_RX_CAL_DQS value */
		for (byte = 0; byte < 4; byte++)
			mmio_clrsetbits_32(PHY_REG(1, 57 + 128 * byte),
					   0xfff << 16,
					   sdram_params->rx_cal_dqs[1][byte]);
	}

	return 0;
}

__pmusramfunc static void pmusram_restore_pll(int pll_id, uint32_t *src)
{
	mmio_write_32((CRU_BASE + CRU_PLL_CON(pll_id, 3)), PLL_SLOW_MODE);

	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 0), src[0] | REG_SOC_WMSK);
	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 1), src[1] | REG_SOC_WMSK);
	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 2), src[2]);
	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 4), src[4] | REG_SOC_WMSK);
	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 5), src[5] | REG_SOC_WMSK);

	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 3), src[3] | REG_SOC_WMSK);

	while ((mmio_read_32(CRU_BASE + CRU_PLL_CON(pll_id, 2)) &
		(1U << 31)) == 0x0)
		;
}

__pmusramfunc static void pmusram_enable_watchdog(void)
{
	/* Make the watchdog use the first global reset. */
	mmio_write_32(CRU_BASE + CRU_GLB_RST_CON, 1 << 1);

	/*
	 * This gives the system ~8 seconds before reset. The pclk for the
	 * watchdog is 4MHz on reset. The value of 0x9 in WDT_TORR means that
	 * the watchdog will wait for 0x1ffffff cycles before resetting.
	 */
	mmio_write_32(WDT0_BASE + 4, 0x9);

	/* Enable the watchdog */
	mmio_setbits_32(WDT0_BASE, 0x1);

	/* Magic reset the watchdog timer value for WDT_CRR. */
	mmio_write_32(WDT0_BASE + 0xc, 0x76);

	secure_watchdog_ungate();

	/* The watchdog is in PD_ALIVE, so deidle it. */
	mmio_clrbits_32(PMU_BASE + PMU_BUS_CLR, PMU_CLR_ALIVE);
}

void dmc_suspend(void)
{
	struct rk3399_sdram_params *sdram_params = &sdram_config;
	struct rk3399_ddr_publ_regs *phy_regs;
	uint32_t *params_ctl;
	uint32_t *params_pi;
	uint32_t refdiv, postdiv2, postdiv1, fbdiv;
	uint32_t ch, byte, i;

	phy_regs = &sdram_params->phy_regs;
	params_ctl = sdram_params->pctl_regs.denali_ctl;
	params_pi = sdram_params->pi_regs.denali_pi;

	/* save dpll register and ddr clock register value to pmusram */
	cru_clksel_con6 = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON6);
	for (i = 0; i < PLL_CON_COUNT; i++)
		dpll_data[i] = mmio_read_32(CRU_BASE + CRU_PLL_CON(DPLL_ID, i));

	fbdiv = dpll_data[0] & 0xfff;
	postdiv2 = POSTDIV2_DEC(dpll_data[1]);
	postdiv1 = POSTDIV1_DEC(dpll_data[1]);
	refdiv = REFDIV_DEC(dpll_data[1]);

	sdram_params->ddr_freq = ((fbdiv * 24) /
				(refdiv * postdiv1 * postdiv2)) * MHz;

	INFO("sdram_params->ddr_freq = %d\n", sdram_params->ddr_freq);
	sdram_params->odt = (((mmio_read_32(PHY_REG(0, 5)) >> 16) &
			       0x7) != 0) ? 1 : 0;

	/* copy the registers CTL PI and PHY */
	dram_regcpy((uintptr_t)&params_ctl[0], CTL_REG(0, 0), CTL_REG_NUM);

	/* mask DENALI_CTL_00_DATA.START, only copy here, will trigger later */
	params_ctl[0] &= ~(0x1 << 0);

	dram_regcpy((uintptr_t)&params_pi[0], PI_REG(0, 0),
		    PI_REG_NUM);

	/* mask DENALI_PI_00_DATA.START, only copy here, will trigger later*/
	params_pi[0] &= ~(0x1 << 0);

	dram_regcpy((uintptr_t)&phy_regs->phy0[0],
			    PHY_REG(0, 0), 91);

	for (i = 0; i < 3; i++)
		dram_regcpy((uintptr_t)&phy_regs->phy512[i][0],
			    PHY_REG(0, 512 + 128 * i), 38);

	dram_regcpy((uintptr_t)&phy_regs->phy896[0], PHY_REG(0, 896), 63);

	for (ch = 0; ch < sdram_params->num_channels; ch++) {
		for (byte = 0; byte < 4; byte++)
			sdram_params->rx_cal_dqs[ch][byte] = (0xfff << 16) &
				mmio_read_32(PHY_REG(ch, 57 + byte * 128));
	}

	/* set DENALI_PHY_957_DATA.PHY_DLL_RST_EN = 0x1 */
	phy_regs->phy896[957 - 896] &= ~(0x3 << 24);
	phy_regs->phy896[957 - 896] |= 1 << 24;
	phy_regs->phy896[0] |= 1;
	phy_regs->phy896[0] &= ~(0x3 << 8);
}

__pmusramfunc void phy_dll_bypass_set(uint32_t ch, uint32_t freq)
{
	if (freq <= (125 * 1000 * 1000)) {
		/* Set master mode to SW for slices*/
		mmio_setbits_32(PHY_REG(ch, 86), 3 << 10);
		mmio_setbits_32(PHY_REG(ch, 214), 3 << 10);
		mmio_setbits_32(PHY_REG(ch, 342), 3 << 10);
		mmio_setbits_32(PHY_REG(ch, 470), 3 << 10);
		/* Set master mode to SW for address slices*/
		mmio_setbits_32(PHY_REG(ch, 547), 3 << 18);
		mmio_setbits_32(PHY_REG(ch, 675), 3 << 18);
		mmio_setbits_32(PHY_REG(ch, 803), 3 << 18);
	} else {
		/* Clear SW master mode for slices*/
		mmio_clrbits_32(PHY_REG(ch, 86), 3 << 10);
		mmio_clrbits_32(PHY_REG(ch, 214), 3 << 10);
		mmio_clrbits_32(PHY_REG(ch, 342), 3 << 10);
		mmio_clrbits_32(PHY_REG(ch, 470), 3 << 10);
		/* Clear SW master mode for address slices*/
		mmio_clrbits_32(PHY_REG(ch, 547), 3 << 18);
		mmio_clrbits_32(PHY_REG(ch, 675), 3 << 18);
		mmio_clrbits_32(PHY_REG(ch, 803), 3 << 18);
	}
}

__pmusramfunc void dmc_resume(void)
{
	struct rk3399_sdram_params *sdram_params = &sdram_config;
	uint32_t channel_mask = 0;
	uint32_t channel;

	/*
	 * We can't turn off the watchdog, so if we have not turned it on before
	 * we should not turn it on here.
	 */
	if ((pmu_enable_watchdog0 & 0x1) == 0x1) {
		pmusram_enable_watchdog();
	}
	pmu_sgrf_rst_hld_release();
	restore_pmu_rsthold();
	sram_secure_timer_init();

	/*
	 * we switch ddr clock to abpll when suspend,
	 * we set back to dpll here
	 */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON6,
			cru_clksel_con6 | REG_SOC_WMSK);
	pmusram_restore_pll(DPLL_ID, dpll_data);

	configure_sgrf();

retry:
	for (channel = 0; channel < sdram_params->num_channels; channel++) {
		phy_pctrl_reset(channel);
		/*
		 * Without this, LPDDR4 will write 0's in place of real data
		 * in a strange pattern.
		 */
		if (sdram_params->dramtype == LPDDR4) {
			phy_dll_bypass_set(channel, sdram_params->ddr_freq);
		}
		pctl_cfg(channel, sdram_params);
	}

	for (channel = 0; channel < 2; channel++) {
		if (sdram_params->ch[channel].col)
			channel_mask |= 1 << channel;
	}

	if (pctl_start(channel_mask, sdram_params) < 0)
		goto retry;

	for (channel = 0; channel < sdram_params->num_channels; channel++) {
		/* LPDDR2/LPDDR3 need to wait DAI complete, max 10us */
		if (sdram_params->dramtype == LPDDR3)
			sram_udelay(10);

		/*
		 * Training here will always fail for LPDDR4, so skip it
		 * If traning fail, retry to do it again.
		 */
		if (sdram_params->dramtype != LPDDR4 &&
		    data_training(channel, sdram_params, PI_FULL_TRAINING))
			goto retry;

		set_ddrconfig(sdram_params, channel,
			      sdram_params->ch[channel].ddrconfig);
	}

	dram_all_config(sdram_params);

	/* Switch to index 1 and prepare for DDR frequency switch. */
	dram_switch_to_next_index(sdram_params);
}
