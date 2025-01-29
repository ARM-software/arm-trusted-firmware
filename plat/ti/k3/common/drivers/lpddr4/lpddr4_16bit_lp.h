/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_16BIT_LP_H
#define LPDDR4_16BIT_LP_H

#include <stdio.h>
#include <string.h>

#define LPDDR4_CTL_LP_REGS_SET                  422
#define LPDDR4_PHY_DS_LP_REGS_SET               149
#define LPDDR4_PHY_ADR_LP_REGS_SET              42
#define LPDDR4_PHY_TOP_LP_REGS_SET              141
#define LPDDR4_PI_LP_REGS_SET                   58
#define LPDDR4_DS2_REG_OFFSET                   256
#define LPDDR4_PHY_DS_FREQ_INDEX_2_REG_OFFSET   97
#define LPDDR4_PHY_ADR_FREQ_INDEX_2_REG_OFFSET  31
#define LPDDR4_PHY_TOP_FREQ_INDEX_2_REG_OFFSET  108

static uint32_t ctllpregnumretention[LPDDR4_CTL_LP_REGS_SET] = { 0, };
static uint32_t phylpdsregnumretention[(uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * (uint32_t)DSLICE_NUM] = { 0, };
static uint32_t phylpadrregnumretention[(uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * (uint32_t)ASLICE_NUM] = { 0, };
static uint32_t phylptopregnumretention[LPDDR4_PHY_TOP_LP_REGS_SET] = { 0, };
static uint32_t pilpregnumretention[LPDDR4_PI_LP_REGS_SET] = { 0, };
static uint32_t ctllpregvalretention[LPDDR4_CTL_LP_REGS_SET] = { 0, };
static uint32_t phylpdsregvalretention[(uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * (uint32_t)DSLICE_NUM] = { 0, };
static uint32_t phylpadrregvalretention[(uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * (uint32_t)ASLICE_NUM] = { 0, };
static uint32_t phylptopregvalretention[LPDDR4_PHY_TOP_LP_REGS_SET] = { 0, };
static uint32_t pilpregvalretention[LPDDR4_PI_LP_REGS_SET] = { 0, };
extern void populatelowpowerconfigarray(void);

static void populatephydsregistersfn1(void)
{
	phylpdsregnumretention[0] = 0;          /* PHY_LP4_BOOT_PAD_DSLICE_IO_CFG_0, PHY_LP4_BOOT_RX_PCLK_CLK_SEL_0 */
	phylpdsregnumretention[1] = 5;          /* PHY_LP4_BOOT_RDDATA_EN_DLY_0, PHY_LP4_BOOT_RDDATA_EN_IE_DLY_0 */
	phylpdsregnumretention[2] = 6;          /* PHY_LP4_BOOT_WRPATH_GATE_DISABLE_0, PHY_LP4_BOOT_RDDQS_LATENCY_ADJUST_0, PHY_LP4_BOOT_RPTR_UPDATE_0, PHY_LP4_BOOT_RDDATA_EN_TSEL_DLY_0 */
	phylpdsregnumretention[3] = 7;          /* PHY_LP4_BOOT_RDDATA_EN_OE_DLY_0 */
	phylpdsregnumretention[4] = 8;          /* PHY_GATE_DELAY_COMP_DISABLE_0, PHY_LPBK_DFX_TIMEOUT_EN_0 */
	phylpdsregnumretention[5] = 9;          /* PHY_AUTO_TIMING_MARGIN_CONTROL_0 */
	phylpdsregnumretention[6] = 10;         /* PHY_AUTO_TIMING_MARGIN_OBS_0 */
	phylpdsregnumretention[7] = 11;         /* PHY_PDA_MODE_EN_0, PHY_DQ_IDLE_0 */
	phylpdsregnumretention[8] = 12;         /* PHY_RDLVL_MULTI_PATT_RST_DISABLE_0, PHY_RDLVL_MULTI_PATT_ENABLE_0 */
	phylpdsregnumretention[9] = 14;         /* PHY_GATE_SMPL1_SLAVE_DELAY_0, PHY_GATE_ERROR_DELAY_SELECT_0 */
	phylpdsregnumretention[10] = 15;        /* PHY_GATE_SMPL2_SLAVE_DELAY_0, PHY_MEM_CLASS_0, PHY_LPDDR_0 */
	phylpdsregnumretention[11] = 16;        /* ON_FLY_GATE_ADJUST_EN_0 */
	phylpdsregnumretention[12] = 18;        /* PHY_LP4_PST_AMBLE_0 */
	phylpdsregnumretention[13] = 19;        /* PHY_RDLVL_PATT8_0 */
	phylpdsregnumretention[14] = 20;        /* PHY_RDLVL_PATT9_0 */
	phylpdsregnumretention[15] = 21;        /* PHY_RDLVL_PATT10_0 */
	phylpdsregnumretention[16] = 22;        /* PHY_RDLVL_PATT11_0 */
	phylpdsregnumretention[17] = 23;        /* PHY_RDLVL_PATT12_0 */
	phylpdsregnumretention[18] = 24;        /* PHY_RDLVL_PATT13_0 */
	phylpdsregnumretention[19] = 25;        /* PHY_RDLVL_PATT14_0 */
	phylpdsregnumretention[20] = 26;        /* PHY_RDLVL_PATT15_0 */
	phylpdsregnumretention[21] = 27;        /* PHY_SW_FIFO_PTR_RST_DISABLE_0, PHY_SLAVE_LOOP_CNT_UPDATE_0 */
	phylpdsregnumretention[22] = 29;        /* PHY_WRLVL_CAPTURE_CNT_0, PHY_WRLVL_ALGO_0 */
	phylpdsregnumretention[23] = 30;        /* PHY_GTLVL_UPDT_WAIT_CNT_0, PHY_GTLVL_CAPTURE_CNT_0, PHY_DQ_MASK_0, PHY_WRLVL_UPDT_WAIT_CNT_0 */
	phylpdsregnumretention[24] = 31;        /* PHY_RDLVL_OP_MODE_0, PHY_RDLVL_UPDT_WAIT_CNT_0, PHY_RDLVL_CAPTURE_CNT_0 */
	phylpdsregnumretention[25] = 32;        /* PHY_RDLVL_DATA_SWIZZLE_0,PHY_RDLVL_DATA_MASK_0 */
}

static void populatephydsregistersfn2(void)
{
	phylpdsregnumretention[26] = 33;        /* PHY_WDQLVL_PATT_0, PHY_WDQLVL_BURST_CNT_0, PHY_WDQLVL_CLK_JITTER_TOLERANCE_0 */
	phylpdsregnumretention[27] = 34;        /* PHY_WDQLVL_UPDT_WAIT_CNT_0, PHY_WDQLVL_DQDM_SLV_DLY_JUMP_OFFSET_0 */
	phylpdsregnumretention[28] = 35;        /* PHY_WDQLVL_DM_DLY_STEP_0, PHY_WDQLVL_DQ_SLV_DELTA_0 */
	phylpdsregnumretention[29] = 36;        /* PHY_WDQLVL_DATADM_MASK_0 */
	phylpdsregnumretention[30] = 37;        /* PHY_USER_PATT0_0 */
	phylpdsregnumretention[31] = 38;        /* PHY_USER_PATT1_0 */
	phylpdsregnumretention[32] = 39;        /* PHY_USER_PATT2_0 */
	phylpdsregnumretention[33] = 40;        /* PHY_USER_PATT3_0 */
	phylpdsregnumretention[34] = 41;        /* PHY_NTP_MULT_TRAIN_0, PHY_USER_PATT4_0 */
	phylpdsregnumretention[35] = 42;        /* PHY_NTP_PERIOD_THRESHOLD_0, PHY_NTP_EARLY_THRESHOLD_0 */
	phylpdsregnumretention[36] = 43;        /* PHY_NTP_PERIOD_THRESHOLD_MAX_0, PHY_NTP_PERIOD_THRESHOLD_MIN_0 */
	phylpdsregnumretention[37] = 44;        /* PHY_CALVL_VREF_DRIVING_SLICE_0 */
	phylpdsregnumretention[38] = 47;        /* PHY_MEAS_DLY_STEP_VALUE_0 */
	phylpdsregnumretention[39] = 61;        /* PHY_DDL_MODE_0 */
	phylpdsregnumretention[40] = 62;        /* PHY_DDL_MASK_0 */
	phylpdsregnumretention[41] = 65;        /* PHY_RX_CAL_DQ0_0, PHY_LP4_WDQS_OE_EXTEND_0, PHY_DDL_TRACK_UPD_THRESHOLD_0 */
	phylpdsregnumretention[42] = 66;        /* PHY_RX_CAL_DQ2_0, PHY_RX_CAL_DQ1_0 */
	phylpdsregnumretention[43] = 67;        /* PHY_RX_CAL_DQ4_0, PHY_RX_CAL_DQ3_0 */
	phylpdsregnumretention[44] = 68;        /* PHY_RX_CAL_DQ6_0, PHY_RX_CAL_DQ5_0 */
	phylpdsregnumretention[45] = 69;        /* PHY_RX_CAL_DQ7_0 */
	phylpdsregnumretention[46] = 70;        /* PHY_RX_CAL_DM_0 */
	phylpdsregnumretention[47] = 71;        /* PHY_RX_CAL_FDBK_0, PHY_RX_CAL_DQS_0 */
	phylpdsregnumretention[48] = 72;        /* PHY_FDBK_PWR_CTRL_0, PHY_STATIC_TOG_DISABLE_0, PHY_PAD_RX_BIAS_EN_0 */
	phylpdsregnumretention[49] = 73;        /* PHY_SLICE_PWR_RDC_DISABLE_0, PHY_DCC_RXCAL_CTRL_GATE_DISABLE_0, PHY_RDPATH_GATE_DISABLE_0, PHY_SLV_DLY_CTRL_GATE_DISABLE_0 */
}

static void populatephydsregistersfn3(void)
{
	phylpdsregnumretention[50] = 74;        /* PHY_DQS_TSEL_ENABLE_0, PHY_DQ_TSEL_SELECT_0, PHY_DQ_TSEL_ENABLE_0 */
	phylpdsregnumretention[51] = 75;        /* PHY_VREF_INITIAL_START_POINT_0, PHY_TWO_CYC_PREAMBLE_0, PHY_DQS_TSEL_SELECT_0 */
	phylpdsregnumretention[52] = 76;        /* PHY_NTP_WDQ_STEP_SIZE_0, PHY_NTP_TRAIN_EN_0, PHY_VREF_TRAINING_CTRL_0, PHY_VREF_INITIAL_STOP_POINT_0 */
	phylpdsregnumretention[53] = 77;        /* PHY_NTP_WDQ_STOP_0, PHY_NTP_WDQ_START_0 */
	phylpdsregnumretention[54] = 78;        /* PHY_WDQLVL_DVW_MIN_0, PHY_NTP_WDQ_BIT_EN_0 */
	phylpdsregnumretention[55] = 79;        /* PHY_PAD_TX_DCD_0, PHY_FAST_LVL_EN_0, PHY_WDQLVL_PER_START_OFFSET_0, PHY_PAD_RX_DCD_0_0 */
	phylpdsregnumretention[56] = 80;        /* PHY_PAD_RX_DCD_1_0, PHY_PAD_RX_DCD_2_0, PHY_PAD_RX_DCD_3_0, PHY_PAD_RX_DCD_4_0 */
	phylpdsregnumretention[57] = 81;        /* PHY_PAD_RX_DCD_5_0, PHY_PAD_RX_DCD_6_0, PHY_PAD_RX_DCD_7_0, PHY_PAD_DM_RX_DCD_0 */
	phylpdsregnumretention[58] = 82;        /* PHY_PAD_DSLICE_IO_CFG_0, PHY_PAD_FDBK_RX_DCD_0, PHY_PAD_DQS_RX_DCD_0 */
	phylpdsregnumretention[59] = 83;        /* PHY_RDDQ1_SLAVE_DELAY_0, PHY_RDDQ0_SLAVE_DELAY_0 */
	phylpdsregnumretention[60] = 84;        /* PHY_RDDQ3_SLAVE_DELAY_0, PHY_RDDQ2_SLAVE_DELAY_0 */
	phylpdsregnumretention[61] = 85;        /* PHY_RDDQ5_SLAVE_DELAY_0, PHY_RDDQ4_SLAVE_DELAY_0 */
	phylpdsregnumretention[62] = 86;        /* PHY_RDDQ7_SLAVE_DELAY_0, PHY_RDDQ6_SLAVE_DELAY_0 */
	phylpdsregnumretention[63] = 87;        /* PHY_RX_CAL_ALL_DLY_0, PHY_RX_PCLK_CLK_SEL_0, PHY_RDDM_SLAVE_DELAY_0 */
	phylpdsregnumretention[64] = 88;        /* PHY_DQS_OE_TIMING_0, PHY_DQ_TSEL_WR_TIMING_0, PHY_DQ_TSEL_RD_TIMING_0, PHY_DQ_OE_TIMING_0 */
	phylpdsregnumretention[65] = 89;        /* PHY_DQS_TSEL_WR_TIMING_0, PHY_DQS_OE_RD_TIMING_0, PHY_DQS_TSEL_RD_TIMING_0, PHY_IO_PAD_DELAY_TIMING_0 */
	phylpdsregnumretention[66] = 90;        /* PHY_PAD_VREF_CTRL_DQ_0, PHY_VREF_SETTING_TIME_0 */
	phylpdsregnumretention[67] = 91;        /* PHY_IE_MODE_0, PHY_RDDATA_EN_IE_DLY_0, PHY_DQS_IE_TIMING_0, PHY_DQ_IE_TIMING_0 */
	phylpdsregnumretention[68] = 92;        /* PHY_WDQLVL_RDDATA_EN_TSEL_DLY_0, PHY_WDQLVL_RDDATA_EN_DLY_0, PHY_WDQLVL_IE_ON_0, PHY_DBI_MODE_0 */
	phylpdsregnumretention[69] = 93;        /* PHY_SW_MASTER_MODE_0, PHY_RDDATA_EN_OE_DLY_0, PHY_RDDATA_EN_TSEL_DLY_0 */
	phylpdsregnumretention[70] = 94;        /* PHY_MASTER_DELAY_WAIT_0, PHY_MASTER_DELAY_STEP_0, PHY_MASTER_DELAY_START_0 */
	phylpdsregnumretention[71] = 95;        /* PHY_WRLVL_DLY_FINE_STEP_0, PHY_WRLVL_DLY_STEP_0, PHY_RPTR_UPDATE_0, PHY_MASTER_DELAY_HALF_MEASURE_0 */
	phylpdsregnumretention[72] = 96;        /* PHY_GTLVL_RESP_WAIT_CNT_0, PHY_WDQLVL_DLY_STEP_0, PHY_WRLVL_RESP_WAIT_CNT_0 */
	phylpdsregnumretention[73] = 97;        /* PHY_GTLVL_FINAL_STEP_0, PHY_GTLVL_BACK_STEP_0 */
	phylpdsregnumretention[74] = 98;        /* PHY_WDQLVL_DM_SEARCH_RANGE_0, PHY_WDQLVL_QTR_DLY_STEP_0, PHY_WDQLVL_DLY_STEP_0 */
	phylpdsregnumretention[75] = 99;        /* PHY_RDLVL_DLY_STEP_0, PHY_TOGGLE_PRE_SUPPORT_0 */
	phylpdsregnumretention[76] = 100;       /* PHY_RDLVL_MAX_EDGE_0 */
	phylpdsregnumretention[77] = 101;       /* PHY_MEAS_DLY_STEP_ENABLE_0, PHY_WDQ_OSC_DELTA_0, PHY_WRPATH_GATE_TIMING_0, PHY_WRPATH_GATE_DISABLE_0 */
	phylpdsregnumretention[78] = 102;       /* PHY_RDDATA_EN_DLY_0 */
	phylpdsregnumretention[79] = 103;       /* PHY_DQ_DM_SWIZZLE0_0 */
	phylpdsregnumretention[80] = 104;       /* PHY_DQ_DM_SWIZZLE1_0 */
	phylpdsregnumretention[81] = 105;       /* PHY_CLK_WRDQ1_SLAVE_DELAY_0, PHY_CLK_WRDQ0_SLAVE_DELAY_0 */
	phylpdsregnumretention[82] = 106;       /* PHY_CLK_WRDQ3_SLAVE_DELAY_0, PHY_CLK_WRDQ2_SLAVE_DELAY_0 */
	phylpdsregnumretention[83] = 107;       /* PHY_CLK_WRDQ5_SLAVE_DELAY_0, PHY_CLK_WRDQ4_SLAVE_DELAY_0 */
	phylpdsregnumretention[84] = 108;       /* PHY_CLK_WRDQ7_SLAVE_DELAY_0, PHY_CLK_WRDQ6_SLAVE_DELAY_0 */
	phylpdsregnumretention[85] = 109;       /* PHY_CLK_WRDQS_SLAVE_DELAY_0, PHY_CLK_WRDM_SLAVE_DELAY_0  */
	phylpdsregnumretention[86] = 110;       /* PHY_RDDQS_DQ0_RISE_SLAVE_DELAY_0, PHY_WRLVL_THRESHOLD_ADJUST_0 */
	phylpdsregnumretention[87] = 111;       /* PHY_RDDQS_DQ1_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ0_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[88] = 112;       /* PHY_RDDQS_DQ2_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ1_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[89] = 113;       /* PHY_RDDQS_DQ3_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ2_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[90] = 114;       /* PHY_RDDQS_DQ4_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ3_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[91] = 115;       /* PHY_RDDQS_DQ5_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ4_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[92] = 120;       /* PHY_WRLVL_DELAY_EARLY_THRESHOLD_0, PHY_WRITE_PATH_LAT_ADD_0, PHY_RDDQS_LATENCY_ADJUST_0 */
	phylpdsregnumretention[93] = 121;       /* PHY_WRLVL_EARLY_FORCE_ZERO_0, PHY_WRLVL_DELAY_PERIOD_THRESHOLD_0 */
	phylpdsregnumretention[94] = 122;       /* PHY_GTLVL_LAT_ADJ_START_0, PHY_GTLVL_RDDQS_SLV_DLY_START_0 */
	phylpdsregnumretention[95] = 123;       /* PHY_NTP_PASS_0, PHY_NTP_WRLAT_START_0, PHY_WDQLVL_DQDM_SLV_DLY_START_0 */
	phylpdsregnumretention[96] = 124;       /* PHY_RDLVL_RDDQS_DQ_SLV_DLY_START_0, PHY_DSLICE_PAD_RX_CTLE_SETTING_0, PHY_DSLICE_PAD_BOOSTPN_SETTING_0 */

	phylpdsregnumretention[97] = 74;        /* PHY_DQS_TSEL_ENABLE_0, PHY_DQ_TSEL_SELECT_0, PHY_DQ_TSEL_ENABLE_0 */
	phylpdsregnumretention[98] = 75;        /* PHY_VREF_INITIAL_START_POINT_0, PHY_TWO_CYC_PREAMBLE_0, PHY_DQS_TSEL_SELECT_0 */
	phylpdsregnumretention[99] = 76;        /* PHY_NTP_WDQ_STEP_SIZE_0, PHY_NTP_TRAIN_EN_0, PHY_VREF_TRAINING_CTRL_0, PHY_VREF_INITIAL_STOP_POINT_0 */
}

static void populatephydsregistersfn4(void)
{
	phylpdsregnumretention[100] = 77;       /* PHY_NTP_WDQ_STOP_0, PHY_NTP_WDQ_START_0 */
	phylpdsregnumretention[101] = 78;       /* PHY_WDQLVL_DVW_MIN_0, PHY_NTP_WDQ_BIT_EN_0 */
	phylpdsregnumretention[102] = 79;       /* PHY_PAD_TX_DCD_0, PHY_FAST_LVL_EN_0, PHY_WDQLVL_PER_START_OFFSET_0, PHY_PAD_RX_DCD_0_0 */
	phylpdsregnumretention[103] = 80;       /* PHY_PAD_RX_DCD_1_0, PHY_PAD_RX_DCD_2_0, PHY_PAD_RX_DCD_3_0, PHY_PAD_RX_DCD_4_0 */
	phylpdsregnumretention[104] = 81;       /* PHY_PAD_RX_DCD_5_0, PHY_PAD_RX_DCD_6_0, PHY_PAD_RX_DCD_7_0, PHY_PAD_DM_RX_DCD_0 */
	phylpdsregnumretention[105] = 82;       /* PHY_PAD_DSLICE_IO_CFG_0, PHY_PAD_FDBK_RX_DCD_0, PHY_PAD_DQS_RX_DCD_0 */
	phylpdsregnumretention[106] = 83;       /* PHY_RDDQ1_SLAVE_DELAY_0, PHY_RDDQ0_SLAVE_DELAY_0 */
	phylpdsregnumretention[107] = 84;       /* PHY_RDDQ3_SLAVE_DELAY_0, PHY_RDDQ2_SLAVE_DELAY_0 */
	phylpdsregnumretention[108] = 85;       /* PHY_RDDQ5_SLAVE_DELAY_0, PHY_RDDQ4_SLAVE_DELAY_0 */
	phylpdsregnumretention[109] = 86;       /* PHY_RDDQ7_SLAVE_DELAY_0, PHY_RDDQ6_SLAVE_DELAY_0 */
	phylpdsregnumretention[110] = 87;       /* PHY_RX_CAL_ALL_DLY_0, PHY_RX_PCLK_CLK_SEL_0, PHY_RDDM_SLAVE_DELAY_0 */
	phylpdsregnumretention[111] = 88;       /* PHY_DQS_OE_TIMING_0, PHY_DQ_TSEL_WR_TIMING_0, PHY_DQ_TSEL_RD_TIMING_0, PHY_DQ_OE_TIMING_0 */
	phylpdsregnumretention[112] = 89;       /* PHY_DQS_TSEL_WR_TIMING_0, PHY_DQS_OE_RD_TIMING_0, PHY_DQS_TSEL_RD_TIMING_0, PHY_IO_PAD_DELAY_TIMING_0 */
	phylpdsregnumretention[113] = 90;       /* PHY_PAD_VREF_CTRL_DQ_0, PHY_VREF_SETTING_TIME_0 */
	phylpdsregnumretention[114] = 91;       /* PHY_IE_MODE_0, PHY_RDDATA_EN_IE_DLY_0, PHY_DQS_IE_TIMING_0, PHY_DQ_IE_TIMING_0 */
	phylpdsregnumretention[115] = 92;       /* PHY_WDQLVL_RDDATA_EN_TSEL_DLY_0, PHY_WDQLVL_RDDATA_EN_DLY_0, PHY_WDQLVL_IE_ON_0, PHY_DBI_MODE_0 */
	phylpdsregnumretention[116] = 93;       /* PHY_SW_MASTER_MODE_0, PHY_RDDATA_EN_OE_DLY_0, PHY_RDDATA_EN_TSEL_DLY_0 */
	phylpdsregnumretention[117] = 94;       /* PHY_MASTER_DELAY_WAIT_0, PHY_MASTER_DELAY_STEP_0, PHY_MASTER_DELAY_START_0 */
	phylpdsregnumretention[118] = 95;       /* PHY_WRLVL_DLY_FINE_STEP_0, PHY_WRLVL_DLY_STEP_0, PHY_RPTR_UPDATE_0, PHY_MASTER_DELAY_HALF_MEASURE_0 */
	phylpdsregnumretention[119] = 96;       /* PHY_GTLVL_RESP_WAIT_CNT_0, PHY_WDQLVL_DLY_STEP_0, PHY_WRLVL_RESP_WAIT_CNT_0 */
	phylpdsregnumretention[120] = 97;       /* PHY_GTLVL_FINAL_STEP_0, PHY_GTLVL_BACK_STEP_0 */
	phylpdsregnumretention[121] = 98;       /* PHY_WDQLVL_DM_SEARCH_RANGE_0, PHY_WDQLVL_QTR_DLY_STEP_0, PHY_WDQLVL_DLY_STEP_0 */
	phylpdsregnumretention[122] = 99;       /* PHY_RDLVL_DLY_STEP_0, PHY_TOGGLE_PRE_SUPPORT_0 */
	phylpdsregnumretention[123] = 100;      /* PHY_RDLVL_MAX_EDGE_0 */
	phylpdsregnumretention[124] = 101;      /* PHY_MEAS_DLY_STEP_ENABLE_0, PHY_WDQ_OSC_DELTA_0, PHY_WRPATH_GATE_TIMING_0, PHY_WRPATH_GATE_DISABLE_0 */
	phylpdsregnumretention[125] = 102;      /* PHY_RDDATA_EN_DLY_0 */
	phylpdsregnumretention[126] = 103;      /* PHY_DQ_DM_SWIZZLE0_0 */
	phylpdsregnumretention[127] = 104;      /* PHY_DQ_DM_SWIZZLE1_0 */
	phylpdsregnumretention[128] = 105;      /* PHY_CLK_WRDQ1_SLAVE_DELAY_0, PHY_CLK_WRDQ0_SLAVE_DELAY_0 */
	phylpdsregnumretention[129] = 106;      /* PHY_CLK_WRDQ3_SLAVE_DELAY_0, PHY_CLK_WRDQ2_SLAVE_DELAY_0 */
	phylpdsregnumretention[130] = 107;      /* PHY_CLK_WRDQ5_SLAVE_DELAY_0, PHY_CLK_WRDQ4_SLAVE_DELAY_0 */
	phylpdsregnumretention[131] = 108;      /* PHY_CLK_WRDQ7_SLAVE_DELAY_0, PHY_CLK_WRDQ6_SLAVE_DELAY_0 */
	phylpdsregnumretention[132] = 109;      /* PHY_CLK_WRDQS_SLAVE_DELAY_0, PHY_CLK_WRDM_SLAVE_DELAY_0  */
	phylpdsregnumretention[133] = 110;      /* PHY_RDDQS_DQ0_RISE_SLAVE_DELAY_0, PHY_WRLVL_THRESHOLD_ADJUST_0 */
	phylpdsregnumretention[134] = 111;      /* PHY_RDDQS_DQ1_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ0_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[135] = 112;      /* PHY_RDDQS_DQ2_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ1_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[136] = 113;      /* PHY_RDDQS_DQ3_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ2_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[137] = 114;      /* PHY_RDDQS_DQ4_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ3_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[138] = 115;      /* PHY_RDDQS_DQ5_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ4_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[139] = 116;      /* PHY_RDDQS_DQ6_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ5_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[140] = 117;      /* PHY_RDDQS_DQ7_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ6_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[141] = 118;      /* PHY_RDDQS_DM_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ7_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[142] = 119;      /* PHY_RDDQS_GATE_SLAVE_DELAY_0, PHY_RDDQS_DM_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[143] = 117;      /* PHY_RDDQS_DQ7_RISE_SLAVE_DELAY_0, PHY_RDDQS_DQ6_FALL_SLAVE_DELAY_0 */
	phylpdsregnumretention[144] = 120;      /* PHY_WRLVL_DELAY_EARLY_THRESHOLD_0, PHY_WRITE_PATH_LAT_ADD_0, PHY_RDDQS_LATENCY_ADJUST_0 */
	phylpdsregnumretention[145] = 121;      /* PHY_WRLVL_EARLY_FORCE_ZERO_0, PHY_WRLVL_DELAY_PERIOD_THRESHOLD_0 */
	phylpdsregnumretention[146] = 122;      /* PHY_GTLVL_LAT_ADJ_START_0, PHY_GTLVL_RDDQS_SLV_DLY_START_0 */
	phylpdsregnumretention[147] = 123;      /* PHY_NTP_PASS_0, PHY_NTP_WRLAT_START_0, PHY_WDQLVL_DQDM_SLV_DLY_START_0 */
	phylpdsregnumretention[148] = 124;      /* PHY_RDLVL_RDDQS_DQ_SLV_DLY_START_0, PHY_DSLICE_PAD_RX_CTLE_SETTING_0, PHY_DSLICE_PAD_BOOSTPN_SETTING_0 */
}

static void populatephydsregisters(void)
{
	uint32_t aindex;

	populatephydsregistersfn1();
	populatephydsregistersfn2();
	populatephydsregistersfn3();
	populatephydsregistersfn4();
	for (aindex = LPDDR4_PHY_DS_LP_REGS_SET; aindex < ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * (uint32_t)DSLICE_NUM); aindex++)
		phylpdsregnumretention[aindex] = phylpdsregnumretention[aindex - (uint32_t)LPDDR4_PHY_DS_LP_REGS_SET] + (uint32_t)LPDDR4_DS2_REG_OFFSET;
}

static void populatephyasregistersfn1(void)
{
	phylpadrregnumretention[0] = 516;       /* PHY_ADR_TSEL_ENABLE_0, PHY_ADR_SLAVE_LOOP_CNT_UPDATE_0 */
	phylpadrregnumretention[1] = 517;       /* PHY_ADR_PWR_RDC_DISABLE_0, PHY_ADR_PRBS_PATTERN_MASK_0, PHY_ADR_PRBS_PATTERN_START_0 */
	phylpadrregnumretention[2] = 518;       /* PHY_ADR_IE_MODE_0, PHY_ADR_TYPE_0, PHY_ADR_SLV_DLY_CTRL_GATE_DISABLE_0 */
	phylpadrregnumretention[3] = 523;       /* PHY_ADR_CALVL_COARSE_DLY_0, PHY_ADR_CALVL_START_0 */
	phylpadrregnumretention[4] = 524;       /* PHY_ADR_CALVL_QTR_0 */
	phylpadrregnumretention[5] = 525;       /* PHY_ADR_CALVL_SWIZZLE0_0 */
	phylpadrregnumretention[6] = 526;       /* PHY_ADR_CALVL_RANK_CTRL_0, PHY_ADR_CALVL_SWIZZLE1_0 */
	phylpadrregnumretention[7] = 527;       /* PHY_ADR_CALVL_PERIODIC_START_OFFSET_0, PHY_ADR_CALVL_RESP_WAIT_CNT_0, PHY_ADR_CALVL_NUM_PATTERNS_0 */
	phylpadrregnumretention[8] = 532;       /* PHY_ADR_CALVL_FG_0_0 */
	phylpadrregnumretention[9] = 533;       /* PHY_ADR_CALVL_BG_0_0 */
	phylpadrregnumretention[10] = 534;      /* PHY_ADR_CALVL_FG_1_0 */
	phylpadrregnumretention[11] = 535;      /* PHY_ADR_CALVL_BG_1_0 */
	phylpadrregnumretention[12] = 536;      /* PHY_ADR_CALVL_FG_2_0 */
	phylpadrregnumretention[13] = 537;      /* PHY_ADR_CALVL_BG_2_0 */
	phylpadrregnumretention[14] = 538;      /* PHY_ADR_CALVL_FG_3_0 */
	phylpadrregnumretention[15] = 539;      /* PHY_ADR_CALVL_BG_3_0 */
	phylpadrregnumretention[16] = 540;      /* PHY_ADR_ADDR_SEL_0 */
	phylpadrregnumretention[17] = 541;      /* PHY_ADR_SEG_MASK_0, PHY_ADR_BIT_MASK_0, PHY_ADR_LP4_BOOT_SLV_DELAY_0 */
	phylpadrregnumretention[18] = 542;      /* PHY_ADR_SW_TXIO_CTRL_0, PHY_ADR_STATIC_TOG_DISABLE_0, PHY_ADR_CSLVL_TRAIN_MASK_0, PHY_ADR_CALVL_TRAIN_MASK_0 */
	phylpadrregnumretention[19] = 543;      /* PHY_ADR_SW_TXPWR_CTRL_0 */

	phylpadrregnumretention[20] = 544;      /* PHY_PAD_ADR_RX_PCLK_CLK_SEL_0, PHY_PAD_ADR_IO_CFG_0, PHY_ADR_TSEL_SELECT_0 */
	phylpadrregnumretention[21] = 545;      /* PHY_ADR1_SW_WRADDR_SHIFT_0, PHY_ADR0_CLK_WR_SLAVE_DELAY_0, PHY_ADR0_SW_WRADDR_SHIFT_0 */
	phylpadrregnumretention[22] = 546;      /* PHY_ADR2_SW_WRADDR_SHIFT_0, PHY_ADR1_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[23] = 547;      /* PHY_ADR3_SW_WRADDR_SHIFT_0, PHY_ADR2_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[24] = 548;      /* PHY_ADR4_SW_WRADDR_SHIFT_0, PHY_ADR3_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[25] = 549;      /* PHY_ADR5_SW_WRADDR_SHIFT_0, PHY_ADR4_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[26] = 550;      /* PHY_ADR_SW_MASTER_MODE_0, PHY_ADR5_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[27] = 551;      /* PHY_ADR_MASTER_DELAY_WAIT_0, PHY_ADR_MASTER_DELAY_STEP_0, PHY_ADR_MASTER_DELAY_START_0 */
	phylpadrregnumretention[28] = 552;      /* PHY_ADR_SW_CALVL_DVW_MIN_EN_0, PHY_ADR_SW_CALVL_DVW_MIN_0, PHY_ADR_MASTER_DELAY_HALF_MEASURE_0 */
	phylpadrregnumretention[29] = 553;      /* PHY_ADR_CALVL_DLY_STEP_0 */
	phylpadrregnumretention[30] = 554;      /* PHY_ADR_MEAS_DLY_STEP_ENABLE_0, PHY_ADR_CALVL_CAPTURE_CNT_0 */

	phylpadrregnumretention[31] = 544;      /* PHY_PAD_ADR_RX_PCLK_CLK_SEL_0, PHY_PAD_ADR_IO_CFG_0, PHY_ADR_TSEL_SELECT_0 */
	phylpadrregnumretention[32] = 545;      /* PHY_ADR1_SW_WRADDR_SHIFT_0, PHY_ADR0_CLK_WR_SLAVE_DELAY_0, PHY_ADR0_SW_WRADDR_SHIFT_0 */
	phylpadrregnumretention[33] = 546;      /* PHY_ADR2_SW_WRADDR_SHIFT_0, PHY_ADR1_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[34] = 547;      /* PHY_ADR3_SW_WRADDR_SHIFT_0, PHY_ADR2_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[35] = 548;      /* PHY_ADR4_SW_WRADDR_SHIFT_0, PHY_ADR3_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[36] = 549;      /* PHY_ADR5_SW_WRADDR_SHIFT_0, PHY_ADR4_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[37] = 550;      /* PHY_ADR_SW_MASTER_MODE_0, PHY_ADR5_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[38] = 551;      /* PHY_ADR_MASTER_DELAY_WAIT_0, PHY_ADR_MASTER_DELAY_STEP_0, PHY_ADR_MASTER_DELAY_START_0 */
	phylpadrregnumretention[39] = 552;      /* PHY_ADR_SW_CALVL_DVW_MIN_EN_0, PHY_ADR_SW_CALVL_DVW_MIN_0, PHY_ADR_MASTER_DELAY_HALF_MEASURE_0 */
	phylpadrregnumretention[40] = 553;      /* PHY_ADR_CALVL_DLY_STEP_0 */
	phylpadrregnumretention[41] = 554;      /* PHY_ADR_MEAS_DLY_STEP_ENABLE_0, PHY_ADR_CALVL_CAPTURE_CNT_0 */
}

static void populatephyasregistersfn2(void)
{
	phylpadrregnumretention[42] = 772;      /* PHY_ADR_TSEL_ENABLE_0, PHY_ADR_SLAVE_LOOP_CNT_UPDATE_0 */
	phylpadrregnumretention[43] = 773;      /* PHY_ADR_PWR_RDC_DISABLE_0, PHY_ADR_PRBS_PATTERN_MASK_0, PHY_ADR_PRBS_PATTERN_START_0 */
	phylpadrregnumretention[44] = 774;      /* PHY_ADR_IE_MODE_0, PHY_ADR_TYPE_0, PHY_ADR_SLV_DLY_CTRL_GATE_DISABLE_0 */
	phylpadrregnumretention[45] = 779;      /* PHY_ADR_CALVL_COARSE_DLY_0, PHY_ADR_CALVL_START_0 */
	phylpadrregnumretention[46] = 780;      /* PHY_ADR_CALVL_QTR_0 */
	phylpadrregnumretention[47] = 781;      /* PHY_ADR_CALVL_SWIZZLE0_0 */
	phylpadrregnumretention[48] = 782;      /* PHY_ADR_CALVL_RANK_CTRL_0, PHY_ADR_CALVL_SWIZZLE1_0 */
	phylpadrregnumretention[49] = 783;      /* PHY_ADR_CALVL_PERIODIC_START_OFFSET_0, PHY_ADR_CALVL_RESP_WAIT_CNT_0, PHY_ADR_CALVL_NUM_PATTERNS_0 */
	phylpadrregnumretention[50] = 788;      /* PHY_ADR_CALVL_FG_0_0 */
	phylpadrregnumretention[51] = 789;      /* PHY_ADR_CALVL_BG_0_0 */
	phylpadrregnumretention[52] = 790;      /* PHY_ADR_CALVL_FG_1_0 */
	phylpadrregnumretention[53] = 791;      /* PHY_ADR_CALVL_BG_1_0 */
	phylpadrregnumretention[54] = 792;      /* PHY_ADR_CALVL_FG_2_0 */
	phylpadrregnumretention[55] = 793;      /* PHY_ADR_CALVL_BG_2_0 */
	phylpadrregnumretention[56] = 794;      /* PHY_ADR_CALVL_FG_3_0 */
	phylpadrregnumretention[57] = 795;      /* PHY_ADR_CALVL_BG_3_0 */
	phylpadrregnumretention[58] = 796;      /* PHY_ADR_ADDR_SEL_0 */
	phylpadrregnumretention[59] = 797;      /* PHY_ADR_SEG_MASK_0, PHY_ADR_BIT_MASK_0, PHY_ADR_LP4_BOOT_SLV_DELAY_0 */
	phylpadrregnumretention[60] = 798;      /* PHY_ADR_SW_TXIO_CTRL_0, PHY_ADR_STATIC_TOG_DISABLE_0, PHY_ADR_CSLVL_TRAIN_MASK_0, PHY_ADR_CALVL_TRAIN_MASK_0 */
	phylpadrregnumretention[61] = 799;      /* PHY_ADR_SW_TXPWR_CTRL_0 */

	phylpadrregnumretention[62] = 800;      /* PHY_PAD_ADR_RX_PCLK_CLK_SEL_0, PHY_PAD_ADR_IO_CFG_0, PHY_ADR_TSEL_SELECT_0 */
	phylpadrregnumretention[63] = 801;      /* PHY_ADR1_SW_WRADDR_SHIFT_0, PHY_ADR0_CLK_WR_SLAVE_DELAY_0, PHY_ADR0_SW_WRADDR_SHIFT_0 */
	phylpadrregnumretention[64] = 802;      /* PHY_ADR2_SW_WRADDR_SHIFT_0, PHY_ADR1_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[65] = 803;      /* PHY_ADR3_SW_WRADDR_SHIFT_0, PHY_ADR2_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[66] = 804;      /* PHY_ADR4_SW_WRADDR_SHIFT_0, PHY_ADR3_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[67] = 805;      /* PHY_ADR5_SW_WRADDR_SHIFT_0, PHY_ADR4_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[68] = 806;      /* PHY_ADR_SW_MASTER_MODE_0, PHY_ADR5_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[69] = 807;      /* PHY_ADR_MASTER_DELAY_WAIT_0, PHY_ADR_MASTER_DELAY_STEP_0, PHY_ADR_MASTER_DELAY_START_0 */
	phylpadrregnumretention[70] = 808;      /* PHY_ADR_SW_CALVL_DVW_MIN_EN_0, PHY_ADR_SW_CALVL_DVW_MIN_0, PHY_ADR_MASTER_DELAY_HALF_MEASURE_0 */
	phylpadrregnumretention[71] = 809;      /* PHY_ADR_CALVL_DLY_STEP_0 */
	phylpadrregnumretention[72] = 810;      /* PHY_ADR_MEAS_DLY_STEP_ENABLE_0, PHY_ADR_CALVL_CAPTURE_CNT_0 */

	phylpadrregnumretention[73] = 800;      /* PHY_PAD_ADR_RX_PCLK_CLK_SEL_0, PHY_PAD_ADR_IO_CFG_0, PHY_ADR_TSEL_SELECT_0 */
	phylpadrregnumretention[74] = 801;      /* PHY_ADR1_SW_WRADDR_SHIFT_0, PHY_ADR0_CLK_WR_SLAVE_DELAY_0, PHY_ADR0_SW_WRADDR_SHIFT_0 */
	phylpadrregnumretention[75] = 802;      /* PHY_ADR2_SW_WRADDR_SHIFT_0, PHY_ADR1_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[76] = 803;      /* PHY_ADR3_SW_WRADDR_SHIFT_0, PHY_ADR2_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[77] = 804;      /* PHY_ADR4_SW_WRADDR_SHIFT_0, PHY_ADR3_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[78] = 805;      /* PHY_ADR5_SW_WRADDR_SHIFT_0, PHY_ADR4_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[79] = 806;      /* PHY_ADR_SW_MASTER_MODE_0, PHY_ADR5_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[80] = 807;      /* PHY_ADR_MASTER_DELAY_WAIT_0, PHY_ADR_MASTER_DELAY_STEP_0, PHY_ADR_MASTER_DELAY_START_0 */
	phylpadrregnumretention[81] = 808;      /* PHY_ADR_SW_CALVL_DVW_MIN_EN_0, PHY_ADR_SW_CALVL_DVW_MIN_0, PHY_ADR_MASTER_DELAY_HALF_MEASURE_0 */
	phylpadrregnumretention[82] = 809;      /* PHY_ADR_CALVL_DLY_STEP_0 */
	phylpadrregnumretention[83] = 810;      /* PHY_ADR_MEAS_DLY_STEP_ENABLE_0, PHY_ADR_CALVL_CAPTURE_CNT_0 */
}

static void populatephyasregisters(void)
{
	populatephyasregistersfn1();
	populatephyasregistersfn2();
	phylpadrregnumretention[84] = 1028;     /* PHY_ADR_TSEL_ENABLE_0, PHY_ADR_SLAVE_LOOP_CNT_UPDATE_0 */
	phylpadrregnumretention[85] = 1029;     /* PHY_ADR_PWR_RDC_DISABLE_0, PHY_ADR_PRBS_PATTERN_MASK_0, PHY_ADR_PRBS_PATTERN_START_0 */
	phylpadrregnumretention[86] = 1030;     /* PHY_ADR_IE_MODE_0, PHY_ADR_TYPE_0, PHY_ADR_SLV_DLY_CTRL_GATE_DISABLE_0 */
	phylpadrregnumretention[87] = 1035;     /* PHY_ADR_CALVL_COARSE_DLY_0, PHY_ADR_CALVL_START_0 */
	phylpadrregnumretention[88] = 1036;     /* PHY_ADR_CALVL_QTR_0 */
	phylpadrregnumretention[89] = 1037;     /* PHY_ADR_CALVL_SWIZZLE0_0 */
	phylpadrregnumretention[90] = 1038;     /* PHY_ADR_CALVL_RANK_CTRL_0, PHY_ADR_CALVL_SWIZZLE1_0 */
	phylpadrregnumretention[91] = 1039;     /* PHY_ADR_CALVL_PERIODIC_START_OFFSET_0, PHY_ADR_CALVL_RESP_WAIT_CNT_0, PHY_ADR_CALVL_NUM_PATTERNS_0 */
	phylpadrregnumretention[92] = 1044;     /* PHY_ADR_CALVL_FG_0_0 */
	phylpadrregnumretention[93] = 1045;     /* PHY_ADR_CALVL_BG_0_0 */
	phylpadrregnumretention[94] = 1046;     /* PHY_ADR_CALVL_FG_1_0 */
	phylpadrregnumretention[95] = 1047;     /* PHY_ADR_CALVL_BG_1_0 */
	phylpadrregnumretention[96] = 1048;     /* PHY_ADR_CALVL_FG_2_0 */
	phylpadrregnumretention[97] = 1049;     /* PHY_ADR_CALVL_BG_2_0 */
	phylpadrregnumretention[98] = 1050;     /* PHY_ADR_CALVL_FG_3_0 */
	phylpadrregnumretention[99] = 1051;     /* PHY_ADR_CALVL_BG_3_0 */
	phylpadrregnumretention[100] = 1052;    /* PHY_ADR_ADDR_SEL_0 */
	phylpadrregnumretention[101] = 1053;    /* PHY_ADR_SEG_MASK_0, PHY_ADR_BIT_MASK_0, PHY_ADR_LP4_BOOT_SLV_DELAY_0 */
	phylpadrregnumretention[102] = 1054;    /* PHY_ADR_SW_TXIO_CTRL_0, PHY_ADR_STATIC_TOG_DISABLE_0, PHY_ADR_CSLVL_TRAIN_MASK_0, PHY_ADR_CALVL_TRAIN_MASK_0 */
	phylpadrregnumretention[103] = 1055;    /* PHY_ADR_SW_TXPWR_CTRL_0 */

	phylpadrregnumretention[104] = 1056;    /* PHY_PAD_ADR_RX_PCLK_CLK_SEL_0, PHY_PAD_ADR_IO_CFG_0, PHY_ADR_TSEL_SELECT_0 */
	phylpadrregnumretention[105] = 1057;    /* PHY_ADR1_SW_WRADDR_SHIFT_0, PHY_ADR0_CLK_WR_SLAVE_DELAY_0, PHY_ADR0_SW_WRADDR_SHIFT_0 */
	phylpadrregnumretention[106] = 1058;    /* PHY_ADR2_SW_WRADDR_SHIFT_0, PHY_ADR1_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[107] = 1059;    /* PHY_ADR3_SW_WRADDR_SHIFT_0, PHY_ADR2_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[108] = 1060;    /* PHY_ADR4_SW_WRADDR_SHIFT_0, PHY_ADR3_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[109] = 1061;    /* PHY_ADR5_SW_WRADDR_SHIFT_0, PHY_ADR4_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[110] = 1062;    /* PHY_ADR_SW_MASTER_MODE_0, PHY_ADR5_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[111] = 1063;    /* PHY_ADR_MASTER_DELAY_WAIT_0, PHY_ADR_MASTER_DELAY_STEP_0, PHY_ADR_MASTER_DELAY_START_0 */
	phylpadrregnumretention[112] = 1064;    /* PHY_ADR_SW_CALVL_DVW_MIN_EN_0, PHY_ADR_SW_CALVL_DVW_MIN_0, PHY_ADR_MASTER_DELAY_HALF_MEASURE_0 */
	phylpadrregnumretention[113] = 1065;    /* PHY_ADR_CALVL_DLY_STEP_0 */
	phylpadrregnumretention[114] = 1066;    /* PHY_ADR_MEAS_DLY_STEP_ENABLE_0, PHY_ADR_CALVL_CAPTURE_CNT_0 */

	phylpadrregnumretention[115] = 1056;    /* PHY_PAD_ADR_RX_PCLK_CLK_SEL_0, PHY_PAD_ADR_IO_CFG_0, PHY_ADR_TSEL_SELECT_0 */
	phylpadrregnumretention[116] = 1057;    /* PHY_ADR1_SW_WRADDR_SHIFT_0, PHY_ADR0_CLK_WR_SLAVE_DELAY_0, PHY_ADR0_SW_WRADDR_SHIFT_0 */
	phylpadrregnumretention[117] = 1058;    /* PHY_ADR2_SW_WRADDR_SHIFT_0, PHY_ADR1_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[118] = 1059;    /* PHY_ADR3_SW_WRADDR_SHIFT_0, PHY_ADR2_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[119] = 1060;    /* PHY_ADR4_SW_WRADDR_SHIFT_0, PHY_ADR3_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[120] = 1061;    /* PHY_ADR5_SW_WRADDR_SHIFT_0, PHY_ADR4_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[121] = 1062;    /* PHY_ADR_SW_MASTER_MODE_0, PHY_ADR5_CLK_WR_SLAVE_DELAY_0 */
	phylpadrregnumretention[122] = 1063;    /* PHY_ADR_MASTER_DELAY_WAIT_0, PHY_ADR_MASTER_DELAY_STEP_0, PHY_ADR_MASTER_DELAY_START_0 */
	phylpadrregnumretention[123] = 1064;    /* PHY_ADR_SW_CALVL_DVW_MIN_EN_0, PHY_ADR_SW_CALVL_DVW_MIN_0, PHY_ADR_MASTER_DELAY_HALF_MEASURE_0 */
	phylpadrregnumretention[124] = 1065;    /* PHY_ADR_CALVL_DLY_STEP_0 */
	phylpadrregnumretention[125] = 1066;    /* PHY_ADR_MEAS_DLY_STEP_ENABLE_0, PHY_ADR_CALVL_CAPTURE_CNT_0 */
}

static void populatephytopregistersfn1(void)
{
	phylptopregnumretention[0] = 1280;      /* PHY_FREQ_SEL */
	phylptopregnumretention[1] = 1281;      /* PHY_SW_GRP0_SHIFT_0, PHY_FREQ_SEL_INDEX, PHY_FREQ_SEL_MULTICAST_EN, PHY_FREQ_SEL_FROM_REGIF */
	phylptopregnumretention[2] = 1282;      /* PHY_SW_GRP0_SHIFT_1, PHY_SW_GRP3_SHIFT_0, PHY_SW_GRP2_SHIFT_0, PHY_SW_GRP1_SHIFT_0 */
	phylptopregnumretention[3] = 1283;      /* PHY_SW_GRP0_SHIFT_2, PHY_SW_GRP3_SHIFT_1, PHY_SW_GRP2_SHIFT_1, PHY_SW_GRP1_SHIFT_1 */
	phylptopregnumretention[4] = 1284;      /* PHY_SW_GRP0_SHIFT_3, PHY_SW_GRP3_SHIFT_2, PHY_SW_GRP1_SHIFT_2, PHY_SW_GRP1_SHIFT_2 */
	phylptopregnumretention[5] = 1285;      /* PHY_SW_GRP3_SHIFT_3, PHY_SW_GRP2_SHIFT_3, PHY_SW_GRP1_SHIFT_3 */
	phylptopregnumretention[6] = 1286;      /* PHY_GRP_BYPASS_OVERRIDE, PHY_SW_GRP_BYPASS_SHIFT, PHY_GRP_BYPASS_SLAVE_DELAY */
	phylptopregnumretention[7] = 1287;      /* PHY_CSLVL_START, PHY_MANUAL_UPDATE_PHYUPD_ENABLE */
	phylptopregnumretention[8] = 1288;      /* PHY_CSLVL_COARSE_DLY */
	phylptopregnumretention[9] = 1289;      /* SC_PHY_CSLVL_ERROR_CLR */
	phylptopregnumretention[10] = 1293;     /* PHY_LP4_BOOT_DISABLE, PHY_CSLVL_PERIODIC_START_OFFSET, PHY_CSLVL_ENABLE */
	phylptopregnumretention[11] = 1294;     /* PHY_CSLVL_QTR, PHY_CSLVL_CS_MAP */
	phylptopregnumretention[12] = 1295;     /* PHY_ADRCTL_SLAVE_LOOP_CNT_UPDATE, PHY_CSLVL_COARSE_CAPTURE_CNT, PHY_CSLVL_COARSE_CHK */
	phylptopregnumretention[13] = 1296;     /* PHY_LP4_ACTIVE, PHY_ADRCTL_LPDDR, PHY_DFI_PHYUPD_TYPE */
	phylptopregnumretention[14] = 1297;     /* PHY_SW_TXIO_CTRL_0, PHY_CONTINUOUS_CLK_CAL_UPDATE, SC_PHY_UPDATE_CLK_CAL_VALUES, PHY_LPDDR3_CS*/
	phylptopregnumretention[15] = 1298;     /* PHY_MEMCLK_SW_TXIO_CTRL, PHY_SW_TXIO_CTRL_3, PHY_SW_TXIO_CTRL_2, PHY_SW_TXIO_CTRL_1 */
	phylptopregnumretention[16] = 1299;     /* PHY_ADRCTL_SW_TXPWR_CTRL_3, PHY_ADRCTL_SW_TXPWR_CTRL_2, PHY_ADRCTL_SW_TXPWR_CTRL_1, PHY_ADRCTL_SW_TXPWR_CTRL_0 */
	phylptopregnumretention[17] = 1300;     /* PHY_BYTE_DISABLE_STATIC_TOG_DISABLE, PHY_TOP_STATIC_TOG_DISABLE, PHY_MEMCLK_SW_TXPWR_CTRL */
	phylptopregnumretention[18] = 1301;     /* PHY_MEMCLK_STATIC_TOG_DISABLE, PHY_ADRCTL_STATIC_TOG_DISABLE, PHY_STATIC_TOG_CONTROL */
	phylptopregnumretention[19] = 1302;     /* PHY_LP4_BOOT_PLL_BYPASS */
	phylptopregnumretention[20] = 1304;     /* PHY_PLL_WAIT */
	phylptopregnumretention[21] = 1305;     /* PHY_SW_PLL_BYPASS */
	phylptopregnumretention[22] = 1306;     /* PHY_SET_DFI_INPUT_3, PHY_SET_DFI_INPUT_2, PHY_SET_DFI_INPUT_1, PHY_SET_DFI_INPUT_0 */
	phylptopregnumretention[23] = 1307;     /* PHY_CS_ACS_ALLOCATION_BIT3_0, PHY_CS_ACS_ALLOCATION_BIT2_0, PHY_CS_ACS_ALLOCATION_BIT1_0, PHY_CS_ACS_ALLOCATION_BIT0_0 */
	phylptopregnumretention[24] = 1308;     /* PHY_CS_ACS_ALLOCATION_BIT3_1, PHY_CS_ACS_ALLOCATION_BIT2_1,PHY_CS_ACS_ALLOCATION_BIT1_1, PHY_CS_ACS_ALLOCATION_BIT0_1 */
	phylptopregnumretention[25] = 1309;     /* PHY_CS_ACS_ALLOCATION_BIT3_2, PHY_CS_ACS_ALLOCATION_BIT3_2, PHY_CS_ACS_ALLOCATION_BIT3_2, PHY_CS_ACS_ALLOCATION_BIT3_2 */
	phylptopregnumretention[26] = 1310;     /* PHY_CS_ACS_ALLOCATION_BIT3_3, PHY_CS_ACS_ALLOCATION_BIT3_3, PHY_CS_ACS_ALLOCATION_BIT3_3, PHY_CS_ACS_ALLOCATION_BIT3_3 */
	phylptopregnumretention[27] = 1311;     /* PHY_PLL_CTRL_OVERRIDE, PHY_LP4_BOOT_PLL_CTRL */
	phylptopregnumretention[28] = 1312;     /* PHY_PLL_SPO_CAL_CTRL, PHY_USE_PLL_DSKEWCALLOCK */
	phylptopregnumretention[29] = 1315;     /* PHY_LP4_BOOT_PLL_DESKEWCALIN_0, PHY_PLL_DESKEWCALIN_0 */
	phylptopregnumretention[30] = 1318;     /* PHY_LP4_BOOT_PLL_DESKEWCALIN_1, PHY_PLL_DESKEWCALIN_1 */
	phylptopregnumretention[31] = 1319;     /* PHY_LP_WAKEUP, PHY_TCKSRE_WAIT, PHY_LP4_BOOT_LOW_FREQ_SEL, PHY_PLL_REFOUT_SEL */
	phylptopregnumretention[32] = 1320;     /* PHY_LP_CTRLUPD_CNTR_CFG, PHY_LS_IDLE_EN */
	phylptopregnumretention[33] = 1321;     /* PHY_TDFI_PHY_WRDELAY, PHY_DS_EXIT_CTRL */
	phylptopregnumretention[34] = 1322;     /* PHY_PAD_FDBK_TERM */
	phylptopregnumretention[35] = 1323;     /* PHY_PAD_DATA_TERM */
	phylptopregnumretention[36] = 1324;     /* PHY_PAD_DQS_TERM */
	phylptopregnumretention[37] = 1325;     /* PHY_PAD_ADDR_TERM */
	phylptopregnumretention[38] = 1326;     /* PHY_PAD_CLK_TERM */
	phylptopregnumretention[39] = 1327;     /* PHY_PAD_ERR_TERM */
	phylptopregnumretention[40] = 1328;     /* PHY_PAD_CKE_TERM */
	phylptopregnumretention[41] = 1329;     /* PHY_PAD_RST_TERM */
	phylptopregnumretention[42] = 1330;     /* PHY_PAD_CS_TERM */
	phylptopregnumretention[43] = 1331;     /* PHY_PAD_ODT_TERM */
	phylptopregnumretention[44] = 1332;     /* PHY_ADRCTL_LP3_RX_CAL, PHY_ADRCTL_RX_CAL */
	phylptopregnumretention[45] = 1333;     /* PHY_CAL_START_0, PHY_CAL_CLEAR_0, PHY_CAL_MODE_0 */
	phylptopregnumretention[46] = 1334;     /* PHY_CAL_INTERVAL_COUNT_0 */
	phylptopregnumretention[47] = 1335;     /* PHY_LP4_BOOT_CAL_CLK_SELECT_0, PHY_CAL_SAMPLE_WAIT_0 */
	phylptopregnumretention[48] = 1341;     /* PHY_CAL_CPTR_CNT_0 */
	phylptopregnumretention[49] = 1342;     /* PHY_CAL_DBG_CFG_0, PHY_CAL_RCV_FINE_ADJ_0, PHY_CAL_PD_FINE_ADJ_0, PHY_CAL_PU_FINE_ADJ_0 */
}

static void populatephytopregistersfn2(void)
{
	phylptopregnumretention[50] = 1343;     /* SC_PHY_PAD_DBG_CONT_0 */
	phylptopregnumretention[51] = 1345;     /* PHY_CAL_SLOPE_ADJ_0, PHY_ADRCTL_PVT_MAP_0 */
	phylptopregnumretention[52] = 1346;     /* PHY_CAL_SLOPE_ADJ_PASS2_0 */
	phylptopregnumretention[53] = 1347;     /* PHY_CAL_TWO_PASS_CFG_0 */
	phylptopregnumretention[54] = 1348;     /* PHY_CAL_RANGE_PASS1_PU_MAX_DELTA_0, PHY_CAL_SW_CAL_CFG_0 */
	phylptopregnumretention[55] = 1349;     /* PHY_CAL_RANGE_PASS2_PD_MAX_DELTA_0, PHY_CAL_RANGE_PASS2_PU_MAX_DELTA_0, PHY_CAL_RANGE_PASS1_RX_MAX_DELTA_0, PHY_CAL_RANGE_PASS1_PD_MAX_DELTA_0 */
	phylptopregnumretention[56] = 1350;     /* PHY_CAL_RANGE_PASS1_RX_MIN_DELTA_0, PHY_CAL_RANGE_PASS1_PD_MIN_DELTA_0, PHY_CAL_RANGE_PASS1_PU_MIN_DELTA_0, PHY_CAL_RANGE_PASS2_RX_MAX_DELTA_0 */
	phylptopregnumretention[57] = 1351;     /* PHY_CAL_RANGE_PASS2_RX_MIN_DELTA_0, PHY_CAL_RANGE_PASS2_PD_MIN_DELTA_0, PHY_CAL_RANGE_PASS2_PU_MIN_DELTA_0 */
	phylptopregnumretention[58] = 1352;     /* PHY_AC_LPBK_ERR_CLEAR, PHY_ADRCTL_MANUAL_UPDATE, PHY_PAD_ATB_CTRL */
	phylptopregnumretention[59] = 1357;     /* PHY_TOP_PWR_RDC_DISABLE, PHY_AC_PWR_RDC_DISABLE */
	phylptopregnumretention[60] = 1358;     /* PHY_AC_SLV_DLY_CTRL_GATE_DISABLE */
	phylptopregnumretention[61] = 1359;     /* PHY_DATA_BYTE_ORDER_SEL */
	phylptopregnumretention[62] = 1360;     /* PHY_ADRCTL_MSTR_DLY_ENC_SEL_0, PHY_ADR_DISABLE, PHY_CALVL_DEVICE_MAP, PHY_DATA_BYTE_ORDER_SEL_HIGH */
	phylptopregnumretention[63] = 1361;     /* PHY_ADRCTL_MSTR_DLY_ENC_SEL_3, PHY_ADRCTL_MSTR_DLY_ENC_SEL_2, PHY_ADRCTL_MSTR_DLY_ENC_SEL_1 */
	phylptopregnumretention[64] = 1362;     /* PHY_DDL_AC_ENABLE */
	phylptopregnumretention[65] = 1363;     /* PHY_DDL_AC_MODE */
	phylptopregnumretention[66] = 1364;     /* PHY_DDL_TRACK_UPD_THRESHOLD_AC, PHY_INIT_UPDATE_CONFIG, PHY_DDL_AC_MASK */
	phylptopregnumretention[67] = 1365;     /* PHY_ERR_STATUS, PHY_ERR_MASK_EN, PHY_CA_PARITY_ERR_PULSE_MIN */
	phylptopregnumretention[68] = 1366;     /* PHY_DS0_DQS_ERR_COUNTER */
	phylptopregnumretention[69] = 1367;     /* PHY_DS1_DQS_ERR_COUNTER */
	phylptopregnumretention[70] = 1368;     /* PHY_DLL_RST_EN */
	phylptopregnumretention[71] = 1369;     /* PHY_ERR_IE, PHY_UPDATE_MASK */

	phylptopregnumretention[72] = 1371;     /* PHY_PAD_CAL_IO_CFG_0 */
	phylptopregnumretention[73] = 1372;     /* PHY_PAD_ACS_RX_PCLK_CLK_SEL, PHY_PAD_ACS_IO_CFG */
	phylptopregnumretention[74] = 1373;     /* PHY_PLL_BYPASS */
	phylptopregnumretention[75] = 1374;     /* PHY_LOW_FREQ_SEL, PHY_PLL_CTRL */
	phylptopregnumretention[76] = 1375;     /* PHY_CSLVL_DLY_STEP, PHY_CSLVL_CAPTURE_CNT, PHY_PAD_VREF_CTRL_AC */
	phylptopregnumretention[77] = 1376;     /* PHY_LVL_MEAS_DLY_STEP_ENABLE, PHY_SW_CSLVL_DVW_MIN_EN, PHY_SW_CSLVL_DVW_MIN */
	phylptopregnumretention[78] = 1377;     /* PHY_GRP1_SLAVE_DELAY_0, PHY_GRP0_SLAVE_DELAY_0 */
	phylptopregnumretention[79] = 1378;     /* PHY_GRP3_SLAVE_DELAY_0, PHY_GRP2_SLAVE_DELAY_0 */
	phylptopregnumretention[80] = 1379;     /* PHY_GRP1_SLAVE_DELAY_1, PHY_GRP0_SLAVE_DELAY_1 */
	phylptopregnumretention[81] = 1380;     /* PHY_GRP3_SLAVE_DELAY_1, PHY_GRP2_SLAVE_DELAY_1 */
	phylptopregnumretention[82] = 1381;     /* PHY_GRP1_SLAVE_DELAY_2, PHY_GRP0_SLAVE_DELAY_2 */
	phylptopregnumretention[83] = 1382;     /* PHY_GRP3_SLAVE_DELAY_2, PHY_GRP2_SLAVE_DELAY_2 */
	phylptopregnumretention[84] = 1383;     /* PHY_GRP0_SLAVE_DELAY_3 */
	phylptopregnumretention[85] = 1384;     /* PHY_GRP1_SLAVE_DELAY_3 */
	phylptopregnumretention[86] = 1385;     /* PHY_GRP2_SLAVE_DELAY_3 */
	phylptopregnumretention[87] = 1386;     /* PHY_GRP3_SLAVE_DELAY_3 */
	phylptopregnumretention[88] = 1387;     /* PHY_PAD_FDBK_DRIVE */
	phylptopregnumretention[89] = 1388;     /* PHY_PAD_FDBK_DRIVE2 */
	phylptopregnumretention[90] = 1389;     /* PHY_PAD_DATA_DRIVE */
	phylptopregnumretention[91] = 1390;     /* PHY_PAD_DQS_DRIVE */
	phylptopregnumretention[92] = 1391;     /* PHY_PAD_ADDR_DRIVE */
	phylptopregnumretention[93] = 1392;     /* PHY_PAD_ADDR_DRIVE2 */
	phylptopregnumretention[94] = 1393;     /* PHY_PAD_CLK_DRIVE */
	phylptopregnumretention[95] = 1394;     /* PHY_PAD_CLK_DRIVE2 */
	phylptopregnumretention[96] = 1395;     /* PHY_PAD_ERR_DRIVE */
	phylptopregnumretention[97] = 1396;     /* PHY_PAD_ERR_DRIVE2 */
	phylptopregnumretention[98] = 1397;     /* PHY_PAD_CKE_DRIVE */
	phylptopregnumretention[99] = 1398;     /* PHY_PAD_CKE_DRIVE2 */
}

static void populatephytopregisters(void)
{
	populatephytopregistersfn1();
	populatephytopregistersfn2();
	phylptopregnumretention[100] = 1399;    /* PHY_PAD_RST_DRIVE */
	phylptopregnumretention[101] = 1400;    /* PHY_PAD_RST_DRIVE2 */
	phylptopregnumretention[102] = 1401;    /* PHY_PAD_CS_DRIVE */
	phylptopregnumretention[103] = 1402;    /* PHY_PAD_CS_DRIVE2 */
	phylptopregnumretention[104] = 1403;    /* PHY_PAD_ODT_DRIVE */
	phylptopregnumretention[105] = 1404;    /* PHY_PAD_ODT_DRIVE2 */
	phylptopregnumretention[106] = 1405;    /* PHY_CAL_SETTLING_PRD_0, PHY_CAL_VREF_SWITCH_TIMER_0, PHY_CAL_CLK_SELECT_0 */

	phylptopregnumretention[107] = 1371;    /* PHY_PAD_CAL_IO_CFG_0 */
	phylptopregnumretention[108] = 1372;    /* PHY_PAD_ACS_RX_PCLK_CLK_SEL, PHY_PAD_ACS_IO_CFG */
	phylptopregnumretention[109] = 1373;    /* PHY_PLL_BYPASS */
	phylptopregnumretention[110] = 1374;    /* PHY_LOW_FREQ_SEL, PHY_PLL_CTRL */
	phylptopregnumretention[111] = 1375;    /* PHY_CSLVL_DLY_STEP, PHY_CSLVL_CAPTURE_CNT, PHY_PAD_VREF_CTRL_AC */
	phylptopregnumretention[112] = 1376;    /* PHY_LVL_MEAS_DLY_STEP_ENABLE, PHY_SW_CSLVL_DVW_MIN_EN, PHY_SW_CSLVL_DVW_MIN */
	phylptopregnumretention[113] = 1377;    /* PHY_GRP1_SLAVE_DELAY_0, PHY_GRP0_SLAVE_DELAY_0 */
	phylptopregnumretention[114] = 1378;    /* PHY_GRP3_SLAVE_DELAY_0, PHY_GRP2_SLAVE_DELAY_0 */
	phylptopregnumretention[115] = 1379;    /* PHY_GRP1_SLAVE_DELAY_1, PHY_GRP0_SLAVE_DELAY_1 */
	phylptopregnumretention[116] = 1380;    /* PHY_GRP3_SLAVE_DELAY_1, PHY_GRP2_SLAVE_DELAY_1 */
	phylptopregnumretention[117] = 1381;    /* PHY_GRP1_SLAVE_DELAY_2, PHY_GRP0_SLAVE_DELAY_2 */
	phylptopregnumretention[118] = 1382;    /* PHY_GRP3_SLAVE_DELAY_2, PHY_GRP2_SLAVE_DELAY_2 */
	phylptopregnumretention[119] = 1383;    /* PHY_GRP0_SLAVE_DELAY_3 */
	phylptopregnumretention[120] = 1384;    /* PHY_GRP1_SLAVE_DELAY_3 */
	phylptopregnumretention[121] = 1385;    /* PHY_GRP2_SLAVE_DELAY_3 */
	phylptopregnumretention[122] = 1386;    /* PHY_GRP3_SLAVE_DELAY_3 */
	phylptopregnumretention[123] = 1387;    /* PHY_PAD_FDBK_DRIVE */
	phylptopregnumretention[124] = 1388;    /* PHY_PAD_FDBK_DRIVE2 */
	phylptopregnumretention[125] = 1389;    /* PHY_PAD_DATA_DRIVE */
	phylptopregnumretention[126] = 1390;    /* PHY_PAD_DQS_DRIVE */
	phylptopregnumretention[127] = 1391;    /* PHY_PAD_ADDR_DRIVE */
	phylptopregnumretention[128] = 1392;    /* PHY_PAD_ADDR_DRIVE2 */
	phylptopregnumretention[129] = 1393;    /* PHY_PAD_CLK_DRIVE */
	phylptopregnumretention[130] = 1394;    /* PHY_PAD_CLK_DRIVE2 */
	phylptopregnumretention[131] = 1395;    /* PHY_PAD_ERR_DRIVE */
	phylptopregnumretention[132] = 1396;    /* PHY_PAD_ERR_DRIVE2 */
	phylptopregnumretention[133] = 1397;    /* PHY_PAD_CKE_DRIVE */
	phylptopregnumretention[134] = 1398;    /* PHY_PAD_CKE_DRIVE2 */
	phylptopregnumretention[135] = 1399;    /* PHY_PAD_RST_DRIVE */
	phylptopregnumretention[136] = 1400;    /* PHY_PAD_RST_DRIVE2 */
	phylptopregnumretention[137] = 1401;    /* PHY_PAD_CS_DRIVE */
	phylptopregnumretention[138] = 1402;    /* PHY_PAD_CS_DRIVE2 */
	phylptopregnumretention[139] = 1403;    /* PHY_PAD_ODT_DRIVE */
	phylptopregnumretention[140] = 1404;    /* PHY_PAD_ODT_DRIVE2 */
}

static void populatepiregistersfn2(void)
{
	pilpregnumretention[40] = 193;  /* PI_RDLVL_DFE_EN_F2, PI_RDLVL_MULTI_EN_F2, PI_RDLVL_RXCAL_EN_F2 */
	pilpregnumretention[41] = 194;  /* PI_RDLAT_ADJ_F2 */
	pilpregnumretention[42] = 216;  /* PI_TDFI_INIT_START_F2 */
	pilpregnumretention[43] = 217;  /* PI_TDFI_INIT_COMPLETE_F2 */
	pilpregnumretention[44] = 220;  /* PI_TFC_F2 */
	pilpregnumretention[45] = 231;  /* PI_RD_DBI_LEVEL_EN_F2 */
	pilpregnumretention[46] = 241;  /* PI_TRP_F2, PI_TRTP_F2 */
	pilpregnumretention[47] = 242;  /* PI_TCCD_L_F2, PI_TRCD_F2, PI_TWR_F2, PI_TWTR_F2 */
	pilpregnumretention[48] = 243;  /* PI_TRAS_MAX_F2 */
	pilpregnumretention[49] = 244;  /* PI_TCCDMW_F2, PI_TDQSCK_MAX_F2, PI_TRAS_MIN_F2 */
	pilpregnumretention[50] = 245;  /* PI_TMOD_F2, PI_TMRD_F2, PI_TMRW_F2 */
	pilpregnumretention[51] = 282;  /* PI_MEMDATA_RATIO_0, PI_MEMDATA_RATIO_1, PI_PREAMBLE_SUPPORT_F2 */
	pilpregnumretention[52] = 283;  /* PI_ODT_RD_MAP_CS0, PI_ODT_RD_MAP_CS1 */
	pilpregnumretention[53] = 286;  /* PI_MR13_DATA_0 */
	pilpregnumretention[54] = 289;  /* PI_MR13_DATA_1 */
	pilpregnumretention[55] = 291;  /* PI_CKE_MUX_0, PI_CKE_MUX_1, PI_CS_MUX_0 */
	pilpregnumretention[56] = 292;  /* PI_CS_MUX_1, PI_ODT_MUX_0, PI_ODT_MUX_1, PI_RESET_N_MUX_0 */
	pilpregnumretention[57] = 293;  /* PI_RESET_N_MUX_1 */
}

static void populatepiregistersfn1(void)
{
	pilpregnumretention[0] = 0;     /* PI_DRAM_CLASS, PI_START */
	pilpregnumretention[1] = 12;    /* PI_FREQ_MAP */
	pilpregnumretention[2] = 13;    /* PI_CS_MAP */
	pilpregnumretention[3] = 14;    /* PI_CS_MASK, PI_SRX_LVL_TARGET_CS_EN, PI_TMRR */
	pilpregnumretention[4] = 15;    /* PI_TMPRR */
	pilpregnumretention[5] = 17;    /* PI_DATA_RETENTION */
	pilpregnumretention[6] = 23;    /* PI_SRE_PERIOD_EN */
	pilpregnumretention[7] = 30;    /* PI_ODT_VALUE, PI_TODTH_RD */
	pilpregnumretention[8] = 31;    /* PI_ADDRESS_MIRRORING */
	pilpregnumretention[9] = 34;    /* PI_RDLVL_CS */
	pilpregnumretention[10] = 35;   /* PI_RDLVL_PAT_0 */
	pilpregnumretention[11] = 36;   /* PI_RDLVL_PAT_1 */
	pilpregnumretention[12] = 37;   /* PI_RDLVL_PAT_2 */
	pilpregnumretention[13] = 38;   /* PI_RDLVL_PAT_3 */
	pilpregnumretention[14] = 39;   /* PI_RDLVL_PAT_4 */
	pilpregnumretention[15] = 40;   /* PI_RDLVL_PAT_5 */
	pilpregnumretention[16] = 41;   /* PI_RDLVL_PAT_6 */
	pilpregnumretention[17] = 42;   /* PI_RDLVL_PAT_7 */
	pilpregnumretention[18] = 43;   /* PI_RDLVL_GATE_ON_SREF_EXIT, PI_RDLVL_ON_SREF_EXIT, PI_RDLVL_SEQ_EN */
	pilpregnumretention[19] = 44;   /* PI_RDLVL_ROTATE */
	pilpregnumretention[20] = 45;   /* PI_RDLVL_CS_MAP, PI_RDLVL_GATE_CS_MAP, PI_RDLVL_GATE_ROTATE */
	pilpregnumretention[21] = 46;   /* PI_TDFI_RDLVL_RR */
	pilpregnumretention[22] = 47;   /* PI_TDFI_RDLVL_RESP */
	pilpregnumretention[23] = 48;   /* PI_RDLVL_RESP_MASK, PI_TDFI_RDLVL_EN */
	pilpregnumretention[24] = 49;   /* PI_TDFI_RDLVL_MAX */
	pilpregnumretention[25] = 51;   /* PI_RDLVL_PATTERN_NUM, PI_RDLVL_PATTERN_START */
	pilpregnumretention[26] = 52;   /* PI_RDLVL_GATE_STROBE_NUM, PI_RDLVL_STROBE_NUM, PI_RD_PREAMBLE_TRAINING_EN, PI_REG_DIMM_ENABLE */
	pilpregnumretention[27] = 74;   /* PI_NO_MEMORY_DM */
	pilpregnumretention[28] = 77;   /* PI_TCCD */
	pilpregnumretention[29] = 91;   /* PI_BSTLEN, PI_CMD_SWAP_EN */
	pilpregnumretention[30] = 92;   /* PI_ACT_N_MUX, PI_BG_MUX_0, PI_BG_MUX_1, PI_PARITY_IN_MUX */
	pilpregnumretention[31] = 93;   /* PI_BANK_MUX_0, PI_CAS_N_MUX, PI_RAS_N_MUX, PI_WE_N_MUX */
	pilpregnumretention[32] = 94;   /* PI_BANK_MUX_1, PI_DATA_BYTE_SWAP_EN, PI_DATA_BYTE_SWAP_SLICE0, PI_DATA_BYTE_SWAP_SLICE1 */
	pilpregnumretention[33] = 96;   /* PI_TDFI_PARIN_LAT */
	pilpregnumretention[34] = 134;  /* PI_MC_PWRUP_SREFRESH_EXIT, PI_PWRUP_SREFRESH_EXIT */
	pilpregnumretention[35] = 153;  /* PI_FREQ_RETENTION_NUM */
	pilpregnumretention[36] = 173;  /* PI_ADDITIVE_LAT_F2 */
	pilpregnumretention[37] = 188;  /* PI_RD_TO_ODTH_F2 */
	pilpregnumretention[38] = 189;  /* PI_RDLVL_EN_F2, PI_RDLVL_GATE_EN_F2 */
	pilpregnumretention[39] = 192;  /* PI_RDLVL_PAT0_EN_F2 */
}

void populatelowpowerconfigarray(void)
{
	uint32_t aindex;

	for (aindex = 0; aindex < (uint32_t)LPDDR4_CTL_LP_REGS_SET; aindex++)
		ctllpregnumretention[aindex] = aindex;

	populatephydsregisters();
	populatephyasregisters();
	populatephytopregisters();

	populatepiregistersfn1();
	populatepiregistersfn2();
}

#endif /* LPDDR4_16BIT_H */
