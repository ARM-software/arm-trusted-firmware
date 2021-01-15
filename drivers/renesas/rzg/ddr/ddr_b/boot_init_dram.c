/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "boot_init_dram.h"
#include "boot_init_dram_regdef.h"
#include "ddr_regdef.h"
#include "dram_sub_func.h"
#include "init_dram_tbl_g2m.h"
#include "micro_delay.h"
#include "rcar_def.h"

/* load board configuration */
#include "boot_init_dram_config.c"

#define DDR_BACKUPMODE
#define FATAL_MSG(x) NOTICE(x)

/* variables */
#ifdef RCAR_DDR_FIXED_LSI_TYPE
#ifndef RCAR_AUTO
#define RCAR_AUTO	99U
#define RZ_G2M		100U

#define RCAR_CUT_10	0U
#define RCAR_CUT_11	1U
#define RCAR_CUT_20	10U
#define RCAR_CUT_30	20U
#endif /* RCAR_AUTO */
#ifndef RCAR_LSI
#define RCAR_LSI	RCAR_AUTO
#endif

#if (RCAR_LSI == RCAR_AUTO)
static uint32_t prr_product;
static uint32_t prr_cut;
#else /* RCAR_LSI == RCAR_AUTO */
#if (RCAR_LSI == RZ_G2M)
static const uint32_t prr_product = PRR_PRODUCT_M3;
#endif /* RCAR_LSI == RZ_G2M */

#ifndef RCAR_LSI_CUT
static uint32_t prr_cut;
#else /* RCAR_LSI_CUT */
#if (RCAR_LSI_CUT == RCAR_CUT_10)
static const uint32_t prr_cut = PRR_PRODUCT_10;
#elif(RCAR_LSI_CUT == RCAR_CUT_11)
static const uint32_t prr_cut = PRR_PRODUCT_11;
#elif(RCAR_LSI_CUT == RCAR_CUT_20)
static const uint32_t prr_cut = PRR_PRODUCT_20;
#elif(RCAR_LSI_CUT == RCAR_CUT_30)
static const uint32_t prr_cut = PRR_PRODUCT_30;
#endif /* RCAR_LSI_CUT == RCAR_CUT_10 */
#endif /* RCAR_LSI_CUT */
#endif /* RCAR_LSI == RCAR_AUTO */
#else /* RCAR_DDR_FIXED_LSI_TYPE */
static uint32_t prr_product;
static uint32_t prr_cut;
#endif /* RCAR_DDR_FIXED_LSI_TYPE */

static const uint32_t *p_ddr_regdef_tbl;
static uint32_t brd_clk;
static uint32_t brd_clkdiv;
static uint32_t brd_clkdiva;
static uint32_t ddr_mbps;
static uint32_t ddr_mbpsdiv;
static uint32_t ddr_tccd;
static uint32_t ddr_phycaslice;
static const struct _boardcnf *board_cnf;
static uint32_t ddr_phyvalid;
static uint32_t ddr_density[DRAM_CH_CNT][CS_CNT];
static uint32_t ch_have_this_cs[CS_CNT] __aligned(64);
static uint32_t rdqdm_dly[DRAM_CH_CNT][CSAB_CNT][SLICE_CNT * 2U][9U];
static uint32_t max_density;
static uint32_t ddr0800_mul;
static uint32_t ddr_mul;
static uint32_t DDR_PHY_SLICE_REGSET_OFS;
static uint32_t DDR_PHY_ADR_V_REGSET_OFS;
static uint32_t DDR_PHY_ADR_I_REGSET_OFS;
static uint32_t DDR_PHY_ADR_G_REGSET_OFS;
static uint32_t DDR_PI_REGSET_OFS;
static uint32_t DDR_PHY_SLICE_REGSET_SIZE;
static uint32_t DDR_PHY_ADR_V_REGSET_SIZE;
static uint32_t DDR_PHY_ADR_I_REGSET_SIZE;
static uint32_t DDR_PHY_ADR_G_REGSET_SIZE;
static uint32_t DDR_PI_REGSET_SIZE;
static uint32_t DDR_PHY_SLICE_REGSET_NUM;
static uint32_t DDR_PHY_ADR_V_REGSET_NUM;
static uint32_t DDR_PHY_ADR_I_REGSET_NUM;
static uint32_t DDR_PHY_ADR_G_REGSET_NUM;
static uint32_t DDR_PI_REGSET_NUM;
static uint32_t DDR_PHY_ADR_I_NUM;
#define DDR_PHY_REGSET_MAX 128
#define DDR_PI_REGSET_MAX 320
static uint32_t _cnf_DDR_PHY_SLICE_REGSET[DDR_PHY_REGSET_MAX];
static uint32_t _cnf_DDR_PHY_ADR_V_REGSET[DDR_PHY_REGSET_MAX];
static uint32_t _cnf_DDR_PHY_ADR_I_REGSET[DDR_PHY_REGSET_MAX];
static uint32_t _cnf_DDR_PHY_ADR_G_REGSET[DDR_PHY_REGSET_MAX];
static uint32_t _cnf_DDR_PI_REGSET[DDR_PI_REGSET_MAX];
static uint32_t pll3_mode;
static uint32_t loop_max;
#ifdef DDR_BACKUPMODE
uint32_t ddr_backup = DRAM_BOOT_STATUS_COLD;
/* #define DDR_BACKUPMODE_HALF  */  /* for Half channel(ch0,1 only) */
#endif

#ifdef DDR_QOS_INIT_SETTING	/*  only for non qos_init */
#define OPERATING_FREQ			(400U)	/* Mhz */
#define BASE_SUB_SLOT_NUM		(0x6U)
#define SUB_SLOT_CYCLE			(0x7EU)	/* 126 */
#define QOSWT_WTSET0_CYCLE		\
	((SUB_SLOT_CYCLE * BASE_SUB_SLOT_NUM * 1000U) / \
	OPERATING_FREQ)	/* unit:ns */

uint32_t get_refperiod(void)
{
	return QOSWT_WTSET0_CYCLE;
}
#else /*  DDR_QOS_INIT_SETTING */
extern uint32_t get_refperiod(void);
#endif /* DDR_QOS_INIT_SETTING */

#define _reg_PHY_RX_CAL_X_NUM 11U
static const uint32_t _reg_PHY_RX_CAL_X[_reg_PHY_RX_CAL_X_NUM] = {
	_reg_PHY_RX_CAL_DQ0,
	_reg_PHY_RX_CAL_DQ1,
	_reg_PHY_RX_CAL_DQ2,
	_reg_PHY_RX_CAL_DQ3,
	_reg_PHY_RX_CAL_DQ4,
	_reg_PHY_RX_CAL_DQ5,
	_reg_PHY_RX_CAL_DQ6,
	_reg_PHY_RX_CAL_DQ7,
	_reg_PHY_RX_CAL_DM,
	_reg_PHY_RX_CAL_DQS,
	_reg_PHY_RX_CAL_FDBK
};

#define _reg_PHY_CLK_WRX_SLAVE_DELAY_NUM 10U
static const uint32_t _reg_PHY_CLK_WRX_SLAVE_DELAY
	[_reg_PHY_CLK_WRX_SLAVE_DELAY_NUM] = {
	_reg_PHY_CLK_WRDQ0_SLAVE_DELAY,
	_reg_PHY_CLK_WRDQ1_SLAVE_DELAY,
	_reg_PHY_CLK_WRDQ2_SLAVE_DELAY,
	_reg_PHY_CLK_WRDQ3_SLAVE_DELAY,
	_reg_PHY_CLK_WRDQ4_SLAVE_DELAY,
	_reg_PHY_CLK_WRDQ5_SLAVE_DELAY,
	_reg_PHY_CLK_WRDQ6_SLAVE_DELAY,
	_reg_PHY_CLK_WRDQ7_SLAVE_DELAY,
	_reg_PHY_CLK_WRDM_SLAVE_DELAY,
	_reg_PHY_CLK_WRDQS_SLAVE_DELAY
};

#define _reg_PHY_RDDQS_X_FALL_SLAVE_DELAY_NUM 9U
static const uint32_t _reg_PHY_RDDQS_X_FALL_SLAVE_DELAY
	[_reg_PHY_RDDQS_X_FALL_SLAVE_DELAY_NUM] = {
	_reg_PHY_RDDQS_DQ0_FALL_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ1_FALL_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ2_FALL_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ3_FALL_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ4_FALL_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ5_FALL_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ6_FALL_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ7_FALL_SLAVE_DELAY,
	_reg_PHY_RDDQS_DM_FALL_SLAVE_DELAY
};

#define _reg_PHY_RDDQS_X_RISE_SLAVE_DELAY_NUM 9U
static const uint32_t _reg_PHY_RDDQS_X_RISE_SLAVE_DELAY
	[_reg_PHY_RDDQS_X_RISE_SLAVE_DELAY_NUM] = {
	_reg_PHY_RDDQS_DQ0_RISE_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ1_RISE_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ2_RISE_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ3_RISE_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ4_RISE_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ5_RISE_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ6_RISE_SLAVE_DELAY,
	_reg_PHY_RDDQS_DQ7_RISE_SLAVE_DELAY,
	_reg_PHY_RDDQS_DM_RISE_SLAVE_DELAY
};

#define _reg_PHY_PAD_TERM_X_NUM 8U
static const uint32_t _reg_PHY_PAD_TERM_X[_reg_PHY_PAD_TERM_X_NUM] = {
	_reg_PHY_PAD_FDBK_TERM,
	_reg_PHY_PAD_DATA_TERM,
	_reg_PHY_PAD_DQS_TERM,
	_reg_PHY_PAD_ADDR_TERM,
	_reg_PHY_PAD_CLK_TERM,
	_reg_PHY_PAD_CKE_TERM,
	_reg_PHY_PAD_RST_TERM,
	_reg_PHY_PAD_CS_TERM
};

#define _reg_PHY_CLK_CACS_SLAVE_DELAY_X_NUM 10U
static const uint32_t _reg_PHY_CLK_CACS_SLAVE_DELAY_X
	[_reg_PHY_CLK_CACS_SLAVE_DELAY_X_NUM] = {
	_reg_PHY_ADR0_CLK_WR_SLAVE_DELAY,
	_reg_PHY_ADR1_CLK_WR_SLAVE_DELAY,
	_reg_PHY_ADR2_CLK_WR_SLAVE_DELAY,
	_reg_PHY_ADR3_CLK_WR_SLAVE_DELAY,
	_reg_PHY_ADR4_CLK_WR_SLAVE_DELAY,
	_reg_PHY_ADR5_CLK_WR_SLAVE_DELAY,

	_reg_PHY_GRP_SLAVE_DELAY_0,
	_reg_PHY_GRP_SLAVE_DELAY_1,
	_reg_PHY_GRP_SLAVE_DELAY_2,
	_reg_PHY_GRP_SLAVE_DELAY_3
};

/* Prototypes */
static inline uint32_t vch_nxt(uint32_t pos);
static void cpg_write_32(uint32_t a, uint32_t v);
static void pll3_control(uint32_t high);
static inline void dsb_sev(void);
static void wait_dbcmd(void);
static void send_dbcmd(uint32_t cmd);
static uint32_t reg_ddrphy_read(uint32_t phyno, uint32_t regadd);
static void reg_ddrphy_write(uint32_t phyno, uint32_t regadd, uint32_t regdata);
static void reg_ddrphy_write_a(uint32_t regadd, uint32_t regdata);
static inline uint32_t ddr_regdef(uint32_t _regdef);
static inline uint32_t ddr_regdef_adr(uint32_t _regdef);
static inline uint32_t ddr_regdef_lsb(uint32_t _regdef);
static void ddr_setval_s(uint32_t ch, uint32_t slice, uint32_t _regdef,
			 uint32_t val);
static uint32_t ddr_getval_s(uint32_t ch, uint32_t slice, uint32_t _regdef);
static void ddr_setval(uint32_t ch, uint32_t regdef, uint32_t val);
static void ddr_setval_ach_s(uint32_t slice, uint32_t regdef, uint32_t val);
static void ddr_setval_ach(uint32_t regdef, uint32_t val);
static void ddr_setval_ach_as(uint32_t regdef, uint32_t val);
static uint32_t ddr_getval(uint32_t ch, uint32_t regdef);
static uint32_t ddr_getval_ach(uint32_t regdef, uint32_t *p);
static uint32_t ddr_getval_ach_as(uint32_t regdef, uint32_t *p);
static void _tblcopy(uint32_t *to, const uint32_t *from, uint32_t size);
static void ddrtbl_setval(uint32_t *tbl, uint32_t _regdef, uint32_t val);
static uint32_t ddrtbl_getval(uint32_t *tbl, uint32_t _regdef);
static uint32_t ddrphy_regif_chk(void);
static inline void ddrphy_regif_idle(void);
static uint16_t _f_scale(uint32_t _ddr_mbps, uint32_t _ddr_mbpsdiv, uint32_t ps,
			 uint16_t cyc);
static void _f_scale_js2(uint32_t _ddr_mbps, uint32_t _ddr_mbpsdiv,
			 uint16_t *_js2);
static int16_t _f_scale_adj(int16_t ps);
static void ddrtbl_load(void);
static void ddr_config_sub(void);
static void ddr_config(void);
static void dbsc_regset(void);
static void dbsc_regset_post(void);
static uint32_t dfi_init_start(void);
static void change_lpddr4_en(uint32_t mode);
static uint32_t set_term_code(void);
static void ddr_register_set(void);
static inline uint32_t wait_freqchgreq(uint32_t assert);
static inline void set_freqchgack(uint32_t assert);
static inline void set_dfifrequency(uint32_t freq);
static uint32_t pll3_freq(uint32_t on);
static void update_dly(void);
static uint32_t pi_training_go(void);
static uint32_t init_ddr(void);
static uint32_t swlvl1(uint32_t ddr_csn, uint32_t reg_cs, uint32_t reg_kick);
static uint32_t wdqdm_man1(void);
static uint32_t wdqdm_man(void);
static uint32_t rdqdm_man1(void);
static uint32_t rdqdm_man(void);

static int32_t _find_change(uint64_t val, uint32_t dir);
static uint32_t _rx_offset_cal_updn(uint32_t code);
static uint32_t rx_offset_cal(void);
static uint32_t rx_offset_cal_hw(void);
static void adjust_wpath_latency(void);

struct ddrt_data {
	int32_t init_temp;	/* Initial Temperature (do) */
	uint32_t init_cal[4U];	/* Initial io-code (4 is for G2H) */
	uint32_t tcomp_cal[4U];	/* Temp. compensated io-code (4 is for G2H) */
};

static struct ddrt_data tcal;

static void pvtcode_update(void);
static void pvtcode_update2(void);
static void ddr_padcal_tcompensate_getinit(uint32_t override);

#ifndef DDR_FAST_INIT
static uint32_t rdqdm_le[DRAM_CH_CNT][CS_CNT][SLICE_CNT * 2U][9U];
static uint32_t rdqdm_te[DRAM_CH_CNT][CS_CNT][SLICE_CNT * 2U][9U];
static uint32_t rdqdm_nw[DRAM_CH_CNT][CS_CNT][SLICE_CNT * 2U][9U];
static uint32_t rdqdm_win[DRAM_CH_CNT][CS_CNT][SLICE_CNT];
static uint32_t rdqdm_st[DRAM_CH_CNT][CS_CNT][SLICE_CNT * 2U];
static void rdqdm_clr1(uint32_t ch, uint32_t ddr_csn);
static uint32_t rdqdm_ana1(uint32_t ch, uint32_t ddr_csn);

static uint32_t wdqdm_le[DRAM_CH_CNT][CS_CNT][SLICE_CNT][9U];
static uint32_t wdqdm_te[DRAM_CH_CNT][CS_CNT][SLICE_CNT][9U];
static uint32_t wdqdm_dly[DRAM_CH_CNT][CS_CNT][SLICE_CNT][9U];
static uint32_t wdqdm_st[DRAM_CH_CNT][CS_CNT][SLICE_CNT];
static uint32_t wdqdm_win[DRAM_CH_CNT][CS_CNT][SLICE_CNT];
static void wdqdm_clr1(uint32_t ch, uint32_t ddr_csn);
static uint32_t wdqdm_ana1(uint32_t ch, uint32_t ddr_csn);
#endif/* DDR_FAST_INIT */

/* macro for channel selection loop */
static inline uint32_t vch_nxt(uint32_t pos)
{
	uint32_t posn;

	for (posn = pos; posn < DRAM_CH_CNT; posn++) {
		if ((ddr_phyvalid & (1U << posn)) != 0U) {
			break;
		}
	}
	return posn;
}

#define foreach_vch(ch) \
for (ch = vch_nxt(0U); ch < DRAM_CH_CNT; ch = vch_nxt(ch + 1U))

#define foreach_ech(ch) \
for (ch = 0U; ch < DRAM_CH_CNT; ch++)

/* Printing functions */
#define MSG_LF(...)

/* clock settings, reset control */
static void cpg_write_32(uint32_t a, uint32_t v)
{
	mmio_write_32(CPG_CPGWPR, ~v);
	mmio_write_32(a, v);
}

static void wait_for_pll3_status_bit_turned_on(void)
{
	uint32_t data_l;

	do {
		data_l = mmio_read_32(CPG_PLLECR);
	} while ((data_l & CPG_PLLECR_PLL3ST_BIT) == 0);
	dsb_sev();
}

