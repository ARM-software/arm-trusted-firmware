/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define BOARDNUM 2
#define BOARD_JUDGE_AUTO

#ifdef BOARD_JUDGE_AUTO
static uint32_t _board_judge(uint32_t prr_product);

static uint32_t boardcnf_get_brd_type(uint32_t prr_product)
{
	return _board_judge(prr_product);
}
#else /* BOARD_JUDGE_AUTO */
static uint32_t boardcnf_get_brd_type(void)
{
	return 1U;
}
#endif /* BOARD_JUDGE_AUTO */

#define DDR_FAST_INIT

struct _boardcnf_ch {
	uint8_t ddr_density[CS_CNT];
	uint64_t ca_swap;
	uint16_t dqs_swap;
	uint32_t dq_swap[SLICE_CNT];
	uint8_t dm_swap[SLICE_CNT];
	uint16_t wdqlvl_patt[16];
	int8_t cacs_adj[16];
	int8_t dm_adj_w[SLICE_CNT];
	int8_t dq_adj_w[SLICE_CNT * 8U];
	int8_t dm_adj_r[SLICE_CNT];
	int8_t dq_adj_r[SLICE_CNT * 8U];
};

struct _boardcnf {
	uint8_t phyvalid;
	uint8_t dbi_en;
	uint16_t cacs_dly;
	int16_t cacs_dly_adj;
	uint16_t dqdm_dly_w;
	uint16_t dqdm_dly_r;
	struct _boardcnf_ch ch[DRAM_CH_CNT];
};

#define WDQLVL_PAT {\
	0x00AA,\
	0x0055,\
	0x00AA,\
	0x0155,\
	0x01CC,\
	0x0133,\
	0x00CC,\
	0x0033,\
	0x00F0,\
	0x010F,\
	0x01F0,\
	0x010F,\
	0x00F0,\
	0x00F0,\
	0x000F,\
	0x010F}

static const struct _boardcnf boardcnfs[BOARDNUM] = {
	{
/* boardcnf[0] HopeRun HiHope RZ/G2M 16Gbit/1rank/2ch board with G2M SoC */
	 .phyvalid = 0x03,
	 .dbi_en = 0x01,
	 .cacs_dly = 0x02c0,
	 .cacs_dly_adj = 0,
	 .dqdm_dly_w = 0x0300,
	 .dqdm_dly_r = 0x00a0,
	 .ch = {
		{
		 {0x04, 0xff},
		 0x00345201U,
		 0x3201,
		 {0x01672543U, 0x45361207U, 0x45632107U, 0x60715234U},
		 {0x08, 0x08, 0x08, 0x08},
		 WDQLVL_PAT,
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0}
		},

		{
		 {0x04, 0xff},
		 0x00302154U,
		 0x2310,
		 {0x01672543U, 0x45361207U, 0x45632107U, 0x60715234U},
		 {0x08, 0x08, 0x08, 0x08},
		 WDQLVL_PAT,
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0}
		}
		}
	},
/* boardcnf[1] HopeRun HiHope RZ/G2M 8Gbit/2rank/2ch board with G2M SoC */
	{
	 0x03,
	 0x01,
	 0x02c0,
	 0,
	 0x0300,
	 0x00a0,
	{
		{
		 {0x02, 0x02},
		 0x00345201U,
		 0x3201,
		 {0x01672543U, 0x45361207U, 0x45632107U, 0x60715234U},
		 {0x08, 0x08, 0x08, 0x08},
		 WDQLVL_PAT,
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0}
		},
		{
		 {0x02, 0x02},
		 0x00302154U,
		 0x2310,
		 {0x01672543U, 0x45361207U, 0x45632107U, 0x60715234U},
		 {0x08, 0x08, 0x08, 0x08},
		 WDQLVL_PAT,
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0}
		}
	}
	}
};

void boardcnf_get_brd_clk(uint32_t brd, uint32_t *clk, uint32_t *div)
{
	uint32_t md;

	md = (mmio_read_32(RST_MODEMR) >> 13) & 0x3U;
	switch (md) {
	case 0x0U:
		*clk = 50U;
		*div = 3U;
		break;
	case 0x1U:
		*clk = 60U;
		*div = 3U;
		break;
	case 0x2U:
		*clk = 75U;
		*div = 3U;
		break;
	case 0x3U:
		*clk = 100U;
		*div = 3U;
		break;
	default:
		break;
	}
	(void)brd;
}

void boardcnf_get_ddr_mbps(uint32_t brd, uint32_t *mbps, uint32_t *div)
{
	uint32_t md;

	md = (mmio_read_32(RST_MODEMR) >> 17U) & 0x5U;
	md = (md | (md >> 1U)) & 0x3U;
	switch (md) {
	case 0x0U:
		*mbps = 3200U;
		*div = 1U;
		break;
	case 0x1U:
		*mbps = 2800U;
		*div = 1U;
		break;
	case 0x2U:
		*mbps = 2400U;
		*div = 1U;
		break;
	case 0x3U:
		*mbps = 1600U;
		*div = 1U;
		break;
	default:
		break;
	}
	(void)brd;
}

#define _def_REFPERIOD  1890

#define M3_SAMPLE_TT_A84        0xB866CC10U, 0x3B250421U
#define M3_SAMPLE_TT_A85        0xB866CC10U, 0x3AA50421U
#define M3_SAMPLE_TT_A86        0xB866CC10U, 0x3AA48421U
#define M3_SAMPLE_FF_B45        0xB866CC10U, 0x3AB00C21U
#define M3_SAMPLE_FF_B49        0xB866CC10U, 0x39B10C21U
#define M3_SAMPLE_FF_B56        0xB866CC10U, 0x3AAF8C21U
#define M3_SAMPLE_SS_E24        0xB866CC10U, 0x3BA39421U
#define M3_SAMPLE_SS_E28        0xB866CC10U, 0x3C231421U
#define M3_SAMPLE_SS_E32        0xB866CC10U, 0x3C241421U

static const uint32_t termcode_by_sample[20][3] = {
	{ M3_SAMPLE_TT_A84, 0x000158D5U },
	{ M3_SAMPLE_TT_A85, 0x00015955U },
	{ M3_SAMPLE_TT_A86, 0x00015955U },
	{ M3_SAMPLE_FF_B45, 0x00015690U },
	{ M3_SAMPLE_FF_B49, 0x00015753U },
	{ M3_SAMPLE_FF_B56, 0x00015793U },
	{ M3_SAMPLE_SS_E24, 0x00015996U },
	{ M3_SAMPLE_SS_E28, 0x000159D7U },
	{ M3_SAMPLE_SS_E32, 0x00015997U },
	{ 0xFFFFFFFFU, 0xFFFFFFFFU, 0x0001554FU}
};

#ifdef BOARD_JUDGE_AUTO
/* Board detect function */
#define GPIO_INDT5	0xE605500CU
#define LPDDR4_2RANK	(0x01U << 25U)

static uint32_t _board_judge(uint32_t prr_product)
{
	uint32_t boardInfo;
	uint32_t boardid = 1U;

	if (prr_product == PRR_PRODUCT_M3) {
		if ((mmio_read_32(PRR) & PRR_CUT_MASK) != RCAR_M3_CUT_VER11) {
			boardInfo = mmio_read_32(GPIO_INDT5);
			if ((boardInfo & LPDDR4_2RANK) == 0U) {
				boardid = 0U;
			}
		}
	}

	return boardid;
}
#endif /* BOARD_JUDGE_AUTO */