static void pll3_control(uint32_t high)
{
	uint32_t data_l, data_div, data_mul, tmp_div;

	if (high != 0U) {
		tmp_div = 3999U * brd_clkdiv * (brd_clkdiva + 1U) /
			(brd_clk * ddr_mul) / 2U;
		data_mul = ((ddr_mul * tmp_div) - 1U) << 24U;
		pll3_mode = 1U;
		loop_max = 2U;
	} else {
		tmp_div = 3999U * brd_clkdiv * (brd_clkdiva + 1U) /
			(brd_clk * ddr0800_mul) / 2U;
		data_mul = ((ddr0800_mul * tmp_div) - 1U) << 24U;
		pll3_mode = 0U;
		loop_max = 8U;
	}

	switch (tmp_div) {
	case 1:
		data_div = 0U;
		break;
	case 2:
	case 3:
	case 4:
		data_div = tmp_div;
		break;
	default:
		data_div = 6U;
		data_mul = (data_mul * tmp_div) / 3U;
		break;
	}
	data_mul = data_mul | (brd_clkdiva << 7);

	/* PLL3 disable */
	data_l = mmio_read_32(CPG_PLLECR) & ~CPG_PLLECR_PLL3E_BIT;
	cpg_write_32(CPG_PLLECR, data_l);
	dsb_sev();

	if (prr_product == PRR_PRODUCT_M3) {
		/* PLL3 DIV resetting(Lowest value:3) */
		data_l = 0x00030003U | (0xFF80FF80U & mmio_read_32(CPG_FRQCRD));
		cpg_write_32(CPG_FRQCRD, data_l);
		dsb_sev();

		/* zb3 clk stop */
		data_l = CPG_ZB3CKCR_ZB3ST_BIT | mmio_read_32(CPG_ZB3CKCR);
		cpg_write_32(CPG_ZB3CKCR, data_l);
		dsb_sev();

		/* PLL3 enable */
		data_l = CPG_PLLECR_PLL3E_BIT | mmio_read_32(CPG_PLLECR);
		cpg_write_32(CPG_PLLECR, data_l);
		dsb_sev();

		wait_for_pll3_status_bit_turned_on();

		/* PLL3 DIV resetting (Highest value:0) */
		data_l = (0xFF80FF80U & mmio_read_32(CPG_FRQCRD));
		cpg_write_32(CPG_FRQCRD, data_l);
		dsb_sev();

		/* DIV SET KICK */
		data_l = CPG_FRQCRB_KICK_BIT | mmio_read_32(CPG_FRQCRB);
		cpg_write_32(CPG_FRQCRB, data_l);
		dsb_sev();

		/* PLL3 multiplier set */
		cpg_write_32(CPG_PLL3CR, data_mul);
		dsb_sev();

		wait_for_pll3_status_bit_turned_on();

		/* PLL3 DIV resetting(Target value) */
		data_l = (data_div << 16U) | data_div |
			 (mmio_read_32(CPG_FRQCRD) & 0xFF80FF80U);
		cpg_write_32(CPG_FRQCRD, data_l);
		dsb_sev();

		/* DIV SET KICK */
		data_l = CPG_FRQCRB_KICK_BIT | mmio_read_32(CPG_FRQCRB);
		cpg_write_32(CPG_FRQCRB, data_l);
		dsb_sev();

		wait_for_pll3_status_bit_turned_on();

		/* zb3 clk start */
		data_l = (~CPG_ZB3CKCR_ZB3ST_BIT) & mmio_read_32(CPG_ZB3CKCR);
		cpg_write_32(CPG_ZB3CKCR, data_l);
		dsb_sev();
	}
}

/* barrier */
static inline void dsb_sev(void)
{
	__asm__ __volatile__("dsb sy");
}

/* DDR memory register access */
static void wait_dbcmd(void)
{
	uint32_t data_l;
	/* dummy read */
	data_l = mmio_read_32(DBSC_DBCMD);
	dsb_sev();
	while (true) {
		/* wait DBCMD 1=busy, 0=ready */
		data_l = mmio_read_32(DBSC_DBWAIT);
		dsb_sev();
		if ((data_l & 0x00000001U) == 0x00U) {
			break;
		}
	}
}

static void send_dbcmd(uint32_t cmd)
{
	/* dummy read */
	wait_dbcmd();
	mmio_write_32(DBSC_DBCMD, cmd);
	dsb_sev();
}

static void dbwait_loop(uint32_t wait_loop)
{
	uint32_t i;

	for (i = 0U; i < wait_loop; i++) {
		wait_dbcmd();
	}
}

/* DDRPHY register access (raw) */
static uint32_t reg_ddrphy_read(uint32_t phyno, uint32_t regadd)
{
	uint32_t val;
	uint32_t loop;

	val = 0U;
	mmio_write_32(DBSC_DBPDRGA(phyno), regadd);
	dsb_sev();

	while (mmio_read_32(DBSC_DBPDRGA(phyno)) != regadd) {
		dsb_sev();
	}
	dsb_sev();

	for (loop = 0U; loop < loop_max; loop++) {
		val = mmio_read_32(DBSC_DBPDRGD(phyno));
		dsb_sev();
	}

	return val;
}

static void reg_ddrphy_write(uint32_t phyno, uint32_t regadd, uint32_t regdata)
{
	uint32_t loop;

	mmio_write_32(DBSC_DBPDRGA(phyno), regadd);
	dsb_sev();
	for (loop = 0U; loop < loop_max; loop++) {
		mmio_read_32(DBSC_DBPDRGA(phyno));
		dsb_sev();
	}
	mmio_write_32(DBSC_DBPDRGD(phyno), regdata);
	dsb_sev();

	for (loop = 0U; loop < loop_max; loop++) {
		mmio_read_32(DBSC_DBPDRGD(phyno));
		dsb_sev();
	}
}

static void reg_ddrphy_write_a(uint32_t regadd, uint32_t regdata)
{
	uint32_t ch;
	uint32_t loop;

	foreach_vch(ch) {
		mmio_write_32(DBSC_DBPDRGA(ch), regadd);
		dsb_sev();
	}

	foreach_vch(ch) {
		mmio_write_32(DBSC_DBPDRGD(ch), regdata);
		dsb_sev();
	}

	for (loop = 0U; loop < loop_max; loop++) {
		mmio_read_32(DBSC_DBPDRGD(0));
		dsb_sev();
	}
}

static inline void ddrphy_regif_idle(void)
{
	reg_ddrphy_read(0U, ddr_regdef_adr(_reg_PI_INT_STATUS));
	dsb_sev();
}

/* DDRPHY register access (field modify) */
static inline uint32_t ddr_regdef(uint32_t _regdef)
{
	return p_ddr_regdef_tbl[_regdef];
}

static inline uint32_t ddr_regdef_adr(uint32_t _regdef)
{
	return DDR_REGDEF_ADR(p_ddr_regdef_tbl[_regdef]);
}

static inline uint32_t ddr_regdef_lsb(uint32_t _regdef)
{
	return DDR_REGDEF_LSB(p_ddr_regdef_tbl[_regdef]);
}

static void ddr_setval_s(uint32_t ch, uint32_t slice, uint32_t _regdef,
			 uint32_t val)
{
	uint32_t adr;
	uint32_t lsb;
	uint32_t len;
	uint32_t msk;
	uint32_t tmp;
	uint32_t regdef;

	regdef = ddr_regdef(_regdef);
	adr = DDR_REGDEF_ADR(regdef) + 0x80U * slice;
	len = DDR_REGDEF_LEN(regdef);
	lsb = DDR_REGDEF_LSB(regdef);
	if (len == 0x20U) {
		msk = 0xffffffffU;
	} else {
		msk = ((1U << len) - 1U) << lsb;
	}

	tmp = reg_ddrphy_read(ch, adr);
	tmp = (tmp & (~msk)) | ((val << lsb) & msk);
	reg_ddrphy_write(ch, adr, tmp);
}

static uint32_t ddr_getval_s(uint32_t ch, uint32_t slice, uint32_t _regdef)
{
	uint32_t adr;
	uint32_t lsb;
	uint32_t len;
	uint32_t msk;
	uint32_t tmp;
	uint32_t regdef;

	regdef = ddr_regdef(_regdef);
	adr = DDR_REGDEF_ADR(regdef) + 0x80U * slice;
	len = DDR_REGDEF_LEN(regdef);
	lsb = DDR_REGDEF_LSB(regdef);
	if (len == 0x20U) {
		msk = 0xffffffffU;
	} else {
		msk = ((1U << len) - 1U);
	}

	tmp = reg_ddrphy_read(ch, adr);
	tmp = (tmp >> lsb) & msk;

	return tmp;
}

static void ddr_setval(uint32_t ch, uint32_t regdef, uint32_t val)
{
	ddr_setval_s(ch, 0U, regdef, val);
}

static void ddr_setval_ach_s(uint32_t slice, uint32_t regdef, uint32_t val)
{
	uint32_t ch;

	foreach_vch(ch) {
	    ddr_setval_s(ch, slice, regdef, val);
	}
}

static void ddr_setval_ach(uint32_t regdef, uint32_t val)
{
	ddr_setval_ach_s(0U, regdef, val);
}

static void ddr_setval_ach_as(uint32_t regdef, uint32_t val)
{
	uint32_t slice;

	for (slice = 0U; slice < SLICE_CNT; slice++) {
		ddr_setval_ach_s(slice, regdef, val);
	}
}

static uint32_t ddr_getval(uint32_t ch, uint32_t regdef)
{
	return ddr_getval_s(ch, 0U, regdef);
}

static uint32_t ddr_getval_ach(uint32_t regdef, uint32_t *p)
{
	uint32_t ch;

	foreach_vch(ch) {
	    p[ch] = ddr_getval_s(ch, 0U, regdef);
	}
	return p[0U];
}

static uint32_t ddr_getval_ach_as(uint32_t regdef, uint32_t *p)
{
	uint32_t ch, slice;
	uint32_t *pp;

	pp = p;
	foreach_vch(ch) {
		for (slice = 0U; slice < SLICE_CNT; slice++) {
			*pp++ = ddr_getval_s(ch, slice, regdef);
		}
	}
	return p[0U];
}

/* handling functions for setting ddrphy value table */
static void _tblcopy(uint32_t *to, const uint32_t *from, uint32_t size)
{
	uint32_t i;

	for (i = 0U; i < size; i++) {
		to[i] = from[i];
	}
}

static void ddrtbl_setval(uint32_t *tbl, uint32_t _regdef, uint32_t val)
{
	uint32_t adr;
	uint32_t lsb;
	uint32_t len;
	uint32_t msk;
	uint32_t tmp;
	uint32_t adrmsk;
	uint32_t regdef;

	regdef = ddr_regdef(_regdef);
	adr = DDR_REGDEF_ADR(regdef);
	len = DDR_REGDEF_LEN(regdef);
	lsb = DDR_REGDEF_LSB(regdef);
	if (len == 0x20U) {
		msk = 0xffffffffU;
	} else {
		msk = ((1U << len) - 1U) << lsb;
	}

	if (adr < 0x400U) {
		adrmsk = 0xffU;
	} else {
		adrmsk = 0x7fU;
	}

	tmp = tbl[adr & adrmsk];
	tmp = (tmp & (~msk)) | ((val << lsb) & msk);
	tbl[adr & adrmsk] = tmp;
}

static uint32_t ddrtbl_getval(uint32_t *tbl, uint32_t _regdef)
{
	uint32_t adr;
	uint32_t lsb;
	uint32_t len;
	uint32_t msk;
	uint32_t tmp;
	uint32_t adrmsk;
	uint32_t regdef;

	regdef = ddr_regdef(_regdef);
	adr = DDR_REGDEF_ADR(regdef);
	len = DDR_REGDEF_LEN(regdef);
	lsb = DDR_REGDEF_LSB(regdef);
	if (len == 0x20U) {
		msk = 0xffffffffU;
	} else {
		msk = ((1U << len) - 1U);
	}

	if (adr < 0x400U) {
		adrmsk = 0xffU;
	} else {
		adrmsk = 0x7fU;
	}

	tmp = tbl[adr & adrmsk];
	tmp = (tmp >> lsb) & msk;

	return tmp;
}

/* DDRPHY register access handling */
static uint32_t ddrphy_regif_chk(void)
{
	uint32_t tmp_ach[DRAM_CH_CNT];
	uint32_t ch;
	uint32_t err;
	uint32_t PI_VERSION_CODE;

	if (prr_product == PRR_PRODUCT_M3) {
		PI_VERSION_CODE = 0x2041U; /* G2M */
	}

	ddr_getval_ach(_reg_PI_VERSION, (uint32_t *)tmp_ach);
	err = 0U;
	foreach_vch(ch) {
		if (tmp_ach[ch] != PI_VERSION_CODE) {
			err = 1U;
		}
	}
	return err;
}

/* functions and parameters for timing setting */
struct _jedec_spec1 {
	uint16_t fx3;
	uint8_t rlwodbi;
	uint8_t rlwdbi;
	uint8_t WL;
	uint8_t nwr;
	uint8_t nrtp;
	uint8_t odtlon;
	uint8_t MR1;
	uint8_t MR2;
};

#define JS1_USABLEC_SPEC_LO 2U
#define JS1_USABLEC_SPEC_HI 5U
#define JS1_FREQ_TBL_NUM 8
#define JS1_MR1(f) (0x04U | ((f) << 4U))
#define JS1_MR2(f) (0x00U | ((f) << 3U) | (f))
static const struct _jedec_spec1 js1[JS1_FREQ_TBL_NUM] = {
	/* 533.333Mbps */
	{  800U,  6U,  6U,  4U,  6U,  8U, 0U, JS1_MR1(0U), JS1_MR2(0U) | 0x40U },
	/* 1066.666Mbps */
	{ 1600U, 10U, 12U,  8U, 10U,  8U, 0U, JS1_MR1(1U), JS1_MR2(1U) | 0x40U },
	/* 1600.000Mbps */
	{ 2400U, 14U, 16U, 12U, 16U,  8U, 6U, JS1_MR1(2U), JS1_MR2(2U) | 0x40U },
	/* 2133.333Mbps */
	{ 3200U, 20U, 22U, 10U, 20U,  8U, 4U, JS1_MR1(3U), JS1_MR2(3U) },
	/* 2666.666Mbps */
	{ 4000U, 24U, 28U, 12U, 24U, 10U, 4U, JS1_MR1(4U), JS1_MR2(4U) },
	/* 3200.000Mbps */
	{ 4800U, 28U, 32U, 14U, 30U, 12U, 6U, JS1_MR1(5U), JS1_MR2(5U) },
	/* 3733.333Mbps */
	{ 5600U, 32U, 36U, 16U, 34U, 14U, 6U, JS1_MR1(6U), JS1_MR2(6U) },
	/* 4266.666Mbps */
	{ 6400U, 36U, 40U, 18U, 40U, 16U, 8U, JS1_MR1(7U), JS1_MR2(7U) }
};

struct _jedec_spec2 {
	uint16_t ps;
	uint16_t cyc;
};

#define js2_tsr 0
#define js2_txp 1
#define js2_trtp 2
#define js2_trcd 3
#define js2_trppb 4
#define js2_trpab 5
#define js2_tras 6
#define js2_twr 7
#define js2_twtr 8
#define js2_trrd 9
#define js2_tppd 10
#define js2_tfaw 11
#define js2_tdqsck 12
#define js2_tckehcmd 13
#define js2_tckelcmd 14
#define js2_tckelpd 15
#define js2_tmrr 16
#define js2_tmrw 17
#define js2_tmrd 18
#define js2_tzqcalns 19
#define js2_tzqlat 20
#define js2_tiedly 21
#define js2_tODTon_min 22
#define JS2_TBLCNT 23

#define js2_trcpb (JS2_TBLCNT)
#define js2_trcab (JS2_TBLCNT + 1)
#define js2_trfcab (JS2_TBLCNT + 2)
#define JS2_CNT (JS2_TBLCNT + 3)

#ifndef JS2_DERATE
#define JS2_DERATE 0
#endif
static const struct _jedec_spec2 jedec_spec2[2][JS2_TBLCNT] = {
	{
/* tSR */	{ 15000, 3 },
/* tXP */	{ 7500, 3 },
/* tRTP */	{ 7500, 8 },
/* tRCD */	{ 18000, 4 },
/* tRPpb */	{ 18000, 3 },
/* tRPab */	{ 21000, 3 },
/* tRAS  */	{ 42000, 3 },
/* tWR   */	{ 18000, 4 },
/* tWTR  */	{ 10000, 8 },
/* tRRD  */	{ 10000, 4 },
/* tPPD  */	{ 0, 0 },
/* tFAW  */	{ 40000, 0 },
/* tDQSCK */	{ 3500, 0 },
/* tCKEHCMD */	{ 7500, 3 },
/* tCKELCMD */	{ 7500, 3 },
/* tCKELPD */	{ 7500, 3 },
/* tMRR */	{ 0, 8 },
/* tMRW */	{ 10000, 10 },
/* tMRD */	{ 14000, 10 },
/* tZQCALns */	{ 1000 * 10, 0 },
/* tZQLAT */	{ 30000, 10 },
/* tIEdly */	{ 12500, 0 },
/* tODTon_min */{ 1500, 0 }
	 }, {
/* tSR */	{ 15000, 3 },
/* tXP */	{ 7500, 3 },
/* tRTP */	{ 7500, 8 },
/* tRCD */	{ 19875, 4 },
/* tRPpb */	{ 19875, 3 },
/* tRPab */	{ 22875, 3 },
/* tRAS */	{ 43875, 3 },
/* tWR */	{ 18000, 4 },
/* tWTR */	{ 10000, 8 },
/* tRRD */	{ 11875, 4 },
/* tPPD */	{ 0, 0 },
/* tFAW */	{ 40000, 0 },
/* tDQSCK */	{ 3600, 0 },
/* tCKEHCMD */	{ 7500, 3 },
/* tCKELCMD */	{ 7500, 3 },
/* tCKELPD */	{ 7500, 3 },
/* tMRR */	{ 0, 8 },
/* tMRW */	{ 10000, 10 },
/* tMRD */	{ 14000, 10 },
/* tZQCALns */	{ 1000 * 10, 0 },
/* tZQLAT */	{ 30000, 10 },
/* tIEdly */	{ 12500, 0 },
/* tODTon_min */{ 1500, 0 }
	}
};

static const uint16_t jedec_spec2_trfc_ab[7] = {
	/* 4Gb, 6Gb,  8Gb,  12Gb, 16Gb, 24Gb(non), 32Gb(non) */
	 130U, 180U, 180U, 280U, 280U, 560U, 560U
};

static uint32_t js1_ind;
static uint16_t js2[JS2_CNT];
static uint8_t RL;
static uint8_t WL;

static uint16_t _f_scale(uint32_t _ddr_mbps, uint32_t _ddr_mbpsdiv, uint32_t ps,
			 uint16_t cyc)
{
	uint16_t ret = cyc;
	uint32_t tmp;
	uint32_t div;

	tmp = (((uint32_t)(ps) + 9U) / 10U) * _ddr_mbps;
	div = tmp / (200000U * _ddr_mbpsdiv);
	if (tmp != (div * 200000U * _ddr_mbpsdiv)) {
		div = div + 1U;
	}

	if (div > cyc) {
		ret = (uint16_t)div;
	}

	return ret;
}

static void _f_scale_js2(uint32_t _ddr_mbps, uint32_t _ddr_mbpsdiv,
			 uint16_t *_js2)
{
	int i;

	for (i = 0; i < JS2_TBLCNT; i++) {
		_js2[i] = _f_scale(_ddr_mbps, _ddr_mbpsdiv,
				   jedec_spec2[JS2_DERATE][i].ps,
				   jedec_spec2[JS2_DERATE][i].cyc);
	}

	_js2[js2_trcpb] = _js2[js2_tras] + _js2[js2_trppb];
	_js2[js2_trcab] = _js2[js2_tras] + _js2[js2_trpab];
}

/* scaler for DELAY value */
static int16_t _f_scale_adj(int16_t ps)
{
	int32_t tmp;
	/*
	 * tmp = (int32_t)512 * ps * ddr_mbps /2 / ddr_mbpsdiv / 1000 / 1000;
	 *     = ps * ddr_mbps /2 / ddr_mbpsdiv *512 / 8 / 8 / 125 / 125
	 *     = ps * ddr_mbps / ddr_mbpsdiv *4 / 125 / 125
	 */
	tmp = (int32_t)4 * (int32_t)ps * (int32_t)ddr_mbps /
		(int32_t)ddr_mbpsdiv;
	tmp = (int32_t)tmp / (int32_t)15625;

	return (int16_t)tmp;
}

static const uint32_t reg_pi_mr1_data_fx_csx[2U][CSAB_CNT] = {
	{
	 _reg_PI_MR1_DATA_F0_0,
	 _reg_PI_MR1_DATA_F0_1,
	 _reg_PI_MR1_DATA_F0_2,
	 _reg_PI_MR1_DATA_F0_3},
	{
	 _reg_PI_MR1_DATA_F1_0,
	 _reg_PI_MR1_DATA_F1_1,
	 _reg_PI_MR1_DATA_F1_2,
	 _reg_PI_MR1_DATA_F1_3}
};

static const uint32_t reg_pi_mr2_data_fx_csx[2U][CSAB_CNT] = {
	{
	 _reg_PI_MR2_DATA_F0_0,
	 _reg_PI_MR2_DATA_F0_1,
	 _reg_PI_MR2_DATA_F0_2,
	 _reg_PI_MR2_DATA_F0_3},
	{
	 _reg_PI_MR2_DATA_F1_0,
	 _reg_PI_MR2_DATA_F1_1,
	 _reg_PI_MR2_DATA_F1_2,
	 _reg_PI_MR2_DATA_F1_3}
};

static const uint32_t reg_pi_mr3_data_fx_csx[2U][CSAB_CNT] = {
	{
	 _reg_PI_MR3_DATA_F0_0,
	 _reg_PI_MR3_DATA_F0_1,
	 _reg_PI_MR3_DATA_F0_2,
	 _reg_PI_MR3_DATA_F0_3},
	{
	 _reg_PI_MR3_DATA_F1_0,
	 _reg_PI_MR3_DATA_F1_1,
	 _reg_PI_MR3_DATA_F1_2,
	 _reg_PI_MR3_DATA_F1_3}
};

static const uint32_t reg_pi_mr11_data_fx_csx[2U][CSAB_CNT] = {
	{
	 _reg_PI_MR11_DATA_F0_0,
	 _reg_PI_MR11_DATA_F0_1,
	 _reg_PI_MR11_DATA_F0_2,
	 _reg_PI_MR11_DATA_F0_3},
	{
	 _reg_PI_MR11_DATA_F1_0,
	 _reg_PI_MR11_DATA_F1_1,
	 _reg_PI_MR11_DATA_F1_2,
	 _reg_PI_MR11_DATA_F1_3}
};

static const uint32_t reg_pi_mr12_data_fx_csx[2U][CSAB_CNT] = {
	{
	 _reg_PI_MR12_DATA_F0_0,
	 _reg_PI_MR12_DATA_F0_1,
	 _reg_PI_MR12_DATA_F0_2,
	 _reg_PI_MR12_DATA_F0_3},
	{
	 _reg_PI_MR12_DATA_F1_0,
	 _reg_PI_MR12_DATA_F1_1,
	 _reg_PI_MR12_DATA_F1_2,
	 _reg_PI_MR12_DATA_F1_3}
};

static const uint32_t reg_pi_mr14_data_fx_csx[2U][CSAB_CNT] = {
	{
	 _reg_PI_MR14_DATA_F0_0,
	 _reg_PI_MR14_DATA_F0_1,
	 _reg_PI_MR14_DATA_F0_2,
	 _reg_PI_MR14_DATA_F0_3},
	{
	 _reg_PI_MR14_DATA_F1_0,
	 _reg_PI_MR14_DATA_F1_1,
	 _reg_PI_MR14_DATA_F1_2,
	 _reg_PI_MR14_DATA_F1_3}
};

/*
 * regif pll w/a   ( REGIF G2M WA )
 */
static void regif_pll_wa(void)
{
	uint32_t ch;
	uint32_t reg_ofs;

	/*  PLL setting for PHY : G2M */
	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_PLL_WAIT),
			   (0x5064U <<
			    ddr_regdef_lsb(_reg_PHY_PLL_WAIT)));

	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_PLL_CTRL),
			   (ddrtbl_getval(_cnf_DDR_PHY_ADR_G_REGSET,
			     _reg_PHY_PLL_CTRL_TOP) << 16) |
			   ddrtbl_getval(_cnf_DDR_PHY_ADR_G_REGSET,
					 _reg_PHY_PLL_CTRL));
	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_PLL_CTRL_CA),
			   ddrtbl_getval(_cnf_DDR_PHY_ADR_G_REGSET,
					 _reg_PHY_PLL_CTRL_CA));

	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_LP4_BOOT_PLL_CTRL),
			   (ddrtbl_getval(_cnf_DDR_PHY_ADR_G_REGSET,
			     _reg_PHY_LP4_BOOT_PLL_CTRL_CA) << 16) |
			   ddrtbl_getval(_cnf_DDR_PHY_ADR_G_REGSET,
					 _reg_PHY_LP4_BOOT_PLL_CTRL));
	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_LP4_BOOT_TOP_PLL_CTRL),
			   ddrtbl_getval(_cnf_DDR_PHY_ADR_G_REGSET,
					 _reg_PHY_LP4_BOOT_TOP_PLL_CTRL));

	reg_ofs = ddr_regdef_adr(_reg_PHY_LPDDR3_CS) - DDR_PHY_ADR_G_REGSET_OFS;
	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_LPDDR3_CS),
			   _cnf_DDR_PHY_ADR_G_REGSET[reg_ofs]);

	/* protect register interface */
	ddrphy_regif_idle();
	pll3_control(0U);

	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_DLL_RST_EN),
			   (0x01U << ddr_regdef_lsb(_reg_PHY_DLL_RST_EN)));
	ddrphy_regif_idle();

	/*
	 * init start
	 * dbdficnt0:
	 * dfi_dram_clk_disable=1
	 * dfi_frequency = 0
	 * freq_ratio = 01 (2:1)
	 * init_start =0
	 */
	foreach_vch(ch) {
	    mmio_write_32(DBSC_DBDFICNT(ch), 0x00000F10U);
	}
	dsb_sev();

	/*
	 * dbdficnt0:
	 * dfi_dram_clk_disable=1
	 * dfi_frequency = 0
	 * freq_ratio = 01 (2:1)
	 * init_start =1
	 */
	foreach_vch(ch) {
	    mmio_write_32(DBSC_DBDFICNT(ch), 0x00000F11U);
	}
	dsb_sev();

	foreach_ech(ch) {
		if ((board_cnf->phyvalid & BIT(ch)) != 0U) {
			while ((mmio_read_32(DBSC_PLL_LOCK(ch)) & 0x1fU) != 0x1fU) {
			}
		}
	}
	dsb_sev();
}

/* load table data into DDR registers */
static void ddrtbl_load(void)
{
	uint32_t i;
	uint32_t slice;
	uint32_t csab;
	uint32_t adr;
	uint32_t data_l;
	uint32_t tmp[3];
	uint16_t dataS;

	/*
	 * TIMING REGISTERS
	 * search jedec_spec1 index
	 */
	for (i = JS1_USABLEC_SPEC_LO; i < (uint32_t)JS1_FREQ_TBL_NUM - 1U; i++) {
		if ((js1[i].fx3 * 2U * ddr_mbpsdiv >= ddr_mbps * 3U) != 0U) {
			break;
		}
	}
	if (i > JS1_USABLEC_SPEC_HI) {
		js1_ind = JS1_USABLEC_SPEC_HI;
	} else {
		js1_ind = i;
	}

	if (board_cnf->dbi_en != 0U) {
		RL = js1[js1_ind].rlwdbi;
	} else {
		RL = js1[js1_ind].rlwodbi;
	}

	WL = js1[js1_ind].WL;

	/* calculate jedec_spec2 */
	_f_scale_js2(ddr_mbps, ddr_mbpsdiv, js2);

	/* PREPARE TBL */
	if (prr_product == PRR_PRODUCT_M3) {
		/*  G2M */
		_tblcopy(_cnf_DDR_PHY_SLICE_REGSET,
			 DDR_PHY_SLICE_REGSET_G2M, DDR_PHY_SLICE_REGSET_NUM_G2M);
		_tblcopy(_cnf_DDR_PHY_ADR_V_REGSET,
			 DDR_PHY_ADR_V_REGSET_G2M, DDR_PHY_ADR_V_REGSET_NUM_G2M);
		_tblcopy(_cnf_DDR_PHY_ADR_I_REGSET,
			 DDR_PHY_ADR_I_REGSET_G2M, DDR_PHY_ADR_I_REGSET_NUM_G2M);
		_tblcopy(_cnf_DDR_PHY_ADR_G_REGSET,
			 DDR_PHY_ADR_G_REGSET_G2M, DDR_PHY_ADR_G_REGSET_NUM_G2M);
		_tblcopy(_cnf_DDR_PI_REGSET,
			 DDR_PI_REGSET_G2M, DDR_PI_REGSET_NUM_G2M);

		DDR_PHY_SLICE_REGSET_OFS = DDR_PHY_SLICE_REGSET_OFS_G2M;
		DDR_PHY_ADR_V_REGSET_OFS = DDR_PHY_ADR_V_REGSET_OFS_G2M;
		DDR_PHY_ADR_I_REGSET_OFS = DDR_PHY_ADR_I_REGSET_OFS_G2M;
		DDR_PHY_ADR_G_REGSET_OFS = DDR_PHY_ADR_G_REGSET_OFS_G2M;
		DDR_PI_REGSET_OFS = DDR_PI_REGSET_OFS_G2M;
		DDR_PHY_SLICE_REGSET_SIZE = DDR_PHY_SLICE_REGSET_SIZE_G2M;
		DDR_PHY_ADR_V_REGSET_SIZE = DDR_PHY_ADR_V_REGSET_SIZE_G2M;
		DDR_PHY_ADR_I_REGSET_SIZE = DDR_PHY_ADR_I_REGSET_SIZE_G2M;
		DDR_PHY_ADR_G_REGSET_SIZE = DDR_PHY_ADR_G_REGSET_SIZE_G2M;
		DDR_PI_REGSET_SIZE = DDR_PI_REGSET_SIZE_G2M;
		DDR_PHY_SLICE_REGSET_NUM = DDR_PHY_SLICE_REGSET_NUM_G2M;
		DDR_PHY_ADR_V_REGSET_NUM = DDR_PHY_ADR_V_REGSET_NUM_G2M;
		DDR_PHY_ADR_I_REGSET_NUM = DDR_PHY_ADR_I_REGSET_NUM_G2M;
		DDR_PHY_ADR_G_REGSET_NUM = DDR_PHY_ADR_G_REGSET_NUM_G2M;
		DDR_PI_REGSET_NUM = DDR_PI_REGSET_NUM_G2M;

		DDR_PHY_ADR_I_NUM = 2U;
	}

	/* on fly gate adjust */
	if ((prr_product == PRR_PRODUCT_M3) && (prr_cut == PRR_PRODUCT_10)) {
		ddrtbl_setval(_cnf_DDR_PHY_SLICE_REGSET,
			      _reg_ON_FLY_GATE_ADJUST_EN, 0x00);
	}

	/* Adjust PI parameters */
#ifdef _def_LPDDR4_ODT
	for (i = 0U; i < 2U; i++) {
		for (csab = 0U; csab < CSAB_CNT; csab++) {
			ddrtbl_setval(_cnf_DDR_PI_REGSET,
				      reg_pi_mr11_data_fx_csx[i][csab],
				      _def_LPDDR4_ODT);
		}
	}
#endif /* _def_LPDDR4_ODT */

#ifdef _def_LPDDR4_VREFCA
	for (i = 0U; i < 2U; i++) {
		for (csab = 0U; csab < CSAB_CNT; csab++) {
			ddrtbl_setval(_cnf_DDR_PI_REGSET,
				      reg_pi_mr12_data_fx_csx[i][csab],
				      _def_LPDDR4_VREFCA);
		}
	}
#endif /* _def_LPDDR4_VREFCA */

	if ((js2[js2_tiedly]) >= 0x0eU) {
		dataS = 0x0eU;
	} else {
		dataS = js2[js2_tiedly];
	}

	ddrtbl_setval(_cnf_DDR_PHY_SLICE_REGSET, _reg_PHY_RDDATA_EN_DLY, dataS);
	ddrtbl_setval(_cnf_DDR_PHY_SLICE_REGSET, _reg_PHY_RDDATA_EN_TSEL_DLY,
		      (dataS - 2U));
	ddrtbl_setval(_cnf_DDR_PI_REGSET, _reg_PI_RDLAT_ADJ_F1, RL - dataS);

	if (ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET, _reg_PHY_WRITE_PATH_LAT_ADD) != 0U) {
		data_l = WL - 1U;
	} else {
		data_l = WL;
	}
	ddrtbl_setval(_cnf_DDR_PI_REGSET, _reg_PI_WRLAT_ADJ_F1, data_l - 2U);
	ddrtbl_setval(_cnf_DDR_PI_REGSET, _reg_PI_WRLAT_F1, data_l);

	if (board_cnf->dbi_en != 0U) {
		ddrtbl_setval(_cnf_DDR_PHY_SLICE_REGSET, _reg_PHY_DBI_MODE,
			      0x01U);
		ddrtbl_setval(_cnf_DDR_PHY_SLICE_REGSET,
			      _reg_PHY_WDQLVL_DATADM_MASK, 0x000U);
	} else {
		ddrtbl_setval(_cnf_DDR_PHY_SLICE_REGSET, _reg_PHY_DBI_MODE,
			      0x00U);
		ddrtbl_setval(_cnf_DDR_PHY_SLICE_REGSET,
			      _reg_PHY_WDQLVL_DATADM_MASK, 0x100U);
	}

	tmp[0] = js1[js1_ind].MR1;
	tmp[1] = js1[js1_ind].MR2;
	data_l = ddrtbl_getval(_cnf_DDR_PI_REGSET, _reg_PI_MR3_DATA_F1_0);
	if (board_cnf->dbi_en != 0U) {
		tmp[2] = data_l | 0xc0U;
	} else {
		tmp[2] = data_l & (~0xc0U);
	}

	for (i = 0U; i < 2U; i++) {
		for (csab = 0U; csab < CSAB_CNT; csab++) {
			ddrtbl_setval(_cnf_DDR_PI_REGSET,
				      reg_pi_mr1_data_fx_csx[i][csab], tmp[0]);
			ddrtbl_setval(_cnf_DDR_PI_REGSET,
				      reg_pi_mr2_data_fx_csx[i][csab], tmp[1]);
			ddrtbl_setval(_cnf_DDR_PI_REGSET,
				      reg_pi_mr3_data_fx_csx[i][csab], tmp[2]);
		}
	}

	/* DDRPHY INT START */
	regif_pll_wa();
	dbwait_loop(5U);

	/* FREQ_SEL_MULTICAST & PER_CS_TRAINING_MULTICAST SET (for safety) */
	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_FREQ_SEL_MULTICAST_EN),
			   BIT(ddr_regdef_lsb(_reg_PHY_FREQ_SEL_MULTICAST_EN)));
	ddr_setval_ach_as(_reg_PHY_PER_CS_TRAINING_MULTICAST_EN, 0x01U);

	/* SET DATA SLICE TABLE */
	for (slice = 0U; slice < SLICE_CNT; slice++) {
		adr =
		    DDR_PHY_SLICE_REGSET_OFS +
		    DDR_PHY_SLICE_REGSET_SIZE * slice;
		for (i = 0U; i < DDR_PHY_SLICE_REGSET_NUM; i++) {
			reg_ddrphy_write_a(adr + i,
					   _cnf_DDR_PHY_SLICE_REGSET[i]);
		}
	}

	/* SET ADR SLICE TABLE */
	adr = DDR_PHY_ADR_V_REGSET_OFS;
	for (i = 0U; i < DDR_PHY_ADR_V_REGSET_NUM; i++) {
		reg_ddrphy_write_a(adr + i, _cnf_DDR_PHY_ADR_V_REGSET[i]);
	}

	if ((prr_product == PRR_PRODUCT_M3) &&
	    ((0x00ffffffU & (uint32_t)((board_cnf->ch[0].ca_swap) >> 40U))
	    != 0x00U)) {
		adr = DDR_PHY_ADR_I_REGSET_OFS + DDR_PHY_ADR_I_REGSET_SIZE;
		for (i = 0U; i < DDR_PHY_ADR_V_REGSET_NUM; i++) {
			reg_ddrphy_write_a(adr + i,
					   _cnf_DDR_PHY_ADR_V_REGSET[i]);
		}
		ddrtbl_setval(_cnf_DDR_PHY_ADR_G_REGSET,
			      _reg_PHY_ADR_DISABLE, 0x02);
		DDR_PHY_ADR_I_NUM -= 1U;
		ddr_phycaslice = 1U;

#ifndef _def_LPDDR4_ODT
		for (i = 0U; i < 2U; i++) {
			for (csab = 0U; csab < CSAB_CNT; csab++) {
				ddrtbl_setval(_cnf_DDR_PI_REGSET,
					      reg_pi_mr11_data_fx_csx[i][csab],
					      0x66);
			}
		}
#endif/* _def_LPDDR4_ODT */
	} else {
		ddr_phycaslice = 0U;
	}

	if (DDR_PHY_ADR_I_NUM > 0U) {
		for (slice = 0U; slice < DDR_PHY_ADR_I_NUM; slice++) {
			adr =
			    DDR_PHY_ADR_I_REGSET_OFS +
			    DDR_PHY_ADR_I_REGSET_SIZE * slice;
			for (i = 0U; i < DDR_PHY_ADR_I_REGSET_NUM; i++) {
				reg_ddrphy_write_a(adr + i,
						   _cnf_DDR_PHY_ADR_I_REGSET
						   [i]);
			}
		}
	}

	/* SET ADRCTRL SLICE TABLE */
	adr = DDR_PHY_ADR_G_REGSET_OFS;
	for (i = 0U; i < DDR_PHY_ADR_G_REGSET_NUM; i++) {
		reg_ddrphy_write_a(adr + i, _cnf_DDR_PHY_ADR_G_REGSET[i]);
	}

	/* SET PI REGISTERS */
	adr = DDR_PI_REGSET_OFS;
	for (i = 0U; i < DDR_PI_REGSET_NUM; i++) {
		reg_ddrphy_write_a(adr + i, _cnf_DDR_PI_REGSET[i]);
	}
}

/* CONFIGURE DDR REGISTERS */
static void ddr_config_sub(void)
{
	const uint32_t _par_CALVL_DEVICE_MAP = 1U;
	uint8_t high_byte[SLICE_CNT];
	uint32_t ch, slice;
	uint32_t data_l;
	uint32_t tmp;
	uint32_t i;

	foreach_vch(ch) {
		/* BOARD SETTINGS (DQ,DM,VREF_DRIVING) */
		for (slice = 0U; slice < SLICE_CNT; slice++) {
			high_byte[slice] =
			    (board_cnf->ch[ch].dqs_swap >> (4U * slice)) % 2U;
			ddr_setval_s(ch, slice, _reg_PHY_DQ_DM_SWIZZLE0,
				     board_cnf->ch[ch].dq_swap[slice]);
			ddr_setval_s(ch, slice, _reg_PHY_DQ_DM_SWIZZLE1,
				     board_cnf->ch[ch].dm_swap[slice]);
			if (high_byte[slice] != 0U) {
				/* HIGHER 16 BYTE */
				ddr_setval_s(ch, slice,
					     _reg_PHY_CALVL_VREF_DRIVING_SLICE,
					     0x00);
			} else {
				/* LOWER 16 BYTE */
				ddr_setval_s(ch, slice,
					     _reg_PHY_CALVL_VREF_DRIVING_SLICE,
					     0x01);
			}
		}

		/* BOARD SETTINGS (CA,ADDR_SEL) */
		data_l = (0x00ffffffU & (uint32_t)(board_cnf->ch[ch].ca_swap)) |
			0x00888888U;

		/* --- ADR_CALVL_SWIZZLE --- */
		if (prr_product == PRR_PRODUCT_M3) {
			ddr_setval(ch, _reg_PHY_ADR_CALVL_SWIZZLE0_0, data_l);
			ddr_setval(ch, _reg_PHY_ADR_CALVL_SWIZZLE1_0,
				   0x00000000);
			ddr_setval(ch, _reg_PHY_ADR_CALVL_SWIZZLE0_1, data_l);
			ddr_setval(ch, _reg_PHY_ADR_CALVL_SWIZZLE1_1,
				   0x00000000);
			ddr_setval(ch, _reg_PHY_ADR_CALVL_DEVICE_MAP,
				   _par_CALVL_DEVICE_MAP);
		} else {
			ddr_setval(ch, _reg_PHY_ADR_CALVL_SWIZZLE0, data_l);
			ddr_setval(ch, _reg_PHY_ADR_CALVL_SWIZZLE1, 0x00000000);
			ddr_setval(ch, _reg_PHY_CALVL_DEVICE_MAP,
				   _par_CALVL_DEVICE_MAP);
		}

		/* --- ADR_ADDR_SEL --- */
		data_l = 0U;
		tmp = board_cnf->ch[ch].ca_swap;
		for (i = 0U; i < 6U; i++) {
			data_l |= ((tmp & 0x0fU) << (i * 5U));
			tmp = tmp >> 4;
		}
		ddr_setval(ch, _reg_PHY_ADR_ADDR_SEL, data_l);
		if (ddr_phycaslice == 1U) {
			/* ----------- adr slice2 swap ----------- */
			tmp  = (uint32_t)((board_cnf->ch[ch].ca_swap) >> 40);
			data_l = (tmp & 0x00ffffffU) | 0x00888888U;

			/* --- ADR_CALVL_SWIZZLE --- */
			if (prr_product == PRR_PRODUCT_M3) {
				ddr_setval_s(ch, 2,
					     _reg_PHY_ADR_CALVL_SWIZZLE0_0,
					     data_l);
				ddr_setval_s(ch, 2,
					     _reg_PHY_ADR_CALVL_SWIZZLE1_0,
					     0x00000000);
				ddr_setval_s(ch, 2,
					     _reg_PHY_ADR_CALVL_SWIZZLE0_1,
					     data_l);
				ddr_setval_s(ch, 2,
					     _reg_PHY_ADR_CALVL_SWIZZLE1_1,
					     0x00000000);
				ddr_setval_s(ch, 2,
					     _reg_PHY_ADR_CALVL_DEVICE_MAP,
					     _par_CALVL_DEVICE_MAP);
			} else {
				ddr_setval_s(ch, 2,
					     _reg_PHY_ADR_CALVL_SWIZZLE0,
					     data_l);
				ddr_setval_s(ch, 2,
					     _reg_PHY_ADR_CALVL_SWIZZLE1,
					     0x00000000);
				ddr_setval_s(ch, 2,
					     _reg_PHY_CALVL_DEVICE_MAP,
					     _par_CALVL_DEVICE_MAP);
			}

			/* --- ADR_ADDR_SEL --- */
			data_l = 0U;
			for (i = 0U; i < 6U; i++) {
				data_l |= ((tmp & 0x0fU) << (i * 5U));
				tmp = tmp >> 4U;
			}

			ddr_setval_s(ch, 2, _reg_PHY_ADR_ADDR_SEL, data_l);
		}

		/* BOARD SETTINGS (BYTE_ORDER_SEL) */
		if (prr_product == PRR_PRODUCT_M3) {
			/* --- DATA_BYTE_SWAP --- */
			data_l = 0U;
			tmp = board_cnf->ch[ch].dqs_swap;
			for (i = 0U; i < 4U; i++) {
				data_l |= ((tmp & 0x03U) << (i * 2U));
				tmp = tmp >> 4U;
			}
		} else {
			/* --- DATA_BYTE_SWAP --- */
			data_l = board_cnf->ch[ch].dqs_swap;
			ddr_setval(ch, _reg_PI_DATA_BYTE_SWAP_EN, 0x01);
			ddr_setval(ch, _reg_PI_DATA_BYTE_SWAP_SLICE0,
				   (data_l) & 0x0fU);
			ddr_setval(ch, _reg_PI_DATA_BYTE_SWAP_SLICE1,
				   (data_l >> 4U * 1U) & 0x0fU);
			ddr_setval(ch, _reg_PI_DATA_BYTE_SWAP_SLICE2,
				   (data_l >> 4U * 2U) & 0x0fU);
			ddr_setval(ch, _reg_PI_DATA_BYTE_SWAP_SLICE3,
				   (data_l >> 4U * 3U) & 0x0fU);

			ddr_setval(ch, _reg_PHY_DATA_BYTE_ORDER_SEL_HIGH, 0x00U);
		}
		ddr_setval(ch, _reg_PHY_DATA_BYTE_ORDER_SEL, data_l);
	}
}

static void ddr_config(void)
{
	uint32_t num_cacs_dly = _reg_PHY_CLK_CACS_SLAVE_DELAY_X_NUM;
	uint32_t reg_ofs, dly;
	uint32_t ch, slice;
	uint32_t data_l;
	uint32_t tmp;
	uint32_t i;
	int8_t _adj;
	int16_t adj;
	uint32_t dq;
	union {
		uint32_t ui32[4];
		uint8_t ui8[16];
	} patt;
	uint16_t patm;

	/* configure ddrphy registers */
	ddr_config_sub();

	/* WDQ_USER_PATT */
	foreach_vch(ch) {
		for (slice = 0U; slice < SLICE_CNT; slice++) {
			patm = 0U;
			for (i = 0U; i < 16U; i++) {
				tmp = board_cnf->ch[ch].wdqlvl_patt[i];
				patt.ui8[i] = tmp & 0xffU;
				if ((tmp & 0x100U) != 0U) {
					patm |= (1U << (uint16_t)i);
				}
			}
			ddr_setval_s(ch, slice, _reg_PHY_USER_PATT0,
				     patt.ui32[0]);
			ddr_setval_s(ch, slice, _reg_PHY_USER_PATT1,
				     patt.ui32[1]);
			ddr_setval_s(ch, slice, _reg_PHY_USER_PATT2,
				     patt.ui32[2]);
			ddr_setval_s(ch, slice, _reg_PHY_USER_PATT3,
				     patt.ui32[3]);
			ddr_setval_s(ch, slice, _reg_PHY_USER_PATT4, patm);
		}
	}

	/* CACS DLY */
	data_l = board_cnf->cacs_dly + (uint32_t)_f_scale_adj(board_cnf->cacs_dly_adj);
	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_FREQ_SEL_MULTICAST_EN), 0x00U);
	foreach_vch(ch) {
		for (i = 0U; i < num_cacs_dly - 4U; i++) {
			adj = _f_scale_adj(board_cnf->ch[ch].cacs_adj[i]);
			dly = _reg_PHY_CLK_CACS_SLAVE_DELAY_X[i];
			ddrtbl_setval(_cnf_DDR_PHY_ADR_V_REGSET, dly,
				      data_l + (uint32_t)adj);
			reg_ofs = ddr_regdef_adr(dly) - DDR_PHY_ADR_V_REGSET_OFS;
			reg_ddrphy_write(ch, ddr_regdef_adr(dly),
					_cnf_DDR_PHY_ADR_V_REGSET[reg_ofs]);
		}

		for (i = num_cacs_dly - 4U; i < num_cacs_dly; i++) {
			adj = _f_scale_adj(board_cnf->ch[ch].cacs_adj[i]);
			dly = _reg_PHY_CLK_CACS_SLAVE_DELAY_X[i];
			ddrtbl_setval(_cnf_DDR_PHY_ADR_G_REGSET, dly,
				      data_l + (uint32_t)adj);
			reg_ofs = ddr_regdef_adr(dly) - DDR_PHY_ADR_G_REGSET_OFS;
			reg_ddrphy_write(ch, ddr_regdef_adr(dly),
					_cnf_DDR_PHY_ADR_G_REGSET[reg_ofs]);
		}

		if (ddr_phycaslice == 1U) {
			for (i = 0U; i < 6U; i++) {
				adj = _f_scale_adj(board_cnf->ch[ch].cacs_adj[i + num_cacs_dly]);
				dly = _reg_PHY_CLK_CACS_SLAVE_DELAY_X[i];
				ddrtbl_setval(_cnf_DDR_PHY_ADR_V_REGSET, dly,
					      data_l + (uint32_t)adj);
				reg_ofs = ddr_regdef_adr(dly) - DDR_PHY_ADR_V_REGSET_OFS;
				reg_ddrphy_write(ch, ddr_regdef_adr(dly) + 0x0100U,
						 _cnf_DDR_PHY_ADR_V_REGSET[reg_ofs]);
			}
		}
	}

	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_FREQ_SEL_MULTICAST_EN),
			   BIT(ddr_regdef_lsb(_reg_PHY_FREQ_SEL_MULTICAST_EN)));

	/* WDQDM DLY */
	data_l = board_cnf->dqdm_dly_w;
	foreach_vch(ch) {
		for (slice = 0U; slice < SLICE_CNT; slice++) {
			for (i = 0U; i <= 8U; i++) {
				dq = slice * 8U + (uint32_t)i;
				if (i == 8U) {
					_adj = board_cnf->ch[ch].dm_adj_w[slice];
				} else {
					_adj = board_cnf->ch[ch].dq_adj_w[dq];
				}
				adj = _f_scale_adj(_adj);
				ddr_setval_s(ch, slice,
					     _reg_PHY_CLK_WRX_SLAVE_DELAY[i],
					     data_l + (uint32_t)adj);
			}
		}
	}

	/* RDQDM DLY */
	data_l = board_cnf->dqdm_dly_r;
	foreach_vch(ch) {
		for (slice = 0U; slice < SLICE_CNT; slice++) {
			for (i = 0U; i <= 8U; i++) {
				dq = slice * 8U + (uint32_t)i;
				if (i == 8U) {
					_adj = board_cnf->ch[ch].dm_adj_r[slice];
				} else {
					_adj = board_cnf->ch[ch].dq_adj_r[dq];
				}
				adj = _f_scale_adj(_adj);
				dly = _reg_PHY_RDDQS_X_FALL_SLAVE_DELAY[i];
				ddr_setval_s(ch, slice, dly, data_l + (uint32_t)adj);
				dly = _reg_PHY_RDDQS_X_RISE_SLAVE_DELAY[i];
				ddr_setval_s(ch, slice, dly, data_l + (uint32_t)adj);
			}
		}
	}
}

/* DBSC register setting functions */
static void dbsc_regset_pre(void)
{
	uint32_t ch, csab;
	uint32_t data_l;

	/* PRIMARY SETTINGS */
	/* LPDDR4, BL=16, DFI interface */
	mmio_write_32(DBSC_DBKIND, 0x0000000aU);
	mmio_write_32(DBSC_DBBL, 0x00000002U);
	mmio_write_32(DBSC_DBPHYCONF0, 0x00000001U);

	/* FREQRATIO=2 */
	mmio_write_32(DBSC_DBSYSCONF1, 0x00000002U);

	/*
	 * DRAM SIZE REGISTER:
	 * set all ranks as density=0(4Gb) for PHY initialization
	 */
	foreach_vch(ch) {
		for (csab = 0U; csab < 4U; csab++) {
			mmio_write_32(DBSC_DBMEMCONF(ch, csab),
				      DBMEMCONF_REGD(0U));
		}
	}

	if (prr_product == PRR_PRODUCT_M3) {
		data_l = 0xe4e4e4e4U;
		foreach_ech(ch) {
			if ((ddr_phyvalid & (1U << ch)) != 0U) {
				data_l = (data_l & (~(0x000000FFU << (ch * 8U))))
					| (((board_cnf->ch[ch].dqs_swap & 0x0003U)
					| ((board_cnf->ch[ch].dqs_swap & 0x0030U) >> 2U)
					| ((board_cnf->ch[ch].dqs_swap & 0x0300U) >> 4U)
					| ((board_cnf->ch[ch].dqs_swap & 0x3000U) >> 6U))
					   << (ch * 8U));
			}
		}
		mmio_write_32(DBSC_DBBSWAP, data_l);
	}
}

static void dbsc_regset(void)
{
	int32_t i;
	uint32_t ch;
	uint32_t data_l;
	uint32_t data_l2;
	uint32_t wdql;
	uint32_t dqenltncy;
	uint32_t dql;
	uint32_t dqienltncy;
	uint32_t wrcslat;
	uint32_t wrcsgap;
	uint32_t rdcslat;
	uint32_t rdcsgap;
	uint32_t scfctst0_act_act;
	uint32_t scfctst0_rda_act;
	uint32_t scfctst0_wra_act;
	uint32_t scfctst0_pre_act;
	uint32_t scfctst1_rd_wr;
	uint32_t scfctst1_wr_rd;
	uint32_t scfctst1_act_rd_wr;
	uint32_t scfctst1_asyncofs;
	uint32_t dbschhrw1_sctrfcab;

	/* RFC */
	js2[js2_trfcab] =
	    _f_scale(ddr_mbps, ddr_mbpsdiv,
		     jedec_spec2_trfc_ab[max_density] * 1000U, 0U);
	/* DBTR0.CL  : RL */
	mmio_write_32(DBSC_DBTR(0), RL);

	/* DBTR1.CWL : WL */
	mmio_write_32(DBSC_DBTR(1), WL);

	/* DBTR2.AL  : 0 */
	mmio_write_32(DBSC_DBTR(2), 0U);

	/* DBTR3.TRCD: tRCD */
	mmio_write_32(DBSC_DBTR(3), js2[js2_trcd]);

	/* DBTR4.TRPA,TRP: tRPab,tRPpb */
	mmio_write_32(DBSC_DBTR(4), (js2[js2_trpab] << 16) | js2[js2_trppb]);

	/* DBTR5.TRC : use tRCpb */
	mmio_write_32(DBSC_DBTR(5), js2[js2_trcpb]);

	/* DBTR6.TRAS : tRAS */
	mmio_write_32(DBSC_DBTR(6), js2[js2_tras]);

	/* DBTR7.TRRD : tRRD */
	mmio_write_32(DBSC_DBTR(7), (js2[js2_trrd] << 16) | js2[js2_trrd]);

	/* DBTR8.TFAW : tFAW */
	mmio_write_32(DBSC_DBTR(8), js2[js2_tfaw]);

	/* DBTR9.TRDPR : tRTP */
	mmio_write_32(DBSC_DBTR(9), js2[js2_trtp]);

	/* DBTR10.TWR : nWR */
	mmio_write_32(DBSC_DBTR(10), js1[js1_ind].nwr);

	/*
	 * DBTR11.TRDWR : RL +  BL / 2 + Rounddown(tRPST) + PHY_ODTLoff -
	 * odtlon + tDQSCK - tODTon,min +
	 * PCB delay (out+in) + tPHY_ODToff
	 */
	mmio_write_32(DBSC_DBTR(11),
		      RL + (16U / 2U) + 1U + 2U - js1[js1_ind].odtlon +
		      js2[js2_tdqsck] - js2[js2_tODTon_min] +
		      _f_scale(ddr_mbps, ddr_mbpsdiv, 1300, 0));

	/* DBTR12.TWRRD : WL + 1 + BL/2 + tWTR */
	data_l = WL + 1U + (16U / 2U) + js2[js2_twtr];
	mmio_write_32(DBSC_DBTR(12), (data_l << 16) | data_l);

	/* DBTR13.TRFCAB : tRFCab */
	mmio_write_32(DBSC_DBTR(13), js2[js2_trfcab]);

	/* DBTR14.TCKEHDLL,tCKEH : tCKEHCMD,tCKEHCMD */
	mmio_write_32(DBSC_DBTR(14),
		      (js2[js2_tckehcmd] << 16) | (js2[js2_tckehcmd]));

	/* DBTR15.TCKESR,TCKEL : tSR,tCKELPD */
	mmio_write_32(DBSC_DBTR(15), (js2[js2_tsr] << 16) | (js2[js2_tckelpd]));

	/* DBTR16 */
	/* WDQL : tphy_wrlat + tphy_wrdata */
	wdql = ddrtbl_getval(_cnf_DDR_PI_REGSET, _reg_PI_WRLAT_F1);
	/* DQENLTNCY : tphy_wrlat = WL-2 : PHY_WRITE_PATH_LAT_ADD == 0
	 *             tphy_wrlat = WL-3 : PHY_WRITE_PATH_LAT_ADD != 0
	 */
	dqenltncy = ddrtbl_getval(_cnf_DDR_PI_REGSET, _reg_PI_WRLAT_ADJ_F1);
	/* DQL : tphy_rdlat + trdata_en */
	/* it is not important for dbsc */
	dql = RL + 16U;
	/* DQIENLTNCY : trdata_en */
	dqienltncy = ddrtbl_getval(_cnf_DDR_PI_REGSET, _reg_PI_RDLAT_ADJ_F1) - 1U;
	mmio_write_32(DBSC_DBTR(16),
		      (dqienltncy << 24) | (dql << 16) | (dqenltncy << 8) | wdql);

	/* DBTR24 */
	/* WRCSLAT = WRLAT -5 */
	wrcslat = wdql - 5U;
	/* WRCSGAP = 5 */
	wrcsgap = 5U;
	/* RDCSLAT = RDLAT_ADJ +2 */
	rdcslat = dqienltncy;
	if (prr_product != PRR_PRODUCT_M3) {
		rdcslat += 2U;
	}
	/* RDCSGAP = 6 */
	rdcsgap = 6U;
	if (prr_product == PRR_PRODUCT_M3) {
		rdcsgap = 4U;
	}
	mmio_write_32(DBSC_DBTR(24),
		      (rdcsgap << 24) | (rdcslat << 16) | (wrcsgap << 8) | wrcslat);

	/* DBTR17.TMODRD,TMOD,TRDMR: tMRR,tMRD,(0) */
	mmio_write_32(DBSC_DBTR(17),
		      (js2[js2_tmrr] << 24) | (js2[js2_tmrd] << 16));

	/* DBTR18.RODTL, RODTA, WODTL, WODTA : do not use in LPDDR4 */
	mmio_write_32(DBSC_DBTR(18), 0);

	/* DBTR19.TZQCL, TZQCS : do not use in LPDDR4 */
	mmio_write_32(DBSC_DBTR(19), 0);

	/* DBTR20.TXSDLL, TXS : tRFCab+tCKEHCMD */
	data_l = js2[js2_trfcab] + js2[js2_tckehcmd];
	mmio_write_32(DBSC_DBTR(20), (data_l << 16) | data_l);

	/* DBTR21.TCCD */
	/* DBTR23.TCCD */
	if (ddr_tccd == 8U) {
		data_l = 8U;
		mmio_write_32(DBSC_DBTR(21), (data_l << 16) | data_l);
		mmio_write_32(DBSC_DBTR(23), 0x00000002);
	} else if (ddr_tccd <= 11U) {
		data_l = 11U;
		mmio_write_32(DBSC_DBTR(21), (data_l << 16) | data_l);
		mmio_write_32(DBSC_DBTR(23), 0x00000000);
	} else {
		data_l = ddr_tccd;
		mmio_write_32(DBSC_DBTR(21), (data_l << 16) | data_l);
		mmio_write_32(DBSC_DBTR(23), 0x00000000);
	}

	/* DBTR22.ZQLAT : */
	data_l = js2[js2_tzqcalns] * 100U;	/*  1000 * 1000 ps */
	data_l = (data_l << 16U) | (js2[js2_tzqlat] + 24U + 20U);
	mmio_write_32(DBSC_DBTR(22), data_l);

	/* DBTR25 : do not use in LPDDR4 */
	mmio_write_32(DBSC_DBTR(25), 0U);

	/*
	 * DBRNK :
	 * DBSC_DBRNK2 rkrr
	 * DBSC_DBRNK3 rkrw
	 * DBSC_DBRNK4 rkwr
	 * DBSC_DBRNK5 rkww
	 */
#define _par_DBRNK_VAL	(0x7007U)

	for (i = 0; i < 4; i++) {
		data_l = (_par_DBRNK_VAL >> ((uint32_t)i * 4U)) & 0x0fU;
		data_l2 = 0U;
		foreach_vch(ch) {
			data_l2 = data_l2 | (data_l << (4U * ch));
		}
		mmio_write_32(DBSC_DBRNK(2 + i), data_l2);
	}
	mmio_write_32(DBSC_DBADJ0, 0x00000000U);

	/* timing registers for scheduler */
	/* SCFCTST0 */
	/* SCFCTST0 ACT-ACT */
	scfctst0_act_act = js2[js2_trcpb] * 800UL * ddr_mbpsdiv / ddr_mbps;
	/* SCFCTST0 RDA-ACT */
	scfctst0_rda_act = ((16U / 2U) + js2[js2_trtp] - 8U +
		  js2[js2_trppb]) * 800UL * ddr_mbpsdiv / ddr_mbps;
	/* SCFCTST0 WRA-ACT */
	scfctst0_wra_act = (WL + 1U + (16U / 2U) +
		  js1[js1_ind].nwr) * 800UL * ddr_mbpsdiv / ddr_mbps;
	/* SCFCTST0 PRE-ACT */
	scfctst0_pre_act = js2[js2_trppb];
	mmio_write_32(DBSC_SCFCTST0,
		      (scfctst0_act_act << 24) | (scfctst0_rda_act << 16) |
		      (scfctst0_wra_act << 8) | scfctst0_pre_act);

	/* SCFCTST1 */
	/* SCFCTST1 RD-WR */
	scfctst1_rd_wr = (mmio_read_32(DBSC_DBTR(11)) & 0xffU) * 800UL * ddr_mbpsdiv /
		ddr_mbps;
	/* SCFCTST1 WR-RD */
	scfctst1_wr_rd = (mmio_read_32(DBSC_DBTR(12)) & 0xff) * 800UL * ddr_mbpsdiv /
		ddr_mbps;
	/* SCFCTST1 ACT-RD/WR */
	scfctst1_act_rd_wr = js2[js2_trcd] * 800UL * ddr_mbpsdiv / ddr_mbps;
	/* SCFCTST1 ASYNCOFS */
	scfctst1_asyncofs = 12U;
	mmio_write_32(DBSC_SCFCTST1,
		      (scfctst1_rd_wr << 24) | (scfctst1_wr_rd << 16) |
		      (scfctst1_act_rd_wr << 8) | scfctst1_asyncofs);

	/* DBSCHRW1 */
	/* DBSCHRW1 SCTRFCAB */
	dbschhrw1_sctrfcab = js2[js2_trfcab] * 800UL * ddr_mbpsdiv / ddr_mbps;
	data_l = (((mmio_read_32(DBSC_DBTR(16)) & 0x00FF0000U) >> 16) +
		  (mmio_read_32(DBSC_DBTR(22)) & 0x0000FFFFU) +
		  (0x28U * 2U)) * 400U * 2U * ddr_mbpsdiv / ddr_mbps + 7U;
	if (dbschhrw1_sctrfcab < data_l) {
		dbschhrw1_sctrfcab = data_l;
	}

	if ((prr_product == PRR_PRODUCT_M3) && (prr_cut < PRR_PRODUCT_30)) {
		mmio_write_32(DBSC_DBSCHRW1, dbschhrw1_sctrfcab +
			      ((mmio_read_32(DBSC_DBTR(22)) & 0x0000FFFFU) *
			       400U * 2U * ddr_mbpsdiv + (ddr_mbps - 1U)) / ddr_mbps - 3U);
	} else {
		mmio_write_32(DBSC_DBSCHRW1, dbschhrw1_sctrfcab +
			      ((mmio_read_32(DBSC_DBTR(22)) & 0x0000FFFFU) *
			       400U * 2U * ddr_mbpsdiv + (ddr_mbps - 1U)) / ddr_mbps);
	}

	/* QOS and CAM */
#ifdef DDR_QOS_INIT_SETTING	/*  only for non qos_init */
	/* wbkwait(0004), wbkmdhi(4,2),wbkmdlo(1,8) */
	mmio_write_32(DBSC_DBCAM0CNF1, 0x00043218U);
	/* 0(fillunit),8(dirtymax),4(dirtymin) */
	mmio_write_32(DBSC_DBCAM0CNF2, 0x000000F4U);
	/* stop_tolerance */
	mmio_write_32(DBSC_DBSCHRW0, 0x22421111U);
	/* rd-wr/wr-rd toggle priority */
	mmio_write_32(DBSC_SCFCTST2, 0x012F1123U);
	mmio_write_32(DBSC_DBSCHSZ0, 0x00000001U);
	mmio_write_32(DBSC_DBSCHCNT0, 0x000F0037U);

	/* QoS Settings */
	mmio_write_32(DBSC_DBSCHQOS00, 0x00000F00U);
	mmio_write_32(DBSC_DBSCHQOS01, 0x00000B00U);
	mmio_write_32(DBSC_DBSCHQOS02, 0x00000000U);
	mmio_write_32(DBSC_DBSCHQOS03, 0x00000000U);
	mmio_write_32(DBSC_DBSCHQOS40, 0x00000300U);
	mmio_write_32(DBSC_DBSCHQOS41, 0x000002F0U);
	mmio_write_32(DBSC_DBSCHQOS42, 0x00000200U);
	mmio_write_32(DBSC_DBSCHQOS43, 0x00000100U);
	mmio_write_32(DBSC_DBSCHQOS90, 0x00000100U);
	mmio_write_32(DBSC_DBSCHQOS91, 0x000000F0U);
	mmio_write_32(DBSC_DBSCHQOS92, 0x000000A0U);
	mmio_write_32(DBSC_DBSCHQOS93, 0x00000040U);
	mmio_write_32(DBSC_DBSCHQOS120, 0x00000040U);
	mmio_write_32(DBSC_DBSCHQOS121, 0x00000030U);
	mmio_write_32(DBSC_DBSCHQOS122, 0x00000020U);
	mmio_write_32(DBSC_DBSCHQOS123, 0x00000010U);
	mmio_write_32(DBSC_DBSCHQOS130, 0x00000100U);
	mmio_write_32(DBSC_DBSCHQOS131, 0x000000F0U);
	mmio_write_32(DBSC_DBSCHQOS132, 0x000000A0U);
	mmio_write_32(DBSC_DBSCHQOS133, 0x00000040U);
	mmio_write_32(DBSC_DBSCHQOS140, 0x000000C0U);
	mmio_write_32(DBSC_DBSCHQOS141, 0x000000B0U);
	mmio_write_32(DBSC_DBSCHQOS142, 0x00000080U);
	mmio_write_32(DBSC_DBSCHQOS143, 0x00000040U);
	mmio_write_32(DBSC_DBSCHQOS150, 0x00000040U);
	mmio_write_32(DBSC_DBSCHQOS151, 0x00000030U);
	mmio_write_32(DBSC_DBSCHQOS152, 0x00000020U);
	mmio_write_32(DBSC_DBSCHQOS153, 0x00000010U);

	mmio_write_32(QOSCTRL_RAEN, 0x00000001U);
#endif /* DDR_QOS_INIT_SETTING */

	/* resrdis */
	mmio_write_32(DBSC_DBBCAMDIS, 0x00000001U);
}

static void dbsc_regset_post(void)
{
	uint32_t slice, rdlat_max, rdlat_min;
	uint32_t ch, cs;
	uint32_t data_l;
	uint32_t srx;

	rdlat_max = 0U;
	rdlat_min = 0xffffU;
	foreach_vch(ch) {
		for (cs = 0U; cs < CS_CNT; cs++) {
			if ((ch_have_this_cs[cs] & (1U << ch)) != 0U) {
				for (slice = 0U; slice < SLICE_CNT; slice++) {
					ddr_setval_s(ch, slice,
						     _reg_PHY_PER_CS_TRAINING_INDEX,
						     cs);
					data_l = ddr_getval_s(ch, slice,
							      _reg_PHY_RDDQS_LATENCY_ADJUST);
					if (data_l > rdlat_max) {
						rdlat_max = data_l;
					}
					if (data_l < rdlat_min) {
						rdlat_min = data_l;
					}
				}
			}
		}
	}

	mmio_write_32(DBSC_DBTR(24),
		      ((rdlat_max + 2U) << 24) +
		      ((rdlat_max + 2U) << 16) +
		      mmio_read_32(DBSC_DBTR(24)));

	/* set ddr density information */
	foreach_ech(ch) {
		for (cs = 0U; cs < CS_CNT; cs++) {
			if (ddr_density[ch][cs] == 0xffU) {
				mmio_write_32(DBSC_DBMEMCONF(ch, cs), 0x00U);
			} else {
				mmio_write_32(DBSC_DBMEMCONF(ch, cs),
					      DBMEMCONF_REGD(ddr_density[ch]
							     [cs]));
			}
		}
		mmio_write_32(DBSC_DBMEMCONF(ch, 2), 0x00000000U);
		mmio_write_32(DBSC_DBMEMCONF(ch, 3), 0x00000000U);
	}

	mmio_write_32(DBSC_DBBUS0CNF1, 0x00000010U);

	/* set DBI */
	if (board_cnf->dbi_en != 0U) {
		mmio_write_32(DBSC_DBDBICNT, 0x00000003U);
	}

	/* set REFCYCLE */
	data_l = (get_refperiod()) * ddr_mbps / 2000U / ddr_mbpsdiv;
	mmio_write_32(DBSC_DBRFCNF1, 0x00080000U | (data_l & 0x0000ffffU));
	mmio_write_32(DBSC_DBRFCNF2, 0x00010000U | DBSC_REFINTS);

#if RCAR_REWT_TRAINING != 0
	/* Periodic-WriteDQ Training seeting */
	if ((prr_product == PRR_PRODUCT_M3) &&
	    (prr_cut == PRR_PRODUCT_10)) {
		/* G2M Ver.1.0 not support */
	} else {
		/* G2M Ver.1.1 or later */
		mmio_write_32(DBSC_DBDFIPMSTRCNF, 0x00000000U);

		ddr_setval_ach_as(_reg_PHY_WDQLVL_PATT, 0x04U);
		ddr_setval_ach_as(_reg_PHY_WDQLVL_QTR_DLY_STEP, 0x0FU);
		ddr_setval_ach_as(_reg_PHY_WDQLVL_DLY_STEP, 0x50U);
		ddr_setval_ach_as(_reg_PHY_WDQLVL_DQDM_SLV_DLY_START, 0x0300U);

		ddr_setval_ach(_reg_PI_WDQLVL_CS_MAP,
			       ddrtbl_getval(_cnf_DDR_PI_REGSET,
					     _reg_PI_WDQLVL_CS_MAP));
		ddr_setval_ach(_reg_PI_LONG_COUNT_MASK, 0x1fU);
		ddr_setval_ach(_reg_PI_WDQLVL_VREF_EN, 0x00U);
		ddr_setval_ach(_reg_PI_WDQLVL_ROTATE, 0x01U);
		ddr_setval_ach(_reg_PI_TREF_F0, 0x0000U);
		ddr_setval_ach(_reg_PI_TREF_F1, 0x0000U);
		ddr_setval_ach(_reg_PI_TREF_F2, 0x0000U);

		if (prr_product == PRR_PRODUCT_M3) {
			ddr_setval_ach(_reg_PI_WDQLVL_EN, 0x02U);
		} else {
			ddr_setval_ach(_reg_PI_WDQLVL_EN_F1, 0x02U);
		}
		ddr_setval_ach(_reg_PI_WDQLVL_PERIODIC, 0x01U);

		/* DFI_PHYMSTR_ACK , WTmode setting */
		/* DFI_PHYMSTR_ACK: WTmode =b'01 */
		mmio_write_32(DBSC_DBDFIPMSTRCNF, 0x00000011U);
	}
#endif /* RCAR_REWT_TRAINING */
	/* periodic dram zqcal enable */
	mmio_write_32(DBSC_DBCALCNF, 0x01000010U);

	/* periodic phy ctrl update enable */
	if ((prr_product == PRR_PRODUCT_M3) &&
	    (prr_cut < PRR_PRODUCT_30)) {
		/* non : G2M Ver.1.x not support */
	} else {
		mmio_write_32(DBSC_DBDFICUPDCNF, 0x28240001U);
	}

#ifdef DDR_BACKUPMODE
	/* SRX */
	srx = 0x0A840001U; /* for All channels */
	if (ddr_backup == DRAM_BOOT_STATUS_WARM) {
#ifdef DDR_BACKUPMODE_HALF /* for Half channel(ch0, 1 only) */
		NOTICE("BL2: [DEBUG_MESS] DDR_BACKUPMODE_HALF\n");
		srx = 0x0A040001U;
#endif /* DDR_BACKUPMODE_HALF */
		send_dbcmd(srx);
	}
#endif /* DDR_BACKUPMODE */

	/* set Auto Refresh */
	mmio_write_32(DBSC_DBRFEN, 0x00000001U);

#if RCAR_REWT_TRAINING != 0
	/* Periodic WriteDQ Traning */
	if ((prr_product == PRR_PRODUCT_M3) &&
	    (prr_cut == PRR_PRODUCT_10)) {
		/* non : G2M Ver.1.0 not support */
	} else {
		/* G2M Ver.1.1 or later */
		ddr_setval_ach(_reg_PI_WDQLVL_INTERVAL, 0x0100U);
	}
#endif /* RCAR_REWT_TRAINING */

	/* dram access enable */
	mmio_write_32(DBSC_DBACEN, 0x00000001U);

	MSG_LF(__func__ "(done)");
}

/* DFI_INIT_START */
static uint32_t dfi_init_start(void)
{
	uint32_t ch;
	uint32_t phytrainingok;
	uint32_t retry;
	uint32_t data_l;
	uint32_t ret = 0U;
	const uint32_t RETRY_MAX = 0x10000U;

	ddr_setval_ach_as(_reg_PHY_DLL_RST_EN, 0x02U);
	dsb_sev();
	ddrphy_regif_idle();

	/* dll_rst negate */
	foreach_vch(ch) {
	    mmio_write_32(DBSC_DBPDCNT3(ch), 0x0000CF01U);
	}
	dsb_sev();

	/* wait init_complete */
	phytrainingok = 0U;
	retry = 0U;
	while (retry++ < RETRY_MAX) {
		foreach_vch(ch) {
			data_l = mmio_read_32(DBSC_DBDFISTAT(ch));
			if (data_l & 0x00000001U)  {
				phytrainingok |= (1U << ch);
			}
		}
		dsb_sev();
		if (phytrainingok == ddr_phyvalid)  {
			break;
		}
		if (retry % 256U == 0U) {
			ddr_setval_ach_as(_reg_SC_PHY_RX_CAL_START, 0x01U);
		}
	}

	/* all ch ok? */
	if ((phytrainingok & ddr_phyvalid) != ddr_phyvalid) {
		ret = 0xffU;
		goto done;
	}

	/*
	 * dbdficnt0:
	 * dfi_dram_clk_disable=0
	 * dfi_frequency = 0
	 * freq_ratio = 01 (2:1)
	 * init_start =0
	 */
	foreach_vch(ch) {
	    mmio_write_32(DBSC_DBDFICNT(ch), 0x00000010U);
	}
	dsb_sev();
done:
	return ret;
}

/* drivability setting : CMOS MODE ON/OFF */
static void change_lpddr4_en(uint32_t mode)
{
	uint32_t ch;
	uint32_t i;
	uint32_t data_l;
	const uint32_t _reg_PHY_PAD_DRIVE_X[3] = {
		_reg_PHY_PAD_ADDR_DRIVE,
		_reg_PHY_PAD_CLK_DRIVE,
		_reg_PHY_PAD_CS_DRIVE
	};

	foreach_vch(ch) {
		for (i = 0U; i < 3U; i++) {
			data_l = ddr_getval(ch, _reg_PHY_PAD_DRIVE_X[i]);
			if (mode != 0U) {
				data_l |= (1U << 14);
			} else {
				data_l &= ~(1U << 14);
			}
			ddr_setval(ch, _reg_PHY_PAD_DRIVE_X[i], data_l);
		}
	}
}

/* drivability setting */
static uint32_t set_term_code(void)
{
	uint32_t i;
	uint32_t ch, index;
	uint32_t data_l;
	uint32_t chip_id[2];
	uint32_t term_code;
	uint32_t override;

	term_code = ddrtbl_getval(_cnf_DDR_PHY_ADR_G_REGSET,
				  _reg_PHY_PAD_DATA_TERM);
	override = 0U;
	for (i = 0U; i < 2U; i++) {
		chip_id[i] = mmio_read_32(LIFEC_CHIPID(i));
	}

	index = 0U;
	while (true) {
		if (termcode_by_sample[index][0] == 0xffffffff) {
			break;
		}
		if ((termcode_by_sample[index][0] == chip_id[0]) &&
		    (termcode_by_sample[index][1] == chip_id[1])) {
			term_code = termcode_by_sample[index][2];
			override = 1;
			break;
		}
		index++;
	}

	if (override != 0U) {
		for (index = 0U; index < _reg_PHY_PAD_TERM_X_NUM; index++) {
			data_l =
			    ddrtbl_getval(_cnf_DDR_PHY_ADR_G_REGSET,
					  _reg_PHY_PAD_TERM_X[index]);
			data_l = (data_l & 0xfffe0000U) | term_code;
			ddr_setval_ach(_reg_PHY_PAD_TERM_X[index], data_l);
		}
	} else if ((prr_product == PRR_PRODUCT_M3) &&
		   (prr_cut == PRR_PRODUCT_10)) {
		/*  non */
	} else {
		ddr_setval_ach(_reg_PHY_PAD_TERM_X[0],
			       (ddrtbl_getval(_cnf_DDR_PHY_ADR_G_REGSET,
					      _reg_PHY_PAD_TERM_X[0]) & 0xFFFE0000U));
		ddr_setval_ach(_reg_PHY_CAL_CLEAR_0, 0x01U);
		ddr_setval_ach(_reg_PHY_CAL_START_0, 0x01U);
		foreach_vch(ch) {
			do {
				data_l =
				    ddr_getval(ch, _reg_PHY_CAL_RESULT2_OBS_0);
			} while (!(data_l & 0x00800000U));
		}

		/* G2M Ver.1.1 or later */
		foreach_vch(ch) {
			for (index = 0U; index < _reg_PHY_PAD_TERM_X_NUM;
			     index++) {
				data_l = ddr_getval(ch, _reg_PHY_PAD_TERM_X[index]);
				ddr_setval(ch, _reg_PHY_PAD_TERM_X[index],
					   (data_l & 0xFFFE0FFFU) | 0x00015000U);
			}
		}
	}

	ddr_padcal_tcompensate_getinit(override);

	return 0U;
}

/* DDR mode register setting */
static void ddr_register_set(void)
{
	int32_t fspwp;
	uint32_t tmp;

	for (fspwp = 1; fspwp >= 0; fspwp--) {
		/*MR13, fspwp */
		send_dbcmd(0x0e840d08U | ((2U - fspwp) << 6));

		tmp = ddrtbl_getval(_cnf_DDR_PI_REGSET,
				    reg_pi_mr1_data_fx_csx[fspwp][0]);
		send_dbcmd(0x0e840100U | tmp);

		tmp = ddrtbl_getval(_cnf_DDR_PI_REGSET,
				    reg_pi_mr2_data_fx_csx[fspwp][0]);
		send_dbcmd(0x0e840200U | tmp);

		tmp = ddrtbl_getval(_cnf_DDR_PI_REGSET,
				    reg_pi_mr3_data_fx_csx[fspwp][0]);
		send_dbcmd(0x0e840300U | tmp);

		tmp = ddrtbl_getval(_cnf_DDR_PI_REGSET,
				    reg_pi_mr11_data_fx_csx[fspwp][0]);
		send_dbcmd(0x0e840b00U | tmp);

		tmp = ddrtbl_getval(_cnf_DDR_PI_REGSET,
				    reg_pi_mr12_data_fx_csx[fspwp][0]);
		send_dbcmd(0x0e840c00U | tmp);

		tmp = ddrtbl_getval(_cnf_DDR_PI_REGSET,
				    reg_pi_mr14_data_fx_csx[fspwp][0]);
		send_dbcmd(0x0e840e00U | tmp);
		/* MR22 */
		send_dbcmd(0x0e841616U);

		/* ZQCAL start */
		send_dbcmd(0x0d84004FU);

		/* ZQLAT */
		send_dbcmd(0x0d840051U);
	}

	/* MR13, fspwp */
	send_dbcmd(0x0e840d08U);
}

/* Training handshake functions */
static inline uint32_t wait_freqchgreq(uint32_t assert)
{
	uint32_t data_l;
	uint32_t count;
	uint32_t ch;

	count = 100000U;

	if (assert != 0U) {
		do {
			data_l = 1U;
			foreach_vch(ch) {
				data_l &= mmio_read_32(DBSC_DBPDSTAT(ch));
			}
			count = count - 1U;
		} while (((data_l & 0x01U) != 0x01U) && (count != 0U));
	} else {
		do {
			data_l = 0U;
			foreach_vch(ch) {
				data_l |= mmio_read_32(DBSC_DBPDSTAT(ch));
			}
			count = count - 1U;
		} while (((data_l & 0x01U) != 0x00U) && (count != 0U));
	}

	return (count == 0U);
}

static inline void set_freqchgack(uint32_t assert)
{
	uint32_t ch;
	uint32_t data_l;

	if (assert != 0U) {
		data_l = 0x0CF20000U;
	} else {
		data_l = 0x00000000U;
	}

	foreach_vch(ch) {
	    mmio_write_32(DBSC_DBPDCNT2(ch), data_l);
	}
}

static inline void set_dfifrequency(uint32_t freq)
{
	uint32_t ch;

	foreach_vch(ch) {
		mmio_clrsetbits_32(DBSC_DBDFICNT(ch), 0x1fU << 24, freq << 24);
	}
	dsb_sev();
}

static uint32_t pll3_freq(uint32_t on)
{
	uint32_t timeout;

	timeout = wait_freqchgreq(1U);

	if (timeout != 0U) {
		return 1U;
	}

	pll3_control(on);
	set_dfifrequency(on);

	set_freqchgack(1U);
	timeout = wait_freqchgreq(0U);
	set_freqchgack(0U);

	if (timeout != 0U) {
		FATAL_MSG("BL2: Time out[2]\n");
		return 1U;
	}

	return 0U;
}

/* update dly */
static void update_dly(void)
{
	ddr_setval_ach(_reg_SC_PHY_MANUAL_UPDATE, 0x01U);
	ddr_setval_ach(_reg_PHY_ADRCTL_MANUAL_UPDATE, 0x01U);
}

/* training by pi */
static uint32_t pi_training_go(void)
{
	uint32_t flag;
	uint32_t data_l;
	uint32_t retry;
	const uint32_t RETRY_MAX = 4096U * 16U;
	uint32_t ch;
	uint32_t mst_ch;
	uint32_t cur_frq;
	uint32_t complete;
	uint32_t frqchg_req;

	/* pi_start */
	ddr_setval_ach(_reg_PI_START, 0x01U);
	foreach_vch(ch) {
	    ddr_getval(ch, _reg_PI_INT_STATUS);
	}

	/* set dfi_phymstr_ack = 1 */
	mmio_write_32(DBSC_DBDFIPMSTRCNF, 0x00000001U);
	dsb_sev();

	/* wait pi_int_status[0] */
	mst_ch = 0U;
	flag = 0U;
	complete = 0U;
	cur_frq = 0U;
	for (retry = 0U; retry < RETRY_MAX; retry++) {
		frqchg_req = mmio_read_32(DBSC_DBPDSTAT(mst_ch)) & 0x01;

		if (frqchg_req != 0U) {
			if (cur_frq != 0U) {
				/* Low frequency */
				flag = pll3_freq(0U);
				cur_frq = 0U;
			} else {
				/* High frequency */
				flag = pll3_freq(1U);
				cur_frq = 1U;
			}
			if (flag != 0U) {
				break;
			}
		} else {
			if (cur_frq != 0U) {
				foreach_vch(ch) {
					if ((complete & (1U << ch)) != 0U) {
						continue;
					}
					data_l = ddr_getval(ch, _reg_PI_INT_STATUS);
					if ((data_l & 0x01U) != 0U) {
						complete |= (1U << ch);
					}
				}
				if (complete == ddr_phyvalid) {
					break;
				}
			}
		}
	}
	foreach_vch(ch) {
		/* dummy read */
		data_l = ddr_getval_s(ch, 0U, _reg_PHY_CAL_RESULT2_OBS_0);
		data_l = ddr_getval(ch, _reg_PI_INT_STATUS);
		ddr_setval(ch, _reg_PI_INT_ACK, data_l);
	}
	if (ddrphy_regif_chk() != 0U) {
		complete = 0xfdU;
	}
	return complete;
}

/* Initialize DDR */
static uint32_t init_ddr(void)
{
	uint32_t i;
	uint32_t data_l;
	uint32_t phytrainingok;
	uint32_t ch, slice;
	uint32_t index;
	uint32_t err;
	int16_t adj;

	MSG_LF(__func__ ":0\n");

	/* unlock phy */
	/* Unlock DDRPHY register(AGAIN) */
	foreach_vch(ch) {
	    mmio_write_32(DBSC_DBPDLK(ch), 0x0000A55AU);
	}
	dsb_sev();

	reg_ddrphy_write_a(0x00001010U, 0x00000001U);
	/* DBSC register pre-setting */
	dbsc_regset_pre();

	/* load ddrphy registers */
	ddrtbl_load();

	/* configure ddrphy registers */
	ddr_config();

	/* dfi_reset assert */
	foreach_vch(ch) {
	    mmio_write_32(DBSC_DBPDCNT0(ch), 0x01U);
	}
	dsb_sev();

	/* dbsc register set */
	dbsc_regset();
	MSG_LF(__func__ ":1\n");

	/* dfi_reset negate */
	foreach_vch(ch) {
	    mmio_write_32(DBSC_DBPDCNT0(ch), 0x00U);
	}
	dsb_sev();

	/* dfi_init_start (start ddrphy) */
	err = dfi_init_start();
	if (err != 0U) {
		return INITDRAM_ERR_I;
	}
	MSG_LF(__func__ ":2\n");

	/* ddr backupmode end */
#ifdef DDR_BACKUPMODE
	if (ddr_backup != 0U) {
		NOTICE("BL2: [WARM_BOOT]\n");
	} else {
		NOTICE("BL2: [COLD_BOOT]\n");
	}
#endif
	MSG_LF(__func__ ":3\n");

	/* override term code after dfi_init_complete */
	err = set_term_code();
	if (err != 0U) {
		return INITDRAM_ERR_I;
	}
	MSG_LF(__func__ ":4\n");

	/* rx offset calibration */
	if (prr_cut > PRR_PRODUCT_11) {
		err = rx_offset_cal_hw();
	} else {
		err = rx_offset_cal();
	}
	if (err != 0U) {
		return INITDRAM_ERR_O;
	}
	MSG_LF(__func__ ":5\n");

	/* Dummy PDE */
	send_dbcmd(0x08840000U);

	/* PDX */
	send_dbcmd(0x08840001U);

	/* check register i/f is alive */
	err = ddrphy_regif_chk();
	if (err != 0U) {
		return INITDRAM_ERR_O;
	}
	MSG_LF(__func__ ":6\n");

	/* phy initialize end */

	/* setup DDR mode registers */
	/* CMOS MODE */
	change_lpddr4_en(0);

	/* MRS */
	ddr_register_set();

	/* Thermal sensor setting */
	/* THCTR Bit6: PONM=0 , Bit0: THSST=1  */
	data_l = (mmio_read_32(THS1_THCTR) & 0xFFFFFFBFU) | 0x00000001U;
	mmio_write_32(THS1_THCTR, data_l);

	/* LPDDR4 MODE */
	change_lpddr4_en(1);

	MSG_LF(__func__ ":7\n");

	/* mask CS_MAP if RANKx is not found */
	foreach_vch(ch) {
		data_l = ddr_getval(ch, _reg_PI_CS_MAP);
		if ((ch_have_this_cs[1] & (1U << ch)) == 0U) {
			data_l = data_l & 0x05U;
		}
		ddr_setval(ch, _reg_PI_CS_MAP, data_l);
	}

	/* exec pi_training */
	reg_ddrphy_write_a(ddr_regdef_adr(_reg_PHY_FREQ_SEL_MULTICAST_EN),
			   BIT(ddr_regdef_lsb(_reg_PHY_FREQ_SEL_MULTICAST_EN)));
	ddr_setval_ach_as(_reg_PHY_PER_CS_TRAINING_MULTICAST_EN, 0x00U);

	foreach_vch(ch) {
		for (slice = 0U; slice < SLICE_CNT; slice++) {
			ddr_setval_s(ch, slice,
				     _reg_PHY_PER_CS_TRAINING_EN,
				     ((ch_have_this_cs[1]) >> ch) & 0x01U);
		}
	}

	phytrainingok = pi_training_go();

	if (ddr_phyvalid != (phytrainingok & ddr_phyvalid)) {
		return INITDRAM_ERR_T | phytrainingok;
	}

	MSG_LF(__func__ ":8\n");

	/* CACS DLY ADJUST */
	data_l = board_cnf->cacs_dly + (uint32_t)_f_scale_adj(board_cnf->cacs_dly_adj);
	foreach_vch(ch) {
		for (i = 0U; i < _reg_PHY_CLK_CACS_SLAVE_DELAY_X_NUM; i++) {
			adj = _f_scale_adj(board_cnf->ch[ch].cacs_adj[i]);
			ddr_setval(ch, _reg_PHY_CLK_CACS_SLAVE_DELAY_X[i],
				   data_l + (uint32_t)adj);
		}

		if (ddr_phycaslice == 1U) {
			for (i = 0U; i < 6U; i++) {
				index = i + _reg_PHY_CLK_CACS_SLAVE_DELAY_X_NUM;
				adj = _f_scale_adj(board_cnf->ch[ch].cacs_adj[index]);
				ddr_setval_s(ch, 2U,
					     _reg_PHY_CLK_CACS_SLAVE_DELAY_X[i],
					     data_l + (uint32_t)adj);
			}
		}
	}

	update_dly();
	MSG_LF(__func__ ":9\n");

	/* Adjust write path latency */
	if (ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET, _reg_PHY_WRITE_PATH_LAT_ADD) != 0U) {
		adjust_wpath_latency();
	}

	/* RDQLVL Training */
	if (ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET, _reg_PHY_IE_MODE) == 0U) {
		ddr_setval_ach_as(_reg_PHY_IE_MODE, 0x01U);
	}

	err = rdqdm_man();

	if (ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET, _reg_PHY_IE_MODE) == 0U) {
		ddr_setval_ach_as(_reg_PHY_IE_MODE, 0x00U);
	}

	if (err != 0U) {
		return INITDRAM_ERR_T;
	}
	update_dly();
	MSG_LF(__func__ ":10\n");

	/* WDQLVL Training */
	err = wdqdm_man();
	if (err != 0U) {
		return INITDRAM_ERR_T;
	}
	update_dly();
	MSG_LF(__func__ ":11\n");

	dbsc_regset_post();
	MSG_LF(__func__ ":12\n");

	return phytrainingok;
}

/* SW LEVELING COMMON */
static uint32_t swlvl1(uint32_t ddr_csn, uint32_t reg_cs, uint32_t reg_kick)
{
	const uint32_t RETRY_MAX = 0x1000U;
	uint32_t ch, data_l;
	uint32_t waiting;
	uint32_t retry;
	uint32_t err = 0U;

	/* set EXIT -> OP_DONE is cleared */
	ddr_setval_ach(_reg_PI_SWLVL_EXIT, 0x01);

	/* kick */
	foreach_vch(ch) {
		if ((ch_have_this_cs[ddr_csn % 2U] & (1U << ch)) != 0U) {
			ddr_setval(ch, reg_cs, ddr_csn);
			ddr_setval(ch, reg_kick, 0x01U);
		}
	}
	foreach_vch(ch) {
		/*PREPARE ADDR REGISTER (for SWLVL_OP_DONE) */
		ddr_getval(ch, _reg_PI_SWLVL_OP_DONE);
	}
	waiting = ch_have_this_cs[ddr_csn % 2U];
	dsb_sev();
	retry = RETRY_MAX;
	do {
		foreach_vch(ch) {
			if ((waiting & (1U << ch)) == 0U) {
				continue;
			}
			data_l = ddr_getval(ch, _reg_PI_SWLVL_OP_DONE);
			if ((data_l & 0x01U) != 0U) {
				waiting &= ~(1U << ch);
			}
		}
		retry--;
	} while ((waiting != 0U) && (retry > 0U));
	if (retry == 0U) {
		err = 1U;
	}

	dsb_sev();
	/* set EXIT -> OP_DONE is cleared */
	ddr_setval_ach(_reg_PI_SWLVL_EXIT, 0x01U);
	dsb_sev();

	return err;
}

/* WDQ TRAINING */
#ifndef DDR_FAST_INIT
static void wdqdm_clr1(uint32_t ch, uint32_t ddr_csn)
{
	uint32_t cs, slice;
	uint32_t data_l;
	int32_t i, k;

	/* clr of training results buffer */
	cs = ddr_csn % 2U;
	data_l = board_cnf->dqdm_dly_w;
	for (slice = 0U; slice < SLICE_CNT; slice++) {
		k = (board_cnf->ch[ch].dqs_swap >> (4 * slice)) & 0x0f;
		if (((k >= 2) && (ddr_csn < 2)) || ((k < 2) && (ddr_csn >= 2))) {
			continue;
		}

		for (i = 0; i <= 8; i++) {
			if ((ch_have_this_cs[CS_CNT - 1 - cs] & (1U << ch)) != 0U) {
				wdqdm_dly[ch][cs][slice][i] =
				    wdqdm_dly[ch][CS_CNT - 1 - cs][slice][i];
			} else {
				wdqdm_dly[ch][cs][slice][i] = data_l;
			}
			wdqdm_le[ch][cs][slice][i] = 0U;
			wdqdm_te[ch][cs][slice][i] = 0U;
		}
		wdqdm_st[ch][cs][slice] = 0U;
		wdqdm_win[ch][cs][slice] = 0U;
	}
}

static uint32_t wdqdm_ana1(uint32_t ch, uint32_t ddr_csn)
{
	const uint32_t _par_WDQLVL_RETRY_THRES = 0x7c0U;
	uint32_t cs, slice;
	uint32_t data_l;
	int32_t min_win;
	int32_t i, k;
	uint32_t err;
	int32_t win;
	int8_t _adj;
	int16_t adj;
	uint32_t dq;

	/* analysis of training results */
	err = 0U;
	for (slice = 0U; slice < SLICE_CNT; slice += 1U) {
		k = (board_cnf->ch[ch].dqs_swap >> (4 * slice)) & 0x0f;
		if (((k >= 2) && (ddr_csn < 2)) || ((k < 2) && (ddr_csn >= 2))) {
			continue;
		}

		cs = ddr_csn % 2U;
		ddr_setval_s(ch, slice, _reg_PHY_PER_CS_TRAINING_INDEX, cs);
		for (i = 0; i < 9; i++) {
			dq = slice * 8U + i;
			if (i == 8) {
				_adj = board_cnf->ch[ch].dm_adj_w[slice];
			} else {
				_adj = board_cnf->ch[ch].dq_adj_w[dq];
			}
			adj = _f_scale_adj(_adj);

			data_l =
			    ddr_getval_s(ch, slice,
					 _reg_PHY_CLK_WRX_SLAVE_DELAY[i]) + adj;
			ddr_setval_s(ch, slice, _reg_PHY_CLK_WRX_SLAVE_DELAY[i],
				     data_l);
			wdqdm_dly[ch][cs][slice][i] = data_l;
		}
		ddr_setval_s(ch, slice, _reg_PHY_PER_CS_TRAINING_EN, 0x00);
		data_l = ddr_getval_s(ch, slice, _reg_PHY_WDQLVL_STATUS_OBS);
		wdqdm_st[ch][cs][slice] = data_l;
		min_win = INT_LEAST32_MAX;
		for (i = 0; i <= 8; i++) {
			ddr_setval_s(ch, slice, _reg_PHY_WDQLVL_DQDM_OBS_SELECT,
				     i);

			data_l =
			    ddr_getval_s(ch, slice,
					 _reg_PHY_WDQLVL_DQDM_TE_DLY_OBS);
			wdqdm_te[ch][cs][slice][i] = data_l;
			data_l =
			    ddr_getval_s(ch, slice,
					 _reg_PHY_WDQLVL_DQDM_LE_DLY_OBS);
			wdqdm_le[ch][cs][slice][i] = data_l;
			win = (int32_t)wdqdm_te[ch][cs][slice][i] -
			      wdqdm_le[ch][cs][slice][i];
			if (min_win > win) {
				min_win = win;
			}
			if (data_l >= _par_WDQLVL_RETRY_THRES) {
				err = 2;
			}
		}
		wdqdm_win[ch][cs][slice] = min_win;
		ddr_setval_s(ch, slice, _reg_PHY_PER_CS_TRAINING_EN,
			     ((ch_have_this_cs[1]) >> ch) & 0x01);
	}
	return err;
}
#endif/* DDR_FAST_INIT */

static void wdqdm_cp(uint32_t ddr_csn, uint32_t restore)
{
	uint32_t tgt_cs, src_cs;
	uint32_t ch, slice;
	uint32_t tmp_r;
	uint32_t i;

	/* copy of training results */
	foreach_vch(ch) {
		for (tgt_cs = 0U; tgt_cs < CS_CNT; tgt_cs++) {
			for (slice = 0U; slice < SLICE_CNT; slice++) {
				ddr_setval_s(ch, slice,
					     _reg_PHY_PER_CS_TRAINING_INDEX,
					     tgt_cs);
				src_cs = ddr_csn % 2U;
				if ((ch_have_this_cs[1] & (1U << ch)) == 0U) {
					src_cs = 0U;
				}
				for (i = 0U; i <= 4U; i += 4U) {
					if (restore != 0U) {
						tmp_r = rdqdm_dly[ch][tgt_cs][slice][i];
					} else {
						tmp_r = rdqdm_dly[ch][src_cs][slice][i];
					}

					ddr_setval_s(ch, slice,
						     _reg_PHY_RDDQS_X_RISE_SLAVE_DELAY[i],
						     tmp_r);
				}
			}
		}
	}
}

static uint32_t wdqdm_man1(void)
{
	uint32_t mr14_csab0_bak[DRAM_CH_CNT];
	uint32_t ch, cs, ddr_csn;
	uint32_t data_l;
	uint32_t err = 0U;
#ifndef DDR_FAST_INIT
	uint32_t err_flg = 0U;
#endif/* DDR_FAST_INIT */

	/* CLEAR PREV RESULT */
	for (cs = 0U; cs < CS_CNT; cs++) {
		ddr_setval_ach_as(_reg_PHY_PER_CS_TRAINING_INDEX, cs);
		ddr_setval_ach_as(_reg_PHY_WDQLVL_CLR_PREV_RESULTS, 0x01U);
	}
	ddrphy_regif_idle();

	for (ddr_csn = 0U; ddr_csn < CSAB_CNT; ddr_csn++) {
		if (((prr_product == PRR_PRODUCT_M3) &&
		     (prr_cut == PRR_PRODUCT_10))) {
			wdqdm_cp(ddr_csn, 0U);
		}

		foreach_vch(ch) {
			data_l = ddr_getval(ch, reg_pi_mr14_data_fx_csx[1][ddr_csn]);
			ddr_setval(ch, reg_pi_mr14_data_fx_csx[1][0], data_l);
		}

		/* KICK WDQLVL */
		err = swlvl1(ddr_csn, _reg_PI_WDQLVL_CS, _reg_PI_WDQLVL_REQ);
		if (err != 0U) {
			goto err_exit;
		}

		if (ddr_csn == 0U) {
			foreach_vch(ch) {
				mr14_csab0_bak[ch] = ddr_getval(ch,
								reg_pi_mr14_data_fx_csx[1][0]);
			}
		} else {
			foreach_vch(ch) {
				ddr_setval(ch, reg_pi_mr14_data_fx_csx[1][0],
					   mr14_csab0_bak[ch]);
			}
		}
#ifndef DDR_FAST_INIT
		foreach_vch(ch) {
			if ((ch_have_this_cs[ddr_csn % 2U] & (1U << ch)) == 0U) {
				wdqdm_clr1(ch, ddr_csn);
				continue;
			}
			err = wdqdm_ana1(ch, ddr_csn);
			if (err != 0U) {
				err_flg |= (1U << (ddr_csn * 4U + ch));
			}
			ddrphy_regif_idle();
		}
#endif/* DDR_FAST_INIT */
	}
err_exit:
#ifndef DDR_FAST_INIT
	err |= err_flg;
#endif/* DDR_FAST_INIT */

	return err;
}

static uint32_t wdqdm_man(void)
{
	uint32_t datal, ch, ddr_csn, mr14_bkup[4][4];
	const uint32_t retry_max = 0x10U;
	uint32_t err, retry_cnt;

	datal = RL + js2[js2_tdqsck] + (16U / 2U) + 1U - WL + 2U + 2U + 19U;
	if ((mmio_read_32(DBSC_DBTR(11)) & 0xFF) > datal) {
		datal = mmio_read_32(DBSC_DBTR(11)) & 0xFF;
	}
	ddr_setval_ach(_reg_PI_TDFI_WDQLVL_RW, datal);

	ddr_setval_ach(_reg_PI_TDFI_WDQLVL_WR,
		       (mmio_read_32(DBSC_DBTR(12)) & 0xFF) + 10);

	ddr_setval_ach(_reg_PI_TRFC_F0, mmio_read_32(DBSC_DBTR(13)) & 0x1FF);
	ddr_setval_ach(_reg_PI_TRFC_F1, mmio_read_32(DBSC_DBTR(13)) & 0x1FF);

	retry_cnt = 0U;
	err = 0U;
	do {
		ddr_setval_ach(_reg_PI_WDQLVL_VREF_EN, 0x01);
		ddr_setval_ach(_reg_PI_WDQLVL_VREF_NORMAL_STEPSIZE, 0x01);
		ddr_setval_ach(_reg_PI_WDQLVL_VREF_DELTA, 0x0C);
		dsb_sev();
		err = wdqdm_man1();
		foreach_vch(ch) {
			for (ddr_csn = 0U; ddr_csn < CSAB_CNT; ddr_csn++) {
				mr14_bkup[ch][ddr_csn] =
				    ddr_getval(ch, reg_pi_mr14_data_fx_csx
					       [1][ddr_csn]);
				dsb_sev();
			}
		}

		ddr_setval_ach(_reg_PI_WDQLVL_VREF_DELTA, 0x04);

		pvtcode_update();
		err = wdqdm_man1();
		foreach_vch(ch) {
			for (ddr_csn = 0U; ddr_csn < CSAB_CNT; ddr_csn++) {
				mr14_bkup[ch][ddr_csn] =
				    (mr14_bkup[ch][ddr_csn] +
				     ddr_getval(ch, reg_pi_mr14_data_fx_csx
						[1][ddr_csn])) / 2U;
				ddr_setval(ch,
					   reg_pi_mr14_data_fx_csx[1]
					   [ddr_csn],
					   mr14_bkup[ch][ddr_csn]);
			}
		}

		ddr_setval_ach(_reg_PI_WDQLVL_VREF_NORMAL_STEPSIZE, 0x0U);
		ddr_setval_ach(_reg_PI_WDQLVL_VREF_DELTA, 0x0U);
		ddr_setval_ach(_reg_PI_WDQLVL_VREF_INITIAL_START_POINT, 0x0U);
		ddr_setval_ach(_reg_PI_WDQLVL_VREF_INITIAL_STOP_POINT, 0x0U);
		ddr_setval_ach(_reg_PI_WDQLVL_VREF_INITIAL_STEPSIZE, 0x0U);

		pvtcode_update2();
		err = wdqdm_man1();
		ddr_setval_ach(_reg_PI_WDQLVL_VREF_EN, 0x0U);

	} while ((err != 0U) && (++retry_cnt < retry_max));

	if (prr_product == PRR_PRODUCT_M3 && prr_cut <= PRR_PRODUCT_10) {
		wdqdm_cp(0U, 1U);
	}

	return (retry_cnt >= retry_max);
}

/* RDQ TRAINING */
#ifndef DDR_FAST_INIT
static void rdqdm_clr1(uint32_t ch, uint32_t ddr_csn)
{
	uint32_t cs, slice;
	uint32_t data_l;
	int32_t i, k;

	/* clr of training results buffer */
	cs = ddr_csn % 2U;
	data_l = board_cnf->dqdm_dly_r;
	for (slice = 0U; slice < SLICE_CNT; slice++) {
		k = (board_cnf->ch[ch].dqs_swap >> (4 * slice)) & 0x0f;
		if (((k >= 2) && (ddr_csn < 2)) || ((k < 2) && (ddr_csn >= 2))) {
			continue;
		}

		for (i = 0; i <= 8; i++) {
			if ((ch_have_this_cs[CS_CNT - 1 - cs] & (1U << ch)) != 0U) {
				rdqdm_dly[ch][cs][slice][i] =
				    rdqdm_dly[ch][CS_CNT - 1 - cs][slice][i];
				rdqdm_dly[ch][cs][slice + SLICE_CNT][i] =
				    rdqdm_dly[ch][CS_CNT - 1 - cs][slice + SLICE_CNT][i];
			} else {
				rdqdm_dly[ch][cs][slice][i] = data_l;
				rdqdm_dly[ch][cs][slice + SLICE_CNT][i] = data_l;
			}
			rdqdm_le[ch][cs][slice][i] = 0U;
			rdqdm_le[ch][cs][slice + SLICE_CNT][i] = 0U;
			rdqdm_te[ch][cs][slice][i] = 0U;
			rdqdm_te[ch][cs][slice + SLICE_CNT][i] = 0U;
			rdqdm_nw[ch][cs][slice][i] = 0U;
			rdqdm_nw[ch][cs][slice + SLICE_CNT][i] = 0U;
		}
		rdqdm_st[ch][cs][slice] = 0U;
		rdqdm_win[ch][cs][slice] = 0U;
	}
}

static uint32_t rdqdm_ana1(uint32_t ch, uint32_t ddr_csn)
{
	uint32_t rdq_status_obs_select;
	uint32_t cs, slice;
	uint32_t data_l;
	uint32_t err;
	uint32_t dq;
	int32_t min_win;
	int8_t _adj;
	int16_t adj;
	int32_t min_win;
	int32_t win;
	int32_t i, k;

	/* analysis of training results */
	err = 0U;
	for (slice = 0U; slice < SLICE_CNT; slice++) {
		k = (board_cnf->ch[ch].dqs_swap >> (4 * slice)) & 0x0f;
		if (((k >= 2) && (ddr_csn < 2)) || ((k < 2) && (ddr_csn >= 2))) {
			continue;
		}

		cs = ddr_csn % 2U;
		ddr_setval_s(ch, slice, _reg_PHY_PER_CS_TRAINING_INDEX, cs);
		ddrphy_regif_idle();

		ddr_getval_s(ch, slice, _reg_PHY_PER_CS_TRAINING_INDEX);
		ddrphy_regif_idle();

		for (i = 0; i <= 8; i++) {
			dq = slice * 8 + i;
			if (i == 8) {
				_adj = board_cnf->ch[ch].dm_adj_r[slice];
			} else {
				_adj = board_cnf->ch[ch].dq_adj_r[dq];
			}

			adj = _f_scale_adj(_adj);

			data_l = ddr_getval_s(ch, slice,
					      _reg_PHY_RDDQS_X_RISE_SLAVE_DELAY[i]) + adj;
			ddr_setval_s(ch, slice,
				     _reg_PHY_RDDQS_X_RISE_SLAVE_DELAY[i],
				     data_l);
			rdqdm_dly[ch][cs][slice][i] = data_l;

			data_l = ddr_getval_s(ch, slice,
					      _reg_PHY_RDDQS_X_FALL_SLAVE_DELAY[i]) + adj;
			ddr_setval_s(ch, slice,
				     _reg_PHY_RDDQS_X_FALL_SLAVE_DELAY[i],
				     data_l);
			rdqdm_dly[ch][cs][slice + SLICE_CNT][i] = data_l;
		}
		min_win = INT_LEAST32_MAX;
		for (i = 0; i <= 8; i++) {
			data_l =
			    ddr_getval_s(ch, slice, _reg_PHY_RDLVL_STATUS_OBS);
			rdqdm_st[ch][cs][slice] = data_l;
			rdqdm_st[ch][cs][slice + SLICE_CNT] = data_l;
			/* k : rise/fall */
			for (k = 0; k < 2; k++) {
				if (i == 8) {
					rdq_status_obs_select = 16 + 8 * k;
				} else {
					rdq_status_obs_select = i + k * 8;
				}
				ddr_setval_s(ch, slice,
					     _reg_PHY_RDLVL_RDDQS_DQ_OBS_SELECT,
					     rdq_status_obs_select);

				data_l =
				    ddr_getval_s(ch, slice,
						 _reg_PHY_RDLVL_RDDQS_DQ_LE_DLY_OBS);
				rdqdm_le[ch][cs][slice + SLICE_CNT * k][i] = data_l;

				data_l =
				    ddr_getval_s(ch, slice,
						 _reg_PHY_RDLVL_RDDQS_DQ_TE_DLY_OBS);
				rdqdm_te[ch][cs][slice + SLICE_CNT * k][i] = data_l;

				data_l =
				    ddr_getval_s(ch, slice,
						 _reg_PHY_RDLVL_RDDQS_DQ_NUM_WINDOWS_OBS);
				rdqdm_nw[ch][cs][slice + SLICE_CNT * k][i] = data_l;

				win =
				    (int32_t)rdqdm_te[ch][cs][slice +
							      SLICE_CNT *
							      k][i] -
				    rdqdm_le[ch][cs][slice + SLICE_CNT * k][i];
				if (i != 8) {
					if (min_win > win) {
						min_win = win;
					}
				}
			}
		}
		rdqdm_win[ch][cs][slice] = min_win;
		if (min_win <= 0) {
			err = 2;
		}
	}
	return err;
}
#else /* DDR_FAST_INIT */
static void rdqdm_man1_set(uint32_t ddr_csn, uint32_t ch, uint32_t slice)
{
	uint32_t i, adj, data_l;

	for (i = 0U; i <= 8U; i++) {
		if (i == 8U) {
			adj = _f_scale_adj(board_cnf->ch[ch].dm_adj_r[slice]);
		} else {
			adj = _f_scale_adj(board_cnf->ch[ch].dq_adj_r[slice * 8U + i]);
		}
		ddr_setval_s(ch, slice, _reg_PHY_PER_CS_TRAINING_INDEX, ddr_csn);
		data_l = ddr_getval_s(ch, slice, _reg_PHY_RDDQS_X_RISE_SLAVE_DELAY[i]) + adj;
		ddr_setval_s(ch, slice, _reg_PHY_RDDQS_X_RISE_SLAVE_DELAY[i], data_l);
		rdqdm_dly[ch][ddr_csn][slice][i] = data_l;
		rdqdm_dly[ch][ddr_csn | 1U][slice][i] = data_l;

		data_l = ddr_getval_s(ch, slice, _reg_PHY_RDDQS_X_FALL_SLAVE_DELAY[i]) + adj;
		ddr_setval_s(ch, slice, _reg_PHY_RDDQS_X_FALL_SLAVE_DELAY[i], data_l);
		rdqdm_dly[ch][ddr_csn][slice + SLICE_CNT][i] = data_l;
		rdqdm_dly[ch][ddr_csn | 1U][slice + SLICE_CNT][i] = data_l;
	}
}
#endif /* DDR_FAST_INIT */

static uint32_t rdqdm_man1(void)
{
	uint32_t ch;
	uint32_t ddr_csn;
	uint32_t val;
#ifdef DDR_FAST_INIT
	uint32_t slice;
#endif/* DDR_FAST_INIT */
	uint32_t err;

	/* manual execution of training */
	err = 0U;

	for (ddr_csn = 0U; ddr_csn < CSAB_CNT; ddr_csn++) {
		/* KICK RDQLVL */
		err = swlvl1(ddr_csn, _reg_PI_RDLVL_CS, _reg_PI_RDLVL_REQ);
		if (err != 0U) {
			goto err_exit;
		}
#ifndef DDR_FAST_INIT
		foreach_vch(ch) {
			if ((ch_have_this_cs[ddr_csn % 2] & (1U << ch)) == 0U) {
				rdqdm_clr1(ch, ddr_csn);
				ddrphy_regif_idle();
				continue;
			}
			err = rdqdm_ana1(ch, ddr_csn);
			ddrphy_regif_idle();
			if (err != 0U)  {
				goto err_exit;
			}
		}
#else/* DDR_FAST_INIT */
		foreach_vch(ch) {
			if ((ch_have_this_cs[ddr_csn] & (1U << ch)) != 0U) {
				for (slice = 0U; slice < SLICE_CNT; slice++) {
					val = ddr_getval_s(ch, slice, _reg_PHY_RDLVL_STATUS_OBS);
					if (val != 0x0D00FFFFU) {
						err = (1U << ch) | (0x10U << slice);
						goto err_exit;
					}
				}
			}
			if ((prr_product == PRR_PRODUCT_M3) &&
			    (prr_cut <= PRR_PRODUCT_10)) {
				for (slice = 0U; slice < SLICE_CNT; slice++) {
					rdqdm_man1_set(ddr_csn, ch, slice);
				}
			}
		}
		ddrphy_regif_idle();

#endif/* DDR_FAST_INIT */
	}

err_exit:
	return err;
}

static uint32_t rdqdm_man(void)
{
	uint32_t err, retry_cnt;
	const uint32_t retry_max = 0x01U;

	ddr_setval_ach_as(_reg_PHY_DQ_TSEL_ENABLE,
			  0x00000004U | ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET,
						     _reg_PHY_DQ_TSEL_ENABLE));
	ddr_setval_ach_as(_reg_PHY_DQS_TSEL_ENABLE,
			  0x00000004U | ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET,
						     _reg_PHY_DQS_TSEL_ENABLE));
	ddr_setval_ach_as(_reg_PHY_DQ_TSEL_SELECT,
			  0xFF0FFFFFU & ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET,
						     _reg_PHY_DQ_TSEL_SELECT));
	ddr_setval_ach_as(_reg_PHY_DQS_TSEL_SELECT,
			  0xFF0FFFFFU & ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET,
						     _reg_PHY_DQS_TSEL_SELECT));

	retry_cnt = 0U;
	do {
		err = rdqdm_man1();
		ddrphy_regif_idle();
	} while ((err != 0U) && (++retry_cnt < retry_max));
	ddr_setval_ach_as(_reg_PHY_DQ_TSEL_ENABLE,
			  ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET,
					_reg_PHY_DQ_TSEL_ENABLE));
	ddr_setval_ach_as(_reg_PHY_DQS_TSEL_ENABLE,
			  ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET,
					_reg_PHY_DQS_TSEL_ENABLE));
	ddr_setval_ach_as(_reg_PHY_DQ_TSEL_SELECT,
			  ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET,
					_reg_PHY_DQ_TSEL_SELECT));
	ddr_setval_ach_as(_reg_PHY_DQS_TSEL_SELECT,
			  ddrtbl_getval(_cnf_DDR_PHY_SLICE_REGSET,
					_reg_PHY_DQS_TSEL_SELECT));

	return (retry_cnt >= retry_max);
}

/* rx offset calibration */
static int32_t _find_change(uint64_t val, uint32_t dir)
{
	int32_t i;
	uint32_t startval;
	uint32_t curval;
	const int32_t VAL_END = 0x3fU;

	if (dir == 0U) {
		startval = (val & 0x01U);
		for (i = 1; i <= VAL_END; i++) {
			curval = (val >> i) & 0x01U;
			if (curval != startval) {
				return i;
			}
		}
		return VAL_END;
	}

	startval = (val >> dir) & 0x01U;
	for (i = (int32_t)dir - 1; i >= 0; i--) {
		curval = (val >> i) & 0x01U;
		if (curval != startval) {
			return i;
		}
	}

	return 0;
}

static uint32_t _rx_offset_cal_updn(uint32_t code)
{
	const uint32_t CODE_MAX = 0x40U;
	uint32_t tmp;

	if (code == 0U) {
		tmp = (1U << 6) | (CODE_MAX - 1U);
	} else {
		tmp = (code << 6) | (CODE_MAX - code);
	}

	return tmp;
}

static uint32_t rx_offset_cal(void)
{
	uint32_t index;
	uint32_t code;
	const uint32_t CODE_MAX = 0x40U;
	const uint32_t CODE_STEP = 2U;
	uint32_t ch, slice;
	uint32_t tmp;
	uint32_t tmp_ach_as[DRAM_CH_CNT][SLICE_CNT];
	uint64_t val[DRAM_CH_CNT][SLICE_CNT][_reg_PHY_RX_CAL_X_NUM];
	uint64_t tmpval;
	int32_t lsb, msb;

	ddr_setval_ach_as(_reg_PHY_RX_CAL_OVERRIDE, 0x01);
	foreach_vch(ch) {
		for (slice = 0U; slice < SLICE_CNT; slice++) {
			for (index = 0U; index < _reg_PHY_RX_CAL_X_NUM; index++) {
				val[ch][slice][index] = 0U;
			}
		}
	}

	for (code = 0U; code < CODE_MAX / CODE_STEP; code++) {
		tmp = _rx_offset_cal_updn(code * CODE_STEP);
		for (index = 0U; index < _reg_PHY_RX_CAL_X_NUM; index++) {
			ddr_setval_ach_as(_reg_PHY_RX_CAL_X[index], tmp);
		}
		dsb_sev();
		ddr_getval_ach_as(_reg_PHY_RX_CAL_OBS, (uint32_t *)tmp_ach_as);

		foreach_vch(ch) {
			for (slice = 0U; slice < SLICE_CNT; slice++) {
				tmp = tmp_ach_as[ch][slice];
				for (index = 0U; index < _reg_PHY_RX_CAL_X_NUM;
				     index++) {
					if ((tmp & (1U << index)) != 0U) {
						val[ch][slice][index] |=
						    (1ULL << code);
					} else {
						val[ch][slice][index] &=
						    ~(1ULL << code);
					}
				}
			}
		}
	}
	foreach_vch(ch) {
		for (slice = 0U; slice < SLICE_CNT; slice++) {
			for (index = 0U; index < _reg_PHY_RX_CAL_X_NUM;
			     index++) {
				tmpval = val[ch][slice][index];
				lsb = _find_change(tmpval, 0U);
				msb = _find_change(tmpval,
						   (CODE_MAX / CODE_STEP) - 1U);
				tmp = (lsb + msb) >> 1U;

				tmp = _rx_offset_cal_updn(tmp * CODE_STEP);
				ddr_setval_s(ch, slice,
					     _reg_PHY_RX_CAL_X[index], tmp);
			}
		}
	}
	ddr_setval_ach_as(_reg_PHY_RX_CAL_OVERRIDE, 0x00);

	return 0U;
}

static uint32_t rx_offset_cal_hw(void)
{
	uint32_t ch, slice;
	uint32_t retry;
	uint32_t complete;
	uint32_t tmp;
	uint32_t tmp_ach_as[DRAM_CH_CNT][SLICE_CNT];

	ddr_setval_ach_as(_reg_PHY_RX_CAL_X[9], 0x00);
	ddr_setval_ach_as(_reg_PHY_RX_CAL_OVERRIDE, 0x00);
	ddr_setval_ach_as(_reg_PHY_RX_CAL_SAMPLE_WAIT, 0x0f);

	retry = 0U;
	while (retry < 4096U) {
		if ((retry & 0xffU) == 0U) {
			ddr_setval_ach_as(_reg_SC_PHY_RX_CAL_START, 0x01);
		}
		foreach_vch(ch)  {
			for (slice = 0U; slice < SLICE_CNT; slice++) {
				tmp_ach_as[ch][slice] =
					ddr_getval_s(ch, slice,
						     _reg_PHY_RX_CAL_X[9]);
			}
		}

		complete = 1U;
		foreach_vch(ch) {
			for (slice = 0U; slice < SLICE_CNT; slice++) {
				tmp = tmp_ach_as[ch][slice];
				tmp = (tmp & 0x3fU) + ((tmp >> 6) & 0x3fU);
				if (tmp != 0x40U) {
					complete = 0U;
				}
			}
		}
		if (complete != 0U) {
			break;
		}

		retry++;
	}

	return (complete == 0U);
}

/* adjust wpath latency */
static void adjust_wpath_latency(void)
{
	uint32_t ch, cs, slice;
	uint32_t dly;
	uint32_t wpath_add;
	const uint32_t _par_EARLY_THRESHOLD_VAL = 0x180U;

	foreach_vch(ch) {
		for (slice = 0U; slice < SLICE_CNT; slice += 1U) {
			for (cs = 0U; cs < CS_CNT; cs++) {
				ddr_setval_s(ch, slice,
					     _reg_PHY_PER_CS_TRAINING_INDEX,
					     cs);
				ddr_getval_s(ch, slice,
					     _reg_PHY_PER_CS_TRAINING_INDEX);
				dly =
				    ddr_getval_s(ch, slice,
						 _reg_PHY_CLK_WRDQS_SLAVE_DELAY);
				if (dly <= _par_EARLY_THRESHOLD_VAL) {
					continue;
				}

				wpath_add =
				    ddr_getval_s(ch, slice,
						 _reg_PHY_WRITE_PATH_LAT_ADD);
				ddr_setval_s(ch, slice,
					     _reg_PHY_WRITE_PATH_LAT_ADD,
					     wpath_add - 1U);
			}
		}
	}
}

/* DDR Initialize entry */
int32_t rzg_dram_init(void)
{
	uint32_t ch, cs;
	uint32_t data_l;
	uint32_t bus_mbps, bus_mbpsdiv;
	uint32_t tmp_tccd;
	uint32_t failcount;
	uint32_t cnf_boardtype;
	int32_t ret = INITDRAM_NG;

	/* Thermal sensor setting */
	data_l = mmio_read_32(CPG_MSTPSR5);
	if ((data_l & BIT(22)) != 0U) {	/*  case THS/TSC Standby */
		data_l &= ~BIT(22);
		cpg_write_32(CPG_SMSTPCR5, data_l);
		while ((mmio_read_32(CPG_MSTPSR5) & BIT(22)) != 0U) {
			/*  wait bit=0 */
		}
	}

	/* THCTR Bit6: PONM=0 , Bit0: THSST=0   */
	data_l = mmio_read_32(THS1_THCTR) & 0xFFFFFFBE;
	mmio_write_32(THS1_THCTR, data_l);

	/* Judge product and cut */
#ifdef RCAR_DDR_FIXED_LSI_TYPE
#if (RCAR_LSI == RCAR_AUTO)
	prr_product = mmio_read_32(PRR) & PRR_PRODUCT_MASK;
	prr_cut = mmio_read_32(PRR) & PRR_CUT_MASK;
#else /* RCAR_LSI */
#ifndef RCAR_LSI_CUT
	prr_cut = mmio_read_32(PRR) & PRR_CUT_MASK;
#endif /* RCAR_LSI_CUT */
#endif /* RCAR_LSI */
#else /* RCAR_DDR_FIXED_LSI_TYPE */
	prr_product = mmio_read_32(PRR) & PRR_PRODUCT_MASK;
	prr_cut = mmio_read_32(PRR) & PRR_CUT_MASK;
#endif /* RCAR_DDR_FIXED_LSI_TYPE */

	if (prr_product == PRR_PRODUCT_M3) {
		p_ddr_regdef_tbl =
			(const uint32_t *)&DDR_REGDEF_TBL[1][0];
	} else {
		FATAL_MSG("BL2: DDR:Unknown Product\n");
		goto done;
	}

	if ((prr_product == PRR_PRODUCT_M3) && (prr_cut < PRR_PRODUCT_30)) {
		/* non : G2M Ver.1.x not support */
	} else {
		mmio_write_32(DBSC_DBSYSCNT0, 0x00001234U);
	}

	/* Judge board type */
	cnf_boardtype = boardcnf_get_brd_type(prr_product);
	if (cnf_boardtype >= (uint32_t)BOARDNUM) {
		FATAL_MSG("BL2: DDR:Unknown Board\n");
		goto done;
	}
	board_cnf = (const struct _boardcnf *)&boardcnfs[cnf_boardtype];

/* RCAR_DRAM_SPLIT_2CH           (2U) */
#if RCAR_DRAM_SPLIT == 2
	ddr_phyvalid = board_cnf->phyvalid;
#else /* RCAR_DRAM_SPLIT_2CH */
	ddr_phyvalid = board_cnf->phyvalid;
#endif /* RCAR_DRAM_SPLIT_2CH */

	max_density = 0U;

	for (cs = 0U; cs < CS_CNT; cs++) {
		ch_have_this_cs[cs] = 0U;
	}

	foreach_ech(ch) {
		for (cs = 0U; cs < CS_CNT; cs++) {
			ddr_density[ch][cs] = 0xffU;
		}
	}

	foreach_vch(ch) {
		for (cs = 0U; cs < CS_CNT; cs++) {
			data_l = board_cnf->ch[ch].ddr_density[cs];
			ddr_density[ch][cs] = data_l;

			if (data_l == 0xffU) {
				continue;
			}
			if (data_l > max_density) {
				max_density = data_l;
			}
			ch_have_this_cs[cs] |= (1U << ch);
		}
	}

	/* Judge board clock frequency (in MHz) */
	boardcnf_get_brd_clk(cnf_boardtype, &brd_clk, &brd_clkdiv);
	if ((brd_clk / brd_clkdiv) > 25U) {
		brd_clkdiva = 1U;
	} else {
		brd_clkdiva = 0U;
	}

	/* Judge ddr operating frequency clock(in Mbps) */
	boardcnf_get_ddr_mbps(cnf_boardtype, &ddr_mbps, &ddr_mbpsdiv);

	ddr0800_mul = CLK_DIV(800U, 2U, brd_clk, brd_clkdiv * (brd_clkdiva + 1U));

	ddr_mul = CLK_DIV(ddr_mbps, ddr_mbpsdiv * 2U, brd_clk,
			  brd_clkdiv * (brd_clkdiva + 1U));

	/* Adjust tccd */
	data_l = (0x00006000 & mmio_read_32(RST_MODEMR)) >> 13;
	bus_mbps = 0U;
	bus_mbpsdiv = 0U;
	switch (data_l) {
	case 0:
		bus_mbps = brd_clk * 0x60U * 2U;
		bus_mbpsdiv = brd_clkdiv * 1U;
		break;
	case 1:
		bus_mbps = brd_clk * 0x50U * 2U;
		bus_mbpsdiv = brd_clkdiv * 1U;
		break;
	case 2:
		bus_mbps = brd_clk * 0x40U * 2U;
		bus_mbpsdiv = brd_clkdiv * 1U;
		break;
	case 3:
		bus_mbps = brd_clk * 0x60U * 2U;
		bus_mbpsdiv = brd_clkdiv * 2U;
		break;
	default:
		bus_mbps = brd_clk * 0x60U * 2U;
		bus_mbpsdiv = brd_clkdiv * 2U;
		WARN("BL2: DDR using default values for adjusting tccd");
		break;
	}
	tmp_tccd = CLK_DIV(ddr_mbps * 8U, ddr_mbpsdiv, bus_mbps, bus_mbpsdiv);
	if (8U * ddr_mbps * bus_mbpsdiv != tmp_tccd * bus_mbps * ddr_mbpsdiv) {
		tmp_tccd = tmp_tccd + 1U;
	}

	if (tmp_tccd < 8U) {
		ddr_tccd = 8U;
	} else {
		ddr_tccd = tmp_tccd;
	}

	NOTICE("BL2: DDR%d(%s)\n", ddr_mbps / ddr_mbpsdiv, RCAR_DDR_VERSION);

	MSG_LF("Start\n");

	/* PLL Setting */
	pll3_control(1U);

	/* initialize DDR */
	data_l = init_ddr();
	if (data_l == ddr_phyvalid) {
		failcount = 0U;
	} else {
		failcount = 1U;
	}

	foreach_vch(ch) {
	    mmio_write_32(DBSC_DBPDLK(ch), 0x00000000U);
	}
	if ((prr_product == PRR_PRODUCT_M3) && (prr_cut < PRR_PRODUCT_30)) {
		/* non : G2M Ver.1.x not support */
	} else {
		mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);
	}

	if (failcount == 0U) {
		ret = INITDRAM_OK;
	}

done:
	return ret;
}

static void pvtcode_update(void)
{
	uint32_t ch;
	uint32_t data_l;
	uint32_t pvtp[4], pvtn[4], pvtp_init, pvtn_init;
	int32_t pvtp_tmp, pvtn_tmp;

	foreach_vch(ch) {
		pvtn_init = (tcal.tcomp_cal[ch] & 0xFC0U) >> 6;
		pvtp_init = (tcal.tcomp_cal[ch] & 0x03FU) >> 0;

		if (8912U * pvtp_init > 44230U) {
			pvtp_tmp = (5000U + 8912U * pvtp_init - 44230U) / 10000U;
		} else {
			pvtp_tmp =
			    -((-(5000 + 8912 * pvtp_init - 44230)) / 10000);
		}
		pvtn_tmp = (5000U + 5776U * (uint32_t)pvtn_init + 30280U) / 10000U;

		pvtn[ch] = (uint32_t)pvtn_tmp + pvtn_init;
		pvtp[ch] = (uint32_t)pvtp_tmp + pvtp_init;

		if (pvtn[ch] > 63U) {
			pvtn[ch] = 63U;
			pvtp[ch] =
			    (pvtp_tmp) * (63 - 6 * pvtn_tmp -
					  pvtn_init) / (pvtn_tmp) +
			    6 * pvtp_tmp + pvtp_init;
		}

		data_l = pvtp[ch] | (pvtn[ch] << 6) | 0x00015000U;
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_FDBK_TERM),
				 data_l | 0x00020000U);
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_DATA_TERM),
				 data_l);
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_DQS_TERM),
				 data_l);
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_ADDR_TERM),
				 data_l);
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_CS_TERM),
				 data_l);
	}
}

static void pvtcode_update2(void)
{
	uint32_t ch;

	foreach_vch(ch) {
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_FDBK_TERM),
				 tcal.init_cal[ch] | 0x00020000U);
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_DATA_TERM),
				 tcal.init_cal[ch]);
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_DQS_TERM),
				 tcal.init_cal[ch]);
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_ADDR_TERM),
				 tcal.init_cal[ch]);
		reg_ddrphy_write(ch, ddr_regdef_adr(_reg_PHY_PAD_CS_TERM),
				 tcal.init_cal[ch]);
	}
}

static void ddr_padcal_tcompensate_getinit(uint32_t override)
{
	uint32_t ch;
	uint32_t data_l;
	uint32_t pvtp, pvtn;

	tcal.init_temp = 0;
	for (ch = 0U; ch < 4U; ch++) {
		tcal.init_cal[ch] = 0U;
		tcal.tcomp_cal[ch] = 0U;
	}

	foreach_vch(ch) {
		tcal.init_cal[ch] = ddr_getval(ch, _reg_PHY_PAD_TERM_X[1]);
		tcal.tcomp_cal[ch] = ddr_getval(ch, _reg_PHY_PAD_TERM_X[1]);
	}

	if (override == 0U) {
		data_l = mmio_read_32(THS1_TEMP);
		if (data_l < 2800U) {
			tcal.init_temp =
			    (143 * (int32_t)data_l - 359000) / 1000;
		} else {
			tcal.init_temp =
			    (121 * (int32_t)data_l - 296300) / 1000;
		}

		foreach_vch(ch) {
			pvtp = (tcal.init_cal[ch] >> 0) & 0x000003FU;
			pvtn = (tcal.init_cal[ch] >> 6) & 0x000003FU;
			if ((int32_t)pvtp >
			    ((tcal.init_temp * 29 - 3625) / 1000)) {
				pvtp = (int32_t)pvtp +
				    ((3625 - tcal.init_temp * 29) / 1000);
			} else {
				pvtp = 0U;
			}

			if ((int32_t)pvtn >
			    ((tcal.init_temp * 54 - 6750) / 1000)) {
				pvtn = (int32_t)pvtn +
				    ((6750 - tcal.init_temp * 54) / 1000);
			} else {
				pvtn = 0U;
			}

			tcal.init_cal[ch] = 0x00015000U | (pvtn << 6) | pvtp;
		}
		tcal.init_temp = 125;
	}
}

#ifndef DDR_QOS_INIT_SETTING
/* For QoS init */
uint8_t rzg_get_boardcnf_phyvalid(void)
{
	return ddr_phyvalid;
}
#endif /* DDR_QOS_INIT_SETTING */
