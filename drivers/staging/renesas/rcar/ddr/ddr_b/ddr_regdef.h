/*
 * Copyright (c) 2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define _reg_PHY_DQ_DM_SWIZZLE0                            0x00000000U
#define _reg_PHY_DQ_DM_SWIZZLE1                            0x00000001U
#define _reg_PHY_CLK_WR_BYPASS_SLAVE_DELAY                 0x00000002U
#define _reg_PHY_RDDQS_GATE_BYPASS_SLAVE_DELAY             0x00000003U
#define _reg_PHY_BYPASS_TWO_CYC_PREAMBLE                   0x00000004U
#define _reg_PHY_CLK_BYPASS_OVERRIDE                       0x00000005U
#define _reg_PHY_SW_WRDQ0_SHIFT                            0x00000006U
#define _reg_PHY_SW_WRDQ1_SHIFT                            0x00000007U
#define _reg_PHY_SW_WRDQ2_SHIFT                            0x00000008U
#define _reg_PHY_SW_WRDQ3_SHIFT                            0x00000009U
#define _reg_PHY_SW_WRDQ4_SHIFT                            0x0000000aU
#define _reg_PHY_SW_WRDQ5_SHIFT                            0x0000000bU
#define _reg_PHY_SW_WRDQ6_SHIFT                            0x0000000cU
#define _reg_PHY_SW_WRDQ7_SHIFT                            0x0000000dU
#define _reg_PHY_SW_WRDM_SHIFT                             0x0000000eU
#define _reg_PHY_SW_WRDQS_SHIFT                            0x0000000fU
#define _reg_PHY_DQ_TSEL_ENABLE                            0x00000010U
#define _reg_PHY_DQ_TSEL_SELECT                            0x00000011U
#define _reg_PHY_DQS_TSEL_ENABLE                           0x00000012U
#define _reg_PHY_DQS_TSEL_SELECT                           0x00000013U
#define _reg_PHY_TWO_CYC_PREAMBLE                          0x00000014U
#define _reg_PHY_DBI_MODE                                  0x00000015U
#define _reg_PHY_PER_RANK_CS_MAP                           0x00000016U
#define _reg_PHY_PER_CS_TRAINING_MULTICAST_EN              0x00000017U
#define _reg_PHY_PER_CS_TRAINING_INDEX                     0x00000018U
#define _reg_PHY_LP4_BOOT_RDDATA_EN_IE_DLY                 0x00000019U
#define _reg_PHY_LP4_BOOT_RDDATA_EN_DLY                    0x0000001aU
#define _reg_PHY_LP4_BOOT_RDDATA_EN_TSEL_DLY               0x0000001bU
#define _reg_PHY_LP4_BOOT_RPTR_UPDATE                      0x0000001cU
#define _reg_PHY_LP4_BOOT_RDDQS_GATE_SLAVE_DELAY           0x0000001dU
#define _reg_PHY_LP4_BOOT_RDDQS_LATENCY_ADJUST             0x0000001eU
#define _reg_PHY_LP4_BOOT_WRPATH_GATE_DISABLE              0x0000001fU
#define _reg_PHY_LP4_BOOT_RDDATA_EN_OE_DLY                 0x00000020U
#define _reg_PHY_LPBK_CONTROL                              0x00000021U
#define _reg_PHY_LPBK_DFX_TIMEOUT_EN                       0x00000022U
#define _reg_PHY_AUTO_TIMING_MARGIN_CONTROL                0x00000023U
#define _reg_PHY_AUTO_TIMING_MARGIN_OBS                    0x00000024U
#define _reg_PHY_SLICE_PWR_RDC_DISABLE                     0x00000025U
#define _reg_PHY_PRBS_PATTERN_START                        0x00000026U
#define _reg_PHY_PRBS_PATTERN_MASK                         0x00000027U
#define _reg_PHY_RDDQS_DQ_BYPASS_SLAVE_DELAY               0x00000028U
#define _reg_PHY_GATE_ERROR_DELAY_SELECT                   0x00000029U
#define _reg_SC_PHY_SNAP_OBS_REGS                          0x0000002aU
#define _reg_PHY_LPDDR                                     0x0000002bU
#define _reg_PHY_LPDDR_TYPE                                0x0000002cU
#define _reg_PHY_GATE_SMPL1_SLAVE_DELAY                    0x0000002dU
#define _reg_PHY_GATE_SMPL2_SLAVE_DELAY                    0x0000002eU
#define _reg_ON_FLY_GATE_ADJUST_EN                         0x0000002fU
#define _reg_PHY_GATE_TRACKING_OBS                         0x00000030U
#define _reg_PHY_DFI40_POLARITY                            0x00000031U
#define _reg_PHY_LP4_PST_AMBLE                             0x00000032U
#define _reg_PHY_RDLVL_PATT8                               0x00000033U
#define _reg_PHY_RDLVL_PATT9                               0x00000034U
#define _reg_PHY_RDLVL_PATT10                              0x00000035U
#define _reg_PHY_RDLVL_PATT11                              0x00000036U
#define _reg_PHY_LP4_RDLVL_PATT8                           0x00000037U
#define _reg_PHY_LP4_RDLVL_PATT9                           0x00000038U
#define _reg_PHY_LP4_RDLVL_PATT10                          0x00000039U
#define _reg_PHY_LP4_RDLVL_PATT11                          0x0000003aU
#define _reg_PHY_SLAVE_LOOP_CNT_UPDATE                     0x0000003bU
#define _reg_PHY_SW_FIFO_PTR_RST_DISABLE                   0x0000003cU
#define _reg_PHY_MASTER_DLY_LOCK_OBS_SELECT                0x0000003dU
#define _reg_PHY_RDDQ_ENC_OBS_SELECT                       0x0000003eU
#define _reg_PHY_RDDQS_DQ_ENC_OBS_SELECT                   0x0000003fU
#define _reg_PHY_WR_ENC_OBS_SELECT                         0x00000040U
#define _reg_PHY_WR_SHIFT_OBS_SELECT                       0x00000041U
#define _reg_PHY_FIFO_PTR_OBS_SELECT                       0x00000042U
#define _reg_PHY_LVL_DEBUG_MODE                            0x00000043U
#define _reg_SC_PHY_LVL_DEBUG_CONT                         0x00000044U
#define _reg_PHY_WRLVL_CAPTURE_CNT                         0x00000045U
#define _reg_PHY_WRLVL_UPDT_WAIT_CNT                       0x00000046U
#define _reg_PHY_WRLVL_DQ_MASK                             0x00000047U
#define _reg_PHY_GTLVL_CAPTURE_CNT                         0x00000048U
#define _reg_PHY_GTLVL_UPDT_WAIT_CNT                       0x00000049U
#define _reg_PHY_RDLVL_CAPTURE_CNT                         0x0000004aU
#define _reg_PHY_RDLVL_UPDT_WAIT_CNT                       0x0000004bU
#define _reg_PHY_RDLVL_OP_MODE                             0x0000004cU
#define _reg_PHY_RDLVL_RDDQS_DQ_OBS_SELECT                 0x0000004dU
#define _reg_PHY_RDLVL_DATA_MASK                           0x0000004eU
#define _reg_PHY_RDLVL_DATA_SWIZZLE                        0x0000004fU
#define _reg_PHY_WDQLVL_BURST_CNT                          0x00000050U
#define _reg_PHY_WDQLVL_PATT                               0x00000051U
#define _reg_PHY_WDQLVL_DQDM_SLV_DLY_JUMP_OFFSET           0x00000052U
#define _reg_PHY_WDQLVL_UPDT_WAIT_CNT                      0x00000053U
#define _reg_PHY_WDQLVL_DQDM_OBS_SELECT                    0x00000054U
#define _reg_PHY_WDQLVL_QTR_DLY_STEP                       0x00000055U
#define _reg_SC_PHY_WDQLVL_CLR_PREV_RESULTS                0x00000056U
#define _reg_PHY_WDQLVL_CLR_PREV_RESULTS                   0x00000057U
#define _reg_PHY_WDQLVL_DATADM_MASK                        0x00000058U
#define _reg_PHY_USER_PATT0                                0x00000059U
#define _reg_PHY_USER_PATT1                                0x0000005aU
#define _reg_PHY_USER_PATT2                                0x0000005bU
#define _reg_PHY_USER_PATT3                                0x0000005cU
#define _reg_PHY_USER_PATT4                                0x0000005dU
#define _reg_PHY_DQ_SWIZZLING                              0x0000005eU
#define _reg_PHY_CALVL_VREF_DRIVING_SLICE                  0x0000005fU
#define _reg_SC_PHY_MANUAL_CLEAR                           0x00000060U
#define _reg_PHY_FIFO_PTR_OBS                              0x00000061U
#define _reg_PHY_LPBK_RESULT_OBS                           0x00000062U
#define _reg_PHY_LPBK_ERROR_COUNT_OBS                      0x00000063U
#define _reg_PHY_MASTER_DLY_LOCK_OBS                       0x00000064U
#define _reg_PHY_RDDQ_SLV_DLY_ENC_OBS                      0x00000065U
#define _reg_PHY_RDDQS_BASE_SLV_DLY_ENC_OBS                0x00000066U
#define _reg_PHY_RDDQS_DQ_RISE_ADDER_SLV_DLY_ENC_OBS       0x00000067U
#define _reg_PHY_RDDQS_DQ_FALL_ADDER_SLV_DLY_ENC_OBS       0x00000068U
#define _reg_PHY_RDDQS_GATE_SLV_DLY_ENC_OBS                0x00000069U
#define _reg_PHY_WRDQS_BASE_SLV_DLY_ENC_OBS                0x0000006aU
#define _reg_PHY_WRDQ_BASE_SLV_DLY_ENC_OBS                 0x0000006bU
#define _reg_PHY_WR_ADDER_SLV_DLY_ENC_OBS                  0x0000006cU
#define _reg_PHY_WR_SHIFT_OBS                              0x0000006dU
#define _reg_PHY_WRLVL_HARD0_DELAY_OBS                     0x0000006eU
#define _reg_PHY_WRLVL_HARD1_DELAY_OBS                     0x0000006fU
#define _reg_PHY_WRLVL_STATUS_OBS                          0x00000070U
#define _reg_PHY_GATE_SMPL1_SLV_DLY_ENC_OBS                0x00000071U
#define _reg_PHY_GATE_SMPL2_SLV_DLY_ENC_OBS                0x00000072U
#define _reg_PHY_WRLVL_ERROR_OBS                           0x00000073U
#define _reg_PHY_GTLVL_HARD0_DELAY_OBS                     0x00000074U
#define _reg_PHY_GTLVL_HARD1_DELAY_OBS                     0x00000075U
#define _reg_PHY_GTLVL_STATUS_OBS                          0x00000076U
#define _reg_PHY_RDLVL_RDDQS_DQ_LE_DLY_OBS                 0x00000077U
#define _reg_PHY_RDLVL_RDDQS_DQ_TE_DLY_OBS                 0x00000078U
#define _reg_PHY_RDLVL_RDDQS_DQ_NUM_WINDOWS_OBS            0x00000079U
#define _reg_PHY_RDLVL_STATUS_OBS                          0x0000007aU
#define _reg_PHY_WDQLVL_DQDM_LE_DLY_OBS                    0x0000007bU
#define _reg_PHY_WDQLVL_DQDM_TE_DLY_OBS                    0x0000007cU
#define _reg_PHY_WDQLVL_STATUS_OBS                         0x0000007dU
#define _reg_PHY_DDL_MODE                                  0x0000007eU
#define _reg_PHY_DDL_TEST_OBS                              0x0000007fU
#define _reg_PHY_DDL_TEST_MSTR_DLY_OBS                     0x00000080U
#define _reg_PHY_DDL_TRACK_UPD_THRESHOLD                   0x00000081U
#define _reg_PHY_LP4_WDQS_OE_EXTEND                        0x00000082U
#define _reg_SC_PHY_RX_CAL_START                           0x00000083U
#define _reg_PHY_RX_CAL_OVERRIDE                           0x00000084U
#define _reg_PHY_RX_CAL_SAMPLE_WAIT                        0x00000085U
#define _reg_PHY_RX_CAL_DQ0                                0x00000086U
#define _reg_PHY_RX_CAL_DQ1                                0x00000087U
#define _reg_PHY_RX_CAL_DQ2                                0x00000088U
#define _reg_PHY_RX_CAL_DQ3                                0x00000089U
#define _reg_PHY_RX_CAL_DQ4                                0x0000008aU
#define _reg_PHY_RX_CAL_DQ5                                0x0000008bU
#define _reg_PHY_RX_CAL_DQ6                                0x0000008cU
#define _reg_PHY_RX_CAL_DQ7                                0x0000008dU
#define _reg_PHY_RX_CAL_DM                                 0x0000008eU
#define _reg_PHY_RX_CAL_DQS                                0x0000008fU
#define _reg_PHY_RX_CAL_FDBK                               0x00000090U
#define _reg_PHY_RX_CAL_OBS                                0x00000091U
#define _reg_PHY_RX_CAL_LOCK_OBS                           0x00000092U
#define _reg_PHY_RX_CAL_DISABLE                            0x00000093U
#define _reg_PHY_CLK_WRDQ0_SLAVE_DELAY                     0x00000094U
#define _reg_PHY_CLK_WRDQ1_SLAVE_DELAY                     0x00000095U
#define _reg_PHY_CLK_WRDQ2_SLAVE_DELAY                     0x00000096U
#define _reg_PHY_CLK_WRDQ3_SLAVE_DELAY                     0x00000097U
#define _reg_PHY_CLK_WRDQ4_SLAVE_DELAY                     0x00000098U
#define _reg_PHY_CLK_WRDQ5_SLAVE_DELAY                     0x00000099U
#define _reg_PHY_CLK_WRDQ6_SLAVE_DELAY                     0x0000009aU
#define _reg_PHY_CLK_WRDQ7_SLAVE_DELAY                     0x0000009bU
#define _reg_PHY_CLK_WRDM_SLAVE_DELAY                      0x0000009cU
#define _reg_PHY_CLK_WRDQS_SLAVE_DELAY                     0x0000009dU
#define _reg_PHY_WRLVL_THRESHOLD_ADJUST                    0x0000009eU
#define _reg_PHY_RDDQ0_SLAVE_DELAY                         0x0000009fU
#define _reg_PHY_RDDQ1_SLAVE_DELAY                         0x000000a0U
#define _reg_PHY_RDDQ2_SLAVE_DELAY                         0x000000a1U
#define _reg_PHY_RDDQ3_SLAVE_DELAY                         0x000000a2U
#define _reg_PHY_RDDQ4_SLAVE_DELAY                         0x000000a3U
#define _reg_PHY_RDDQ5_SLAVE_DELAY                         0x000000a4U
#define _reg_PHY_RDDQ6_SLAVE_DELAY                         0x000000a5U
#define _reg_PHY_RDDQ7_SLAVE_DELAY                         0x000000a6U
#define _reg_PHY_RDDM_SLAVE_DELAY                          0x000000a7U
#define _reg_PHY_RDDQS_DQ0_RISE_SLAVE_DELAY                0x000000a8U
#define _reg_PHY_RDDQS_DQ0_FALL_SLAVE_DELAY                0x000000a9U
#define _reg_PHY_RDDQS_DQ1_RISE_SLAVE_DELAY                0x000000aaU
#define _reg_PHY_RDDQS_DQ1_FALL_SLAVE_DELAY                0x000000abU
#define _reg_PHY_RDDQS_DQ2_RISE_SLAVE_DELAY                0x000000acU
#define _reg_PHY_RDDQS_DQ2_FALL_SLAVE_DELAY                0x000000adU
#define _reg_PHY_RDDQS_DQ3_RISE_SLAVE_DELAY                0x000000aeU
#define _reg_PHY_RDDQS_DQ3_FALL_SLAVE_DELAY                0x000000afU
#define _reg_PHY_RDDQS_DQ4_RISE_SLAVE_DELAY                0x000000b0U
#define _reg_PHY_RDDQS_DQ4_FALL_SLAVE_DELAY                0x000000b1U
#define _reg_PHY_RDDQS_DQ5_RISE_SLAVE_DELAY                0x000000b2U
#define _reg_PHY_RDDQS_DQ5_FALL_SLAVE_DELAY                0x000000b3U
#define _reg_PHY_RDDQS_DQ6_RISE_SLAVE_DELAY                0x000000b4U
#define _reg_PHY_RDDQS_DQ6_FALL_SLAVE_DELAY                0x000000b5U
#define _reg_PHY_RDDQS_DQ7_RISE_SLAVE_DELAY                0x000000b6U
#define _reg_PHY_RDDQS_DQ7_FALL_SLAVE_DELAY                0x000000b7U
#define _reg_PHY_RDDQS_DM_RISE_SLAVE_DELAY                 0x000000b8U
#define _reg_PHY_RDDQS_DM_FALL_SLAVE_DELAY                 0x000000b9U
#define _reg_PHY_RDDQS_GATE_SLAVE_DELAY                    0x000000baU
#define _reg_PHY_RDDQS_LATENCY_ADJUST                      0x000000bbU
#define _reg_PHY_WRITE_PATH_LAT_ADD                        0x000000bcU
#define _reg_PHY_WRLVL_DELAY_EARLY_THRESHOLD               0x000000bdU
#define _reg_PHY_WRLVL_DELAY_PERIOD_THRESHOLD              0x000000beU
#define _reg_PHY_WRLVL_EARLY_FORCE_ZERO                    0x000000bfU
#define _reg_PHY_GTLVL_RDDQS_SLV_DLY_START                 0x000000c0U
#define _reg_PHY_GTLVL_LAT_ADJ_START                       0x000000c1U
#define _reg_PHY_WDQLVL_DQDM_SLV_DLY_START                 0x000000c2U
#define _reg_PHY_RDLVL_RDDQS_DQ_SLV_DLY_START              0x000000c3U
#define _reg_PHY_FDBK_PWR_CTRL                             0x000000c4U
#define _reg_PHY_DQ_OE_TIMING                              0x000000c5U
#define _reg_PHY_DQ_TSEL_RD_TIMING                         0x000000c6U
#define _reg_PHY_DQ_TSEL_WR_TIMING                         0x000000c7U
#define _reg_PHY_DQS_OE_TIMING                             0x000000c8U
#define _reg_PHY_DQS_TSEL_RD_TIMING                        0x000000c9U
#define _reg_PHY_DQS_OE_RD_TIMING                          0x000000caU
#define _reg_PHY_DQS_TSEL_WR_TIMING                        0x000000cbU
#define _reg_PHY_PER_CS_TRAINING_EN                        0x000000ccU
#define _reg_PHY_DQ_IE_TIMING                              0x000000cdU
#define _reg_PHY_DQS_IE_TIMING                             0x000000ceU
#define _reg_PHY_RDDATA_EN_IE_DLY                          0x000000cfU
#define _reg_PHY_IE_MODE                                   0x000000d0U
#define _reg_PHY_RDDATA_EN_DLY                             0x000000d1U
#define _reg_PHY_RDDATA_EN_TSEL_DLY                        0x000000d2U
#define _reg_PHY_RDDATA_EN_OE_DLY                          0x000000d3U
#define _reg_PHY_SW_MASTER_MODE                            0x000000d4U
#define _reg_PHY_MASTER_DELAY_START                        0x000000d5U
#define _reg_PHY_MASTER_DELAY_STEP                         0x000000d6U
#define _reg_PHY_MASTER_DELAY_WAIT                         0x000000d7U
#define _reg_PHY_MASTER_DELAY_HALF_MEASURE                 0x000000d8U
#define _reg_PHY_RPTR_UPDATE                               0x000000d9U
#define _reg_PHY_WRLVL_DLY_STEP                            0x000000daU
#define _reg_PHY_WRLVL_RESP_WAIT_CNT                       0x000000dbU
#define _reg_PHY_GTLVL_DLY_STEP                            0x000000dcU
#define _reg_PHY_GTLVL_RESP_WAIT_CNT                       0x000000ddU
#define _reg_PHY_GTLVL_BACK_STEP                           0x000000deU
#define _reg_PHY_GTLVL_FINAL_STEP                          0x000000dfU
#define _reg_PHY_WDQLVL_DLY_STEP                           0x000000e0U
#define _reg_PHY_TOGGLE_PRE_SUPPORT                        0x000000e1U
#define _reg_PHY_RDLVL_DLY_STEP                            0x000000e2U
#define _reg_PHY_WRPATH_GATE_DISABLE                       0x000000e3U
#define _reg_PHY_WRPATH_GATE_TIMING                        0x000000e4U
#define _reg_PHY_ADR0_SW_WRADDR_SHIFT                      0x000000e5U
#define _reg_PHY_ADR1_SW_WRADDR_SHIFT                      0x000000e6U
#define _reg_PHY_ADR2_SW_WRADDR_SHIFT                      0x000000e7U
#define _reg_PHY_ADR3_SW_WRADDR_SHIFT                      0x000000e8U
#define _reg_PHY_ADR4_SW_WRADDR_SHIFT                      0x000000e9U
#define _reg_PHY_ADR5_SW_WRADDR_SHIFT                      0x000000eaU
#define _reg_PHY_ADR_CLK_WR_BYPASS_SLAVE_DELAY             0x000000ebU
#define _reg_PHY_ADR_CLK_BYPASS_OVERRIDE                   0x000000ecU
#define _reg_SC_PHY_ADR_MANUAL_CLEAR                       0x000000edU
#define _reg_PHY_ADR_LPBK_RESULT_OBS                       0x000000eeU
#define _reg_PHY_ADR_LPBK_ERROR_COUNT_OBS                  0x000000efU
#define _reg_PHY_ADR_MASTER_DLY_LOCK_OBS_SELECT            0x000000f0U
#define _reg_PHY_ADR_MASTER_DLY_LOCK_OBS                   0x000000f1U
#define _reg_PHY_ADR_BASE_SLV_DLY_ENC_OBS                  0x000000f2U
#define _reg_PHY_ADR_ADDER_SLV_DLY_ENC_OBS                 0x000000f3U
#define _reg_PHY_ADR_SLAVE_LOOP_CNT_UPDATE                 0x000000f4U
#define _reg_PHY_ADR_SLV_DLY_ENC_OBS_SELECT                0x000000f5U
#define _reg_SC_PHY_ADR_SNAP_OBS_REGS                      0x000000f6U
#define _reg_PHY_ADR_TSEL_ENABLE                           0x000000f7U
#define _reg_PHY_ADR_LPBK_CONTROL                          0x000000f8U
#define _reg_PHY_ADR_PRBS_PATTERN_START                    0x000000f9U
#define _reg_PHY_ADR_PRBS_PATTERN_MASK                     0x000000faU
#define _reg_PHY_ADR_PWR_RDC_DISABLE                       0x000000fbU
#define _reg_PHY_ADR_TYPE                                  0x000000fcU
#define _reg_PHY_ADR_WRADDR_SHIFT_OBS                      0x000000fdU
#define _reg_PHY_ADR_IE_MODE                               0x000000feU
#define _reg_PHY_ADR_DDL_MODE                              0x000000ffU
#define _reg_PHY_ADR_DDL_TEST_OBS                          0x00000100U
#define _reg_PHY_ADR_DDL_TEST_MSTR_DLY_OBS                 0x00000101U
#define _reg_PHY_ADR_CALVL_START                           0x00000102U
#define _reg_PHY_ADR_CALVL_COARSE_DLY                      0x00000103U
#define _reg_PHY_ADR_CALVL_QTR                             0x00000104U
#define _reg_PHY_ADR_CALVL_SWIZZLE0                        0x00000105U
#define _reg_PHY_ADR_CALVL_SWIZZLE1                        0x00000106U
#define _reg_PHY_ADR_CALVL_SWIZZLE0_0                      0x00000107U
#define _reg_PHY_ADR_CALVL_SWIZZLE1_0                      0x00000108U
#define _reg_PHY_ADR_CALVL_SWIZZLE0_1                      0x00000109U
#define _reg_PHY_ADR_CALVL_SWIZZLE1_1                      0x0000010aU
#define _reg_PHY_ADR_CALVL_DEVICE_MAP                      0x0000010bU
#define _reg_PHY_ADR_CALVL_RANK_CTRL                       0x0000010cU
#define _reg_PHY_ADR_CALVL_NUM_PATTERNS                    0x0000010dU
#define _reg_PHY_ADR_CALVL_CAPTURE_CNT                     0x0000010eU
#define _reg_PHY_ADR_CALVL_RESP_WAIT_CNT                   0x0000010fU
#define _reg_PHY_ADR_CALVL_DEBUG_MODE                      0x00000110U
#define _reg_SC_PHY_ADR_CALVL_DEBUG_CONT                   0x00000111U
#define _reg_SC_PHY_ADR_CALVL_ERROR_CLR                    0x00000112U
#define _reg_PHY_ADR_CALVL_OBS_SELECT                      0x00000113U
#define _reg_PHY_ADR_CALVL_OBS0                            0x00000114U
#define _reg_PHY_ADR_CALVL_OBS1                            0x00000115U
#define _reg_PHY_ADR_CALVL_RESULT                          0x00000116U
#define _reg_PHY_ADR_CALVL_FG_0                            0x00000117U
#define _reg_PHY_ADR_CALVL_BG_0                            0x00000118U
#define _reg_PHY_ADR_CALVL_FG_1                            0x00000119U
#define _reg_PHY_ADR_CALVL_BG_1                            0x0000011aU
#define _reg_PHY_ADR_CALVL_FG_2                            0x0000011bU
#define _reg_PHY_ADR_CALVL_BG_2                            0x0000011cU
#define _reg_PHY_ADR_CALVL_FG_3                            0x0000011dU
#define _reg_PHY_ADR_CALVL_BG_3                            0x0000011eU
#define _reg_PHY_ADR_ADDR_SEL                              0x0000011fU
#define _reg_PHY_ADR_LP4_BOOT_SLV_DELAY                    0x00000120U
#define _reg_PHY_ADR_BIT_MASK                              0x00000121U
#define _reg_PHY_ADR_SEG_MASK                              0x00000122U
#define _reg_PHY_ADR_CALVL_TRAIN_MASK                      0x00000123U
#define _reg_PHY_ADR_CSLVL_TRAIN_MASK                      0x00000124U
#define _reg_PHY_ADR_SW_TXIO_CTRL                          0x00000125U
#define _reg_PHY_ADR_TSEL_SELECT                           0x00000126U
#define _reg_PHY_ADR0_CLK_WR_SLAVE_DELAY                   0x00000127U
#define _reg_PHY_ADR1_CLK_WR_SLAVE_DELAY                   0x00000128U
#define _reg_PHY_ADR2_CLK_WR_SLAVE_DELAY                   0x00000129U
#define _reg_PHY_ADR3_CLK_WR_SLAVE_DELAY                   0x0000012aU
#define _reg_PHY_ADR4_CLK_WR_SLAVE_DELAY                   0x0000012bU
#define _reg_PHY_ADR5_CLK_WR_SLAVE_DELAY                   0x0000012cU
#define _reg_PHY_ADR_SW_MASTER_MODE                        0x0000012dU
#define _reg_PHY_ADR_MASTER_DELAY_START                    0x0000012eU
#define _reg_PHY_ADR_MASTER_DELAY_STEP                     0x0000012fU
#define _reg_PHY_ADR_MASTER_DELAY_WAIT                     0x00000130U
#define _reg_PHY_ADR_MASTER_DELAY_HALF_MEASURE             0x00000131U
#define _reg_PHY_ADR_CALVL_DLY_STEP                        0x00000132U
#define _reg_PHY_FREQ_SEL                                  0x00000133U
#define _reg_PHY_FREQ_SEL_FROM_REGIF                       0x00000134U
#define _reg_PHY_FREQ_SEL_MULTICAST_EN                     0x00000135U
#define _reg_PHY_FREQ_SEL_INDEX                            0x00000136U
#define _reg_PHY_SW_GRP_SHIFT_0                            0x00000137U
#define _reg_PHY_SW_GRP_SHIFT_1                            0x00000138U
#define _reg_PHY_SW_GRP_SHIFT_2                            0x00000139U
#define _reg_PHY_SW_GRP_SHIFT_3                            0x0000013aU
#define _reg_PHY_GRP_BYPASS_SLAVE_DELAY                    0x0000013bU
#define _reg_PHY_SW_GRP_BYPASS_SHIFT                       0x0000013cU
#define _reg_PHY_GRP_BYPASS_OVERRIDE                       0x0000013dU
#define _reg_SC_PHY_MANUAL_UPDATE                          0x0000013eU
#define _reg_SC_PHY_MANUAL_UPDATE_PHYUPD_ENABLE            0x0000013fU
#define _reg_PHY_LP4_BOOT_DISABLE                          0x00000140U
#define _reg_PHY_CSLVL_ENABLE                              0x00000141U
#define _reg_PHY_CSLVL_CS_MAP                              0x00000142U
#define _reg_PHY_CSLVL_START                               0x00000143U
#define _reg_PHY_CSLVL_QTR                                 0x00000144U
#define _reg_PHY_CSLVL_COARSE_CHK                          0x00000145U
#define _reg_PHY_CSLVL_CAPTURE_CNT                         0x00000146U
#define _reg_PHY_CSLVL_COARSE_DLY                          0x00000147U
#define _reg_PHY_CSLVL_COARSE_CAPTURE_CNT                  0x00000148U
#define _reg_PHY_CSLVL_DEBUG_MODE                          0x00000149U
#define _reg_SC_PHY_CSLVL_DEBUG_CONT                       0x0000014aU
#define _reg_SC_PHY_CSLVL_ERROR_CLR                        0x0000014bU
#define _reg_PHY_CSLVL_OBS0                                0x0000014cU
#define _reg_PHY_CSLVL_OBS1                                0x0000014dU
#define _reg_PHY_CALVL_CS_MAP                              0x0000014eU
#define _reg_PHY_GRP_SLV_DLY_ENC_OBS_SELECT                0x0000014fU
#define _reg_PHY_GRP_SHIFT_OBS_SELECT                      0x00000150U
#define _reg_PHY_GRP_SLV_DLY_ENC_OBS                       0x00000151U
#define _reg_PHY_GRP_SHIFT_OBS                             0x00000152U
#define _reg_PHY_ADRCTL_SLAVE_LOOP_CNT_UPDATE              0x00000153U
#define _reg_PHY_ADRCTL_SNAP_OBS_REGS                      0x00000154U
#define _reg_PHY_DFI_PHYUPD_TYPE                           0x00000155U
#define _reg_PHY_ADRCTL_LPDDR                              0x00000156U
#define _reg_PHY_LP4_ACTIVE                                0x00000157U
#define _reg_PHY_LPDDR3_CS                                 0x00000158U
#define _reg_PHY_CALVL_RESULT_MASK                         0x00000159U
#define _reg_SC_PHY_UPDATE_CLK_CAL_VALUES                  0x0000015aU
#define _reg_PHY_SW_TXIO_CTRL_0                            0x0000015bU
#define _reg_PHY_SW_TXIO_CTRL_1                            0x0000015cU
#define _reg_PHY_SW_TXIO_CTRL_2                            0x0000015dU
#define _reg_PHY_SW_TXIO_CTRL_3                            0x0000015eU
#define _reg_PHY_MEMCLK_SW_TXIO_CTRL                       0x0000015fU
#define _reg_PHY_CA_SW_TXPWR_CTRL                          0x00000160U
#define _reg_PHY_MEMCLK_SW_TXPWR_CTRL                      0x00000161U
#define _reg_PHY_USER_DEF_REG_AC_0                         0x00000162U
#define _reg_PHY_USER_DEF_REG_AC_1                         0x00000163U
#define _reg_PHY_USER_DEF_REG_AC_2                         0x00000164U
#define _reg_PHY_USER_DEF_REG_AC_3                         0x00000165U
#define _reg_PHY_UPDATE_CLK_CAL_VALUES                     0x00000166U
#define _reg_PHY_CONTINUOUS_CLK_CAL_UPDATE                 0x00000167U
#define _reg_PHY_PLL_CTRL                                  0x00000168U
#define _reg_PHY_PLL_CTRL_TOP                              0x00000169U
#define _reg_PHY_PLL_CTRL_CA                               0x0000016aU
#define _reg_PHY_PLL_BYPASS                                0x0000016bU
#define _reg_PHY_LOW_FREQ_SEL                              0x0000016cU
#define _reg_PHY_PAD_VREF_CTRL_DQ_0                        0x0000016dU
#define _reg_PHY_PAD_VREF_CTRL_DQ_1                        0x0000016eU
#define _reg_PHY_PAD_VREF_CTRL_DQ_2                        0x0000016fU
#define _reg_PHY_PAD_VREF_CTRL_DQ_3                        0x00000170U
#define _reg_PHY_PAD_VREF_CTRL_AC                          0x00000171U
#define _reg_PHY_CSLVL_DLY_STEP                            0x00000172U
#define _reg_PHY_SET_DFI_INPUT_0                           0x00000173U
#define _reg_PHY_SET_DFI_INPUT_1                           0x00000174U
#define _reg_PHY_SET_DFI_INPUT_2                           0x00000175U
#define _reg_PHY_SET_DFI_INPUT_3                           0x00000176U
#define _reg_PHY_GRP_SLAVE_DELAY_0                         0x00000177U
#define _reg_PHY_GRP_SLAVE_DELAY_1                         0x00000178U
#define _reg_PHY_GRP_SLAVE_DELAY_2                         0x00000179U
#define _reg_PHY_GRP_SLAVE_DELAY_3                         0x0000017aU
#define _reg_PHY_CS_ACS_ALLOCATION_0                       0x0000017bU
#define _reg_PHY_CS_ACS_ALLOCATION_1                       0x0000017cU
#define _reg_PHY_CS_ACS_ALLOCATION_2                       0x0000017dU
#define _reg_PHY_CS_ACS_ALLOCATION_3                       0x0000017eU
#define _reg_PHY_LP4_BOOT_PLL_CTRL                         0x0000017fU
#define _reg_PHY_LP4_BOOT_PLL_CTRL_CA                      0x00000180U
#define _reg_PHY_LP4_BOOT_TOP_PLL_CTRL                     0x00000181U
#define _reg_PHY_PLL_CTRL_OVERRIDE                         0x00000182U
#define _reg_PHY_PLL_WAIT                                  0x00000183U
#define _reg_PHY_PLL_WAIT_TOP                              0x00000184U
#define _reg_PHY_PLL_OBS_0                                 0x00000185U
#define _reg_PHY_PLL_OBS_1                                 0x00000186U
#define _reg_PHY_PLL_OBS_2                                 0x00000187U
#define _reg_PHY_PLL_OBS_3                                 0x00000188U
#define _reg_PHY_PLL_OBS_4                                 0x00000189U
#define _reg_PHY_PLL_TESTOUT_SEL                           0x0000018aU
#define _reg_PHY_TCKSRE_WAIT                               0x0000018bU
#define _reg_PHY_LP4_BOOT_LOW_FREQ_SEL                     0x0000018cU
#define _reg_PHY_LP_WAKEUP                                 0x0000018dU
#define _reg_PHY_LS_IDLE_EN                                0x0000018eU
#define _reg_PHY_LP_CTRLUPD_CNTR_CFG                       0x0000018fU
#define _reg_PHY_TDFI_PHY_WRDELAY                          0x00000190U
#define _reg_PHY_PAD_FDBK_DRIVE                            0x00000191U
#define _reg_PHY_PAD_DATA_DRIVE                            0x00000192U
#define _reg_PHY_PAD_DQS_DRIVE                             0x00000193U
#define _reg_PHY_PAD_ADDR_DRIVE                            0x00000194U
#define _reg_PHY_PAD_CLK_DRIVE                             0x00000195U
#define _reg_PHY_PAD_FDBK_TERM                             0x00000196U
#define _reg_PHY_PAD_DATA_TERM                             0x00000197U
#define _reg_PHY_PAD_DQS_TERM                              0x00000198U
#define _reg_PHY_PAD_ADDR_TERM                             0x00000199U
#define _reg_PHY_PAD_CLK_TERM                              0x0000019aU
#define _reg_PHY_PAD_CKE_DRIVE                             0x0000019bU
#define _reg_PHY_PAD_CKE_TERM                              0x0000019cU
#define _reg_PHY_PAD_RST_DRIVE                             0x0000019dU
#define _reg_PHY_PAD_RST_TERM                              0x0000019eU
#define _reg_PHY_PAD_CS_DRIVE                              0x0000019fU
#define _reg_PHY_PAD_CS_TERM                               0x000001a0U
#define _reg_PHY_PAD_ODT_DRIVE                             0x000001a1U
#define _reg_PHY_PAD_ODT_TERM                              0x000001a2U
#define _reg_PHY_ADRCTL_RX_CAL                             0x000001a3U
#define _reg_PHY_ADRCTL_LP3_RX_CAL                         0x000001a4U
#define _reg_PHY_TST_CLK_PAD_CTRL                          0x000001a5U
#define _reg_PHY_TST_CLK_PAD_CTRL2                         0x000001a6U
#define _reg_PHY_CAL_MODE_0                                0x000001a7U
#define _reg_PHY_CAL_CLEAR_0                               0x000001a8U
#define _reg_PHY_CAL_START_0                               0x000001a9U
#define _reg_PHY_CAL_INTERVAL_COUNT_0                      0x000001aaU
#define _reg_PHY_CAL_SAMPLE_WAIT_0                         0x000001abU
#define _reg_PHY_LP4_BOOT_CAL_CLK_SELECT_0                 0x000001acU
#define _reg_PHY_CAL_CLK_SELECT_0                          0x000001adU
#define _reg_PHY_CAL_RESULT_OBS_0                          0x000001aeU
#define _reg_PHY_CAL_RESULT2_OBS_0                         0x000001afU
#define _reg_PHY_CAL_CPTR_CNT_0                            0x000001b0U
#define _reg_PHY_CAL_SETTLING_PRD_0                        0x000001b1U
#define _reg_PHY_CAL_PU_FINE_ADJ_0                         0x000001b2U
#define _reg_PHY_CAL_PD_FINE_ADJ_0                         0x000001b3U
#define _reg_PHY_CAL_RCV_FINE_ADJ_0                        0x000001b4U
#define _reg_PHY_CAL_DBG_CFG_0                             0x000001b5U
#define _reg_SC_PHY_PAD_DBG_CONT_0                         0x000001b6U
#define _reg_PHY_CAL_RESULT3_OBS_0                         0x000001b7U
#define _reg_PHY_ADRCTL_PVT_MAP_0                          0x000001b8U
#define _reg_PHY_CAL_SLOPE_ADJ_0                           0x000001b9U
#define _reg_PHY_CAL_SLOPE_ADJ_PASS2_0                     0x000001baU
#define _reg_PHY_CAL_TWO_PASS_CFG_0                        0x000001bbU
#define _reg_PHY_CAL_SW_CAL_CFG_0                          0x000001bcU
#define _reg_PHY_CAL_RANGE_MIN_0                           0x000001bdU
#define _reg_PHY_CAL_RANGE_MAX_0                           0x000001beU
#define _reg_PHY_PAD_ATB_CTRL                              0x000001bfU
#define _reg_PHY_ADRCTL_MANUAL_UPDATE                      0x000001c0U
#define _reg_PHY_AC_LPBK_ERR_CLEAR                         0x000001c1U
#define _reg_PHY_AC_LPBK_OBS_SELECT                        0x000001c2U
#define _reg_PHY_AC_LPBK_ENABLE                            0x000001c3U
#define _reg_PHY_AC_LPBK_CONTROL                           0x000001c4U
#define _reg_PHY_AC_PRBS_PATTERN_START                     0x000001c5U
#define _reg_PHY_AC_PRBS_PATTERN_MASK                      0x000001c6U
#define _reg_PHY_AC_LPBK_RESULT_OBS                        0x000001c7U
#define _reg_PHY_AC_CLK_LPBK_OBS_SELECT                    0x000001c8U
#define _reg_PHY_AC_CLK_LPBK_ENABLE                        0x000001c9U
#define _reg_PHY_AC_CLK_LPBK_CONTROL                       0x000001caU
#define _reg_PHY_AC_CLK_LPBK_RESULT_OBS                    0x000001cbU
#define _reg_PHY_AC_PWR_RDC_DISABLE                        0x000001ccU
#define _reg_PHY_DATA_BYTE_ORDER_SEL                       0x000001cdU
#define _reg_PHY_DATA_BYTE_ORDER_SEL_HIGH                  0x000001ceU
#define _reg_PHY_LPDDR4_CONNECT                            0x000001cfU
#define _reg_PHY_CALVL_DEVICE_MAP                          0x000001d0U
#define _reg_PHY_ADR_DISABLE                               0x000001d1U
#define _reg_PHY_ADRCTL_MSTR_DLY_ENC_SEL                   0x000001d2U
#define _reg_PHY_CS_DLY_UPT_PER_AC_SLICE                   0x000001d3U
#define _reg_PHY_DDL_AC_ENABLE                             0x000001d4U
#define _reg_PHY_DDL_AC_MODE                               0x000001d5U
#define _reg_PHY_PAD_BACKGROUND_CAL                        0x000001d6U
#define _reg_PHY_INIT_UPDATE_CONFIG                        0x000001d7U
#define _reg_PHY_DDL_TRACK_UPD_THRESHOLD_AC                0x000001d8U
#define _reg_PHY_DLL_RST_EN                                0x000001d9U
#define _reg_PHY_AC_INIT_COMPLETE_OBS                      0x000001daU
#define _reg_PHY_DS_INIT_COMPLETE_OBS                      0x000001dbU
#define _reg_PHY_UPDATE_MASK                               0x000001dcU
#define _reg_PHY_PLL_SWITCH_CNT                            0x000001ddU
#define _reg_PI_START                                      0x000001deU
#define _reg_PI_DRAM_CLASS                                 0x000001dfU
#define _reg_PI_VERSION                                    0x000001e0U
#define _reg_PI_NORMAL_LVL_SEQ                             0x000001e1U
#define _reg_PI_INIT_LVL_EN                                0x000001e2U
#define _reg_PI_NOTCARE_PHYUPD                             0x000001e3U
#define _reg_PI_ONBUS_MBIST                                0x000001e4U
#define _reg_PI_TCMD_GAP                                   0x000001e5U
#define _reg_PI_MASTER_ACK_DURATION_MIN                    0x000001e6U
#define _reg_PI_DFI_VERSION                                0x000001e7U
#define _reg_PI_TDFI_PHYMSTR_TYPE0                         0x000001e8U
#define _reg_PI_TDFI_PHYMSTR_TYPE1                         0x000001e9U
#define _reg_PI_TDFI_PHYMSTR_TYPE2                         0x000001eaU
#define _reg_PI_TDFI_PHYMSTR_TYPE3                         0x000001ebU
#define _reg_PI_DFI_PHYMSTR_TYPE                           0x000001ecU
#define _reg_PI_DFI_PHYMSTR_CS_STATE_R                     0x000001edU
#define _reg_PI_DFI_PHYMSTR_STATE_SEL_R                    0x000001eeU
#define _reg_PI_TDFI_PHYMSTR_MAX_F0                        0x000001efU
#define _reg_PI_TDFI_PHYMSTR_RESP_F0                       0x000001f0U
#define _reg_PI_TDFI_PHYMSTR_MAX_F1                        0x000001f1U
#define _reg_PI_TDFI_PHYMSTR_RESP_F1                       0x000001f2U
#define _reg_PI_TDFI_PHYMSTR_MAX_F2                        0x000001f3U
#define _reg_PI_TDFI_PHYMSTR_RESP_F2                       0x000001f4U
#define _reg_PI_TDFI_PHYUPD_RESP_F0                        0x000001f5U
#define _reg_PI_TDFI_PHYUPD_TYPE0_F0                       0x000001f6U
#define _reg_PI_TDFI_PHYUPD_TYPE1_F0                       0x000001f7U
#define _reg_PI_TDFI_PHYUPD_TYPE2_F0                       0x000001f8U
#define _reg_PI_TDFI_PHYUPD_TYPE3_F0                       0x000001f9U
#define _reg_PI_TDFI_PHYUPD_RESP_F1                        0x000001faU
#define _reg_PI_TDFI_PHYUPD_TYPE0_F1                       0x000001fbU
#define _reg_PI_TDFI_PHYUPD_TYPE1_F1                       0x000001fcU
#define _reg_PI_TDFI_PHYUPD_TYPE2_F1                       0x000001fdU
#define _reg_PI_TDFI_PHYUPD_TYPE3_F1                       0x000001feU
#define _reg_PI_TDFI_PHYUPD_RESP_F2                        0x000001ffU
#define _reg_PI_TDFI_PHYUPD_TYPE0_F2                       0x00000200U
#define _reg_PI_TDFI_PHYUPD_TYPE1_F2                       0x00000201U
#define _reg_PI_TDFI_PHYUPD_TYPE2_F2                       0x00000202U
#define _reg_PI_TDFI_PHYUPD_TYPE3_F2                       0x00000203U
#define _reg_PI_CONTROL_ERROR_STATUS                       0x00000204U
#define _reg_PI_EXIT_AFTER_INIT_CALVL                      0x00000205U
#define _reg_PI_FREQ_MAP                                   0x00000206U
#define _reg_PI_INIT_WORK_FREQ                             0x00000207U
#define _reg_PI_INIT_DFS_CALVL_ONLY                        0x00000208U
#define _reg_PI_POWER_ON_SEQ_BYPASS_ARRAY                  0x00000209U
#define _reg_PI_POWER_ON_SEQ_END_ARRAY                     0x0000020aU
#define _reg_PI_SEQ1_PAT                                   0x0000020bU
#define _reg_PI_SEQ1_PAT_MASK                              0x0000020cU
#define _reg_PI_SEQ2_PAT                                   0x0000020dU
#define _reg_PI_SEQ2_PAT_MASK                              0x0000020eU
#define _reg_PI_SEQ3_PAT                                   0x0000020fU
#define _reg_PI_SEQ3_PAT_MASK                              0x00000210U
#define _reg_PI_SEQ4_PAT                                   0x00000211U
#define _reg_PI_SEQ4_PAT_MASK                              0x00000212U
#define _reg_PI_SEQ5_PAT                                   0x00000213U
#define _reg_PI_SEQ5_PAT_MASK                              0x00000214U
#define _reg_PI_SEQ6_PAT                                   0x00000215U
#define _reg_PI_SEQ6_PAT_MASK                              0x00000216U
#define _reg_PI_SEQ7_PAT                                   0x00000217U
#define _reg_PI_SEQ7_PAT_MASK                              0x00000218U
#define _reg_PI_SEQ8_PAT                                   0x00000219U
#define _reg_PI_SEQ8_PAT_MASK                              0x0000021aU
#define _reg_PI_WDT_DISABLE                                0x0000021bU
#define _reg_PI_SW_RST_N                                   0x0000021cU
#define _reg_RESERVED_R0                                   0x0000021dU
#define _reg_PI_CS_MAP                                     0x0000021eU
#define _reg_PI_TDELAY_RDWR_2_BUS_IDLE_F0                  0x0000021fU
#define _reg_PI_TDELAY_RDWR_2_BUS_IDLE_F1                  0x00000220U
#define _reg_PI_TDELAY_RDWR_2_BUS_IDLE_F2                  0x00000221U
#define _reg_PI_TMRR                                       0x00000222U
#define _reg_PI_WRLAT_F0                                   0x00000223U
#define _reg_PI_ADDITIVE_LAT_F0                            0x00000224U
#define _reg_PI_CASLAT_LIN_F0                              0x00000225U
#define _reg_PI_WRLAT_F1                                   0x00000226U
#define _reg_PI_ADDITIVE_LAT_F1                            0x00000227U
#define _reg_PI_CASLAT_LIN_F1                              0x00000228U
#define _reg_PI_WRLAT_F2                                   0x00000229U
#define _reg_PI_ADDITIVE_LAT_F2                            0x0000022aU
#define _reg_PI_CASLAT_LIN_F2                              0x0000022bU
#define _reg_PI_PREAMBLE_SUPPORT                           0x0000022cU
#define _reg_PI_AREFRESH                                   0x0000022dU
#define _reg_PI_MCAREF_FORWARD_ONLY                        0x0000022eU
#define _reg_PI_TRFC_F0                                    0x0000022fU
#define _reg_PI_TREF_F0                                    0x00000230U
#define _reg_PI_TRFC_F1                                    0x00000231U
#define _reg_PI_TREF_F1                                    0x00000232U
#define _reg_PI_TRFC_F2                                    0x00000233U
#define _reg_PI_TREF_F2                                    0x00000234U
#define _reg_RESERVED_H3VER2                               0x00000235U
#define _reg_PI_TREF_INTERVAL                              0x00000236U
#define _reg_PI_FREQ_CHANGE_REG_COPY                       0x00000237U
#define _reg_PI_FREQ_SEL_FROM_REGIF                        0x00000238U
#define _reg_PI_SWLVL_LOAD                                 0x00000239U
#define _reg_PI_SWLVL_OP_DONE                              0x0000023aU
#define _reg_PI_SW_WRLVL_RESP_0                            0x0000023bU
#define _reg_PI_SW_WRLVL_RESP_1                            0x0000023cU
#define _reg_PI_SW_WRLVL_RESP_2                            0x0000023dU
#define _reg_PI_SW_WRLVL_RESP_3                            0x0000023eU
#define _reg_PI_SW_RDLVL_RESP_0                            0x0000023fU
#define _reg_PI_SW_RDLVL_RESP_1                            0x00000240U
#define _reg_PI_SW_RDLVL_RESP_2                            0x00000241U
#define _reg_PI_SW_RDLVL_RESP_3                            0x00000242U
#define _reg_PI_SW_CALVL_RESP_0                            0x00000243U
#define _reg_PI_SW_LEVELING_MODE                           0x00000244U
#define _reg_PI_SWLVL_START                                0x00000245U
#define _reg_PI_SWLVL_EXIT                                 0x00000246U
#define _reg_PI_SWLVL_WR_SLICE_0                           0x00000247U
#define _reg_PI_SWLVL_RD_SLICE_0                           0x00000248U
#define _reg_PI_SWLVL_VREF_UPDATE_SLICE_0                  0x00000249U
#define _reg_PI_SW_WDQLVL_RESP_0                           0x0000024aU
#define _reg_PI_SWLVL_WR_SLICE_1                           0x0000024bU
#define _reg_PI_SWLVL_RD_SLICE_1                           0x0000024cU
#define _reg_PI_SWLVL_VREF_UPDATE_SLICE_1                  0x0000024dU
#define _reg_PI_SW_WDQLVL_RESP_1                           0x0000024eU
#define _reg_PI_SWLVL_WR_SLICE_2                           0x0000024fU
#define _reg_PI_SWLVL_RD_SLICE_2                           0x00000250U
#define _reg_PI_SWLVL_VREF_UPDATE_SLICE_2                  0x00000251U
#define _reg_PI_SW_WDQLVL_RESP_2                           0x00000252U
#define _reg_PI_SWLVL_WR_SLICE_3                           0x00000253U
#define _reg_PI_SWLVL_RD_SLICE_3                           0x00000254U
#define _reg_PI_SWLVL_VREF_UPDATE_SLICE_3                  0x00000255U
#define _reg_PI_SW_WDQLVL_RESP_3                           0x00000256U
#define _reg_PI_SW_WDQLVL_VREF                             0x00000257U
#define _reg_PI_SWLVL_SM2_START                            0x00000258U
#define _reg_PI_SWLVL_SM2_WR                               0x00000259U
#define _reg_PI_SWLVL_SM2_RD                               0x0000025aU
#define _reg_PI_SEQUENTIAL_LVL_REQ                         0x0000025bU
#define _reg_PI_DFS_PERIOD_EN                              0x0000025cU
#define _reg_PI_SRE_PERIOD_EN                              0x0000025dU
#define _reg_PI_DFI40_POLARITY                             0x0000025eU
#define _reg_PI_16BIT_DRAM_CONNECT                         0x0000025fU
#define _reg_PI_TDFI_CTRL_DELAY_F0                         0x00000260U
#define _reg_PI_TDFI_CTRL_DELAY_F1                         0x00000261U
#define _reg_PI_TDFI_CTRL_DELAY_F2                         0x00000262U
#define _reg_PI_WRLVL_REQ                                  0x00000263U
#define _reg_PI_WRLVL_CS                                   0x00000264U
#define _reg_PI_WLDQSEN                                    0x00000265U
#define _reg_PI_WLMRD                                      0x00000266U
#define _reg_PI_WRLVL_EN_F0                                0x00000267U
#define _reg_PI_WRLVL_EN_F1                                0x00000268U
#define _reg_PI_WRLVL_EN_F2                                0x00000269U
#define _reg_PI_WRLVL_EN                                   0x0000026aU
#define _reg_PI_WRLVL_INTERVAL                             0x0000026bU
#define _reg_PI_WRLVL_PERIODIC                             0x0000026cU
#define _reg_PI_WRLVL_ON_SREF_EXIT                         0x0000026dU
#define _reg_PI_WRLVL_DISABLE_DFS                          0x0000026eU
#define _reg_PI_WRLVL_RESP_MASK                            0x0000026fU
#define _reg_PI_WRLVL_ROTATE                               0x00000270U
#define _reg_PI_WRLVL_CS_MAP                               0x00000271U
#define _reg_PI_WRLVL_ERROR_STATUS                         0x00000272U
#define _reg_PI_TDFI_WRLVL_EN                              0x00000273U
#define _reg_PI_TDFI_WRLVL_WW_F0                           0x00000274U
#define _reg_PI_TDFI_WRLVL_WW_F1                           0x00000275U
#define _reg_PI_TDFI_WRLVL_WW_F2                           0x00000276U
#define _reg_PI_TDFI_WRLVL_WW                              0x00000277U
#define _reg_PI_TDFI_WRLVL_RESP                            0x00000278U
#define _reg_PI_TDFI_WRLVL_MAX                             0x00000279U
#define _reg_PI_WRLVL_STROBE_NUM                           0x0000027aU
#define _reg_PI_WRLVL_MRR_DQ_RETURN_HIZ                    0x0000027bU
#define _reg_PI_WRLVL_EN_DEASSERT_2_MRR                    0x0000027cU
#define _reg_PI_TODTL_2CMD_F0                              0x0000027dU
#define _reg_PI_ODT_EN_F0                                  0x0000027eU
#define _reg_PI_TODTL_2CMD_F1                              0x0000027fU
#define _reg_PI_ODT_EN_F1                                  0x00000280U
#define _reg_PI_TODTL_2CMD_F2                              0x00000281U
#define _reg_PI_ODT_EN_F2                                  0x00000282U
#define _reg_PI_TODTH_WR                                   0x00000283U
#define _reg_PI_TODTH_RD                                   0x00000284U
#define _reg_PI_ODT_RD_MAP_CS0                             0x00000285U
#define _reg_PI_ODT_WR_MAP_CS0                             0x00000286U
#define _reg_PI_ODT_RD_MAP_CS1                             0x00000287U
#define _reg_PI_ODT_WR_MAP_CS1                             0x00000288U
#define _reg_PI_ODT_RD_MAP_CS2                             0x00000289U
#define _reg_PI_ODT_WR_MAP_CS2                             0x0000028aU
#define _reg_PI_ODT_RD_MAP_CS3                             0x0000028bU
#define _reg_PI_ODT_WR_MAP_CS3                             0x0000028cU
#define _reg_PI_EN_ODT_ASSERT_EXCEPT_RD                    0x0000028dU
#define _reg_PI_ODTLON_F0                                  0x0000028eU
#define _reg_PI_TODTON_MIN_F0                              0x0000028fU
#define _reg_PI_ODTLON_F1                                  0x00000290U
#define _reg_PI_TODTON_MIN_F1                              0x00000291U
#define _reg_PI_ODTLON_F2                                  0x00000292U
#define _reg_PI_TODTON_MIN_F2                              0x00000293U
#define _reg_PI_WR_TO_ODTH_F0                              0x00000294U
#define _reg_PI_WR_TO_ODTH_F1                              0x00000295U
#define _reg_PI_WR_TO_ODTH_F2                              0x00000296U
#define _reg_PI_RD_TO_ODTH_F0                              0x00000297U
#define _reg_PI_RD_TO_ODTH_F1                              0x00000298U
#define _reg_PI_RD_TO_ODTH_F2                              0x00000299U
#define _reg_PI_ADDRESS_MIRRORING                          0x0000029aU
#define _reg_PI_RDLVL_REQ                                  0x0000029bU
#define _reg_PI_RDLVL_GATE_REQ                             0x0000029cU
#define _reg_PI_RDLVL_CS                                   0x0000029dU
#define _reg_PI_RDLVL_PAT_0                                0x0000029eU
#define _reg_PI_RDLVL_PAT_1                                0x0000029fU
#define _reg_PI_RDLVL_PAT_2                                0x000002a0U
#define _reg_PI_RDLVL_PAT_3                                0x000002a1U
#define _reg_PI_RDLVL_PAT_4                                0x000002a2U
#define _reg_PI_RDLVL_PAT_5                                0x000002a3U
#define _reg_PI_RDLVL_PAT_6                                0x000002a4U
#define _reg_PI_RDLVL_PAT_7                                0x000002a5U
#define _reg_PI_RDLVL_SEQ_EN                               0x000002a6U
#define _reg_PI_RDLVL_GATE_SEQ_EN                          0x000002a7U
#define _reg_PI_RDLVL_PERIODIC                             0x000002a8U
#define _reg_PI_RDLVL_ON_SREF_EXIT                         0x000002a9U
#define _reg_PI_RDLVL_DISABLE_DFS                          0x000002aaU
#define _reg_PI_RDLVL_GATE_PERIODIC                        0x000002abU
#define _reg_PI_RDLVL_GATE_ON_SREF_EXIT                    0x000002acU
#define _reg_PI_RDLVL_GATE_DISABLE_DFS                     0x000002adU
#define _reg_RESERVED_R1                                   0x000002aeU
#define _reg_PI_RDLVL_ROTATE                               0x000002afU
#define _reg_PI_RDLVL_GATE_ROTATE                          0x000002b0U
#define _reg_PI_RDLVL_CS_MAP                               0x000002b1U
#define _reg_PI_RDLVL_GATE_CS_MAP                          0x000002b2U
#define _reg_PI_TDFI_RDLVL_RR                              0x000002b3U
#define _reg_PI_TDFI_RDLVL_RESP                            0x000002b4U
#define _reg_PI_RDLVL_RESP_MASK                            0x000002b5U
#define _reg_PI_TDFI_RDLVL_EN                              0x000002b6U
#define _reg_PI_RDLVL_EN_F0                                0x000002b7U
#define _reg_PI_RDLVL_GATE_EN_F0                           0x000002b8U
#define _reg_PI_RDLVL_EN_F1                                0x000002b9U
#define _reg_PI_RDLVL_GATE_EN_F1                           0x000002baU
#define _reg_PI_RDLVL_EN_F2                                0x000002bbU
#define _reg_PI_RDLVL_GATE_EN_F2                           0x000002bcU
#define _reg_PI_RDLVL_EN                                   0x000002bdU
#define _reg_PI_RDLVL_GATE_EN                              0x000002beU
#define _reg_PI_TDFI_RDLVL_MAX                             0x000002bfU
#define _reg_PI_RDLVL_ERROR_STATUS                         0x000002c0U
#define _reg_PI_RDLVL_INTERVAL                             0x000002c1U
#define _reg_PI_RDLVL_GATE_INTERVAL                        0x000002c2U
#define _reg_PI_RDLVL_PATTERN_START                        0x000002c3U
#define _reg_PI_RDLVL_PATTERN_NUM                          0x000002c4U
#define _reg_PI_RDLVL_STROBE_NUM                           0x000002c5U
#define _reg_PI_RDLVL_GATE_STROBE_NUM                      0x000002c6U
#define _reg_PI_LPDDR4_RDLVL_PATTERN_8                     0x000002c7U
#define _reg_PI_LPDDR4_RDLVL_PATTERN_9                     0x000002c8U
#define _reg_PI_LPDDR4_RDLVL_PATTERN_10                    0x000002c9U
#define _reg_PI_LPDDR4_RDLVL_PATTERN_11                    0x000002caU
#define _reg_PI_RD_PREAMBLE_TRAINING_EN                    0x000002cbU
#define _reg_PI_REG_DIMM_ENABLE                            0x000002ccU
#define _reg_PI_RDLAT_ADJ_F0                               0x000002cdU
#define _reg_PI_RDLAT_ADJ_F1                               0x000002ceU
#define _reg_PI_RDLAT_ADJ_F2                               0x000002cfU
#define _reg_PI_TDFI_RDDATA_EN                             0x000002d0U
#define _reg_PI_WRLAT_ADJ_F0                               0x000002d1U
#define _reg_PI_WRLAT_ADJ_F1                               0x000002d2U
#define _reg_PI_WRLAT_ADJ_F2                               0x000002d3U
#define _reg_PI_TDFI_PHY_WRLAT                             0x000002d4U
#define _reg_PI_TDFI_WRCSLAT_F0                            0x000002d5U
#define _reg_PI_TDFI_WRCSLAT_F1                            0x000002d6U
#define _reg_PI_TDFI_WRCSLAT_F2                            0x000002d7U
#define _reg_PI_TDFI_RDCSLAT_F0                            0x000002d8U
#define _reg_PI_TDFI_RDCSLAT_F1                            0x000002d9U
#define _reg_PI_TDFI_RDCSLAT_F2                            0x000002daU
#define _reg_PI_TDFI_PHY_WRDATA_F0                         0x000002dbU
#define _reg_PI_TDFI_PHY_WRDATA_F1                         0x000002dcU
#define _reg_PI_TDFI_PHY_WRDATA_F2                         0x000002ddU
#define _reg_PI_TDFI_PHY_WRDATA                            0x000002deU
#define _reg_PI_CALVL_REQ                                  0x000002dfU
#define _reg_PI_CALVL_CS                                   0x000002e0U
#define _reg_RESERVED_R2                                   0x000002e1U
#define _reg_RESERVED_R3                                   0x000002e2U
#define _reg_PI_CALVL_SEQ_EN                               0x000002e3U
#define _reg_PI_CALVL_PERIODIC                             0x000002e4U
#define _reg_PI_CALVL_ON_SREF_EXIT                         0x000002e5U
#define _reg_PI_CALVL_DISABLE_DFS                          0x000002e6U
#define _reg_PI_CALVL_ROTATE                               0x000002e7U
#define _reg_PI_CALVL_CS_MAP                               0x000002e8U
#define _reg_PI_TDFI_CALVL_EN                              0x000002e9U
#define _reg_PI_TDFI_CALVL_CC_F0                           0x000002eaU
#define _reg_PI_TDFI_CALVL_CAPTURE_F0                      0x000002ebU
#define _reg_PI_TDFI_CALVL_CC_F1                           0x000002ecU
#define _reg_PI_TDFI_CALVL_CAPTURE_F1                      0x000002edU
#define _reg_PI_TDFI_CALVL_CC_F2                           0x000002eeU
#define _reg_PI_TDFI_CALVL_CAPTURE_F2                      0x000002efU
#define _reg_PI_TDFI_CALVL_RESP                            0x000002f0U
#define _reg_PI_TDFI_CALVL_MAX                             0x000002f1U
#define _reg_PI_CALVL_RESP_MASK                            0x000002f2U
#define _reg_PI_CALVL_EN_F0                                0x000002f3U
#define _reg_PI_CALVL_EN_F1                                0x000002f4U
#define _reg_PI_CALVL_EN_F2                                0x000002f5U
#define _reg_PI_CALVL_EN                                   0x000002f6U
#define _reg_PI_CALVL_ERROR_STATUS                         0x000002f7U
#define _reg_PI_CALVL_INTERVAL                             0x000002f8U
#define _reg_PI_TCACKEL                                    0x000002f9U
#define _reg_PI_TCAMRD                                     0x000002faU
#define _reg_PI_TCACKEH                                    0x000002fbU
#define _reg_PI_TMRZ_F0                                    0x000002fcU
#define _reg_PI_TCAENT_F0                                  0x000002fdU
#define _reg_PI_TMRZ_F1                                    0x000002feU
#define _reg_PI_TCAENT_F1                                  0x000002ffU
#define _reg_PI_TMRZ_F2                                    0x00000300U
#define _reg_PI_TCAENT_F2                                  0x00000301U
#define _reg_PI_TCAEXT                                     0x00000302U
#define _reg_PI_CA_TRAIN_VREF_EN                           0x00000303U
#define _reg_PI_TDFI_CACSCA_F0                             0x00000304U
#define _reg_PI_TDFI_CASEL_F0                              0x00000305U
#define _reg_PI_TVREF_SHORT_F0                             0x00000306U
#define _reg_PI_TVREF_LONG_F0                              0x00000307U
#define _reg_PI_TDFI_CACSCA_F1                             0x00000308U
#define _reg_PI_TDFI_CASEL_F1                              0x00000309U
#define _reg_PI_TVREF_SHORT_F1                             0x0000030aU
#define _reg_PI_TVREF_LONG_F1                              0x0000030bU
#define _reg_PI_TDFI_CACSCA_F2                             0x0000030cU
#define _reg_PI_TDFI_CASEL_F2                              0x0000030dU
#define _reg_PI_TVREF_SHORT_F2                             0x0000030eU
#define _reg_PI_TVREF_LONG_F2                              0x0000030fU
#define _reg_PI_CALVL_VREF_INITIAL_START_POINT_F0          0x00000310U
#define _reg_PI_CALVL_VREF_INITIAL_STOP_POINT_F0           0x00000311U
#define _reg_PI_CALVL_VREF_INITIAL_START_POINT_F1          0x00000312U
#define _reg_PI_CALVL_VREF_INITIAL_STOP_POINT_F1           0x00000313U
#define _reg_PI_CALVL_VREF_INITIAL_START_POINT_F2          0x00000314U
#define _reg_PI_CALVL_VREF_INITIAL_STOP_POINT_F2           0x00000315U
#define _reg_PI_CALVL_VREF_INITIAL_START_POINT             0x00000316U
#define _reg_PI_CALVL_VREF_INITIAL_STOP_POINT              0x00000317U
#define _reg_PI_CALVL_VREF_INITIAL_STEPSIZE                0x00000318U
#define _reg_PI_CALVL_VREF_NORMAL_STEPSIZE                 0x00000319U
#define _reg_PI_CALVL_VREF_DELTA_F0                        0x0000031aU
#define _reg_PI_CALVL_VREF_DELTA_F1                        0x0000031bU
#define _reg_PI_CALVL_VREF_DELTA_F2                        0x0000031cU
#define _reg_PI_CALVL_VREF_DELTA                           0x0000031dU
#define _reg_PI_TDFI_INIT_START_MIN                        0x0000031eU
#define _reg_PI_TDFI_INIT_COMPLETE_MIN                     0x0000031fU
#define _reg_PI_TDFI_CALVL_STROBE_F0                       0x00000320U
#define _reg_PI_TXP_F0                                     0x00000321U
#define _reg_PI_TMRWCKEL_F0                                0x00000322U
#define _reg_PI_TCKELCK_F0                                 0x00000323U
#define _reg_PI_TDFI_CALVL_STROBE_F1                       0x00000324U
#define _reg_PI_TXP_F1                                     0x00000325U
#define _reg_PI_TMRWCKEL_F1                                0x00000326U
#define _reg_PI_TCKELCK_F1                                 0x00000327U
#define _reg_PI_TDFI_CALVL_STROBE_F2                       0x00000328U
#define _reg_PI_TXP_F2                                     0x00000329U
#define _reg_PI_TMRWCKEL_F2                                0x0000032aU
#define _reg_PI_TCKELCK_F2                                 0x0000032bU
#define _reg_PI_TCKCKEH                                    0x0000032cU
#define _reg_PI_CALVL_STROBE_NUM                           0x0000032dU
#define _reg_PI_SW_CA_TRAIN_VREF                           0x0000032eU
#define _reg_PI_TDFI_INIT_START_F0                         0x0000032fU
#define _reg_PI_TDFI_INIT_COMPLETE_F0                      0x00000330U
#define _reg_PI_TDFI_INIT_START_F1                         0x00000331U
#define _reg_PI_TDFI_INIT_COMPLETE_F1                      0x00000332U
#define _reg_PI_TDFI_INIT_START_F2                         0x00000333U
#define _reg_PI_TDFI_INIT_COMPLETE_F2                      0x00000334U
#define _reg_PI_CLKDISABLE_2_INIT_START                    0x00000335U
#define _reg_PI_INIT_STARTORCOMPLETE_2_CLKDISABLE          0x00000336U
#define _reg_PI_DRAM_CLK_DISABLE_DEASSERT_SEL              0x00000337U
#define _reg_PI_REFRESH_BETWEEN_SEGMENT_DISABLE            0x00000338U
#define _reg_PI_TCKEHDQS_F0                                0x00000339U
#define _reg_PI_TCKEHDQS_F1                                0x0000033aU
#define _reg_PI_TCKEHDQS_F2                                0x0000033bU
#define _reg_PI_MC_DFS_PI_SET_VREF_ENABLE                  0x0000033cU
#define _reg_PI_WDQLVL_VREF_EN                             0x0000033dU
#define _reg_PI_WDQLVL_BST_NUM                             0x0000033eU
#define _reg_PI_TDFI_WDQLVL_WR_F0                          0x0000033fU
#define _reg_PI_TDFI_WDQLVL_WR_F1                          0x00000340U
#define _reg_PI_TDFI_WDQLVL_WR_F2                          0x00000341U
#define _reg_PI_TDFI_WDQLVL_WR                             0x00000342U
#define _reg_PI_TDFI_WDQLVL_RW                             0x00000343U
#define _reg_PI_WDQLVL_RESP_MASK                           0x00000344U
#define _reg_PI_WDQLVL_ROTATE                              0x00000345U
#define _reg_PI_WDQLVL_CS_MAP                              0x00000346U
#define _reg_PI_WDQLVL_VREF_INITIAL_START_POINT_F0         0x00000347U
#define _reg_PI_WDQLVL_VREF_INITIAL_STOP_POINT_F0          0x00000348U
#define _reg_PI_WDQLVL_VREF_INITIAL_START_POINT_F1         0x00000349U
#define _reg_PI_WDQLVL_VREF_INITIAL_STOP_POINT_F1          0x0000034aU
#define _reg_PI_WDQLVL_VREF_INITIAL_START_POINT_F2         0x0000034bU
#define _reg_PI_WDQLVL_VREF_INITIAL_STOP_POINT_F2          0x0000034cU
#define _reg_PI_WDQLVL_VREF_INITIAL_START_POINT            0x0000034dU
#define _reg_PI_WDQLVL_VREF_INITIAL_STOP_POINT             0x0000034eU
#define _reg_PI_WDQLVL_VREF_INITIAL_STEPSIZE               0x0000034fU
#define _reg_PI_WDQLVL_VREF_NORMAL_STEPSIZE                0x00000350U
#define _reg_PI_WDQLVL_VREF_DELTA_F0                       0x00000351U
#define _reg_PI_WDQLVL_VREF_DELTA_F1                       0x00000352U
#define _reg_PI_WDQLVL_VREF_DELTA_F2                       0x00000353U
#define _reg_PI_WDQLVL_VREF_DELTA                          0x00000354U
#define _reg_PI_WDQLVL_PERIODIC                            0x00000355U
#define _reg_PI_WDQLVL_REQ                                 0x00000356U
#define _reg_PI_WDQLVL_CS                                  0x00000357U
#define _reg_PI_TDFI_WDQLVL_EN                             0x00000358U
#define _reg_PI_TDFI_WDQLVL_RESP                           0x00000359U
#define _reg_PI_TDFI_WDQLVL_MAX                            0x0000035aU
#define _reg_PI_WDQLVL_INTERVAL                            0x0000035bU
#define _reg_PI_WDQLVL_EN_F0                               0x0000035cU
#define _reg_PI_WDQLVL_EN_F1                               0x0000035dU
#define _reg_PI_WDQLVL_EN_F2                               0x0000035eU
#define _reg_PI_WDQLVL_EN                                  0x0000035fU
#define _reg_PI_WDQLVL_ON_SREF_EXIT                        0x00000360U
#define _reg_PI_WDQLVL_DISABLE_DFS                         0x00000361U
#define _reg_PI_WDQLVL_ERROR_STATUS                        0x00000362U
#define _reg_PI_MR1_DATA_F0_0                              0x00000363U
#define _reg_PI_MR2_DATA_F0_0                              0x00000364U
#define _reg_PI_MR3_DATA_F0_0                              0x00000365U
#define _reg_PI_MR11_DATA_F0_0                             0x00000366U
#define _reg_PI_MR12_DATA_F0_0                             0x00000367U
#define _reg_PI_MR14_DATA_F0_0                             0x00000368U
#define _reg_PI_MR22_DATA_F0_0                             0x00000369U
#define _reg_PI_MR1_DATA_F1_0                              0x0000036aU
#define _reg_PI_MR2_DATA_F1_0                              0x0000036bU
#define _reg_PI_MR3_DATA_F1_0                              0x0000036cU
#define _reg_PI_MR11_DATA_F1_0                             0x0000036dU
#define _reg_PI_MR12_DATA_F1_0                             0x0000036eU
#define _reg_PI_MR14_DATA_F1_0                             0x0000036fU
#define _reg_PI_MR22_DATA_F1_0                             0x00000370U
#define _reg_PI_MR1_DATA_F2_0                              0x00000371U
#define _reg_PI_MR2_DATA_F2_0                              0x00000372U
#define _reg_PI_MR3_DATA_F2_0                              0x00000373U
#define _reg_PI_MR11_DATA_F2_0                             0x00000374U
#define _reg_PI_MR12_DATA_F2_0                             0x00000375U
#define _reg_PI_MR14_DATA_F2_0                             0x00000376U
#define _reg_PI_MR22_DATA_F2_0                             0x00000377U
#define _reg_PI_MR13_DATA_0                                0x00000378U
#define _reg_PI_MR1_DATA_F0_1                              0x00000379U
#define _reg_PI_MR2_DATA_F0_1                              0x0000037aU
#define _reg_PI_MR3_DATA_F0_1                              0x0000037bU
#define _reg_PI_MR11_DATA_F0_1                             0x0000037cU
#define _reg_PI_MR12_DATA_F0_1                             0x0000037dU
#define _reg_PI_MR14_DATA_F0_1                             0x0000037eU
#define _reg_PI_MR22_DATA_F0_1                             0x0000037fU
#define _reg_PI_MR1_DATA_F1_1                              0x00000380U
#define _reg_PI_MR2_DATA_F1_1                              0x00000381U
#define _reg_PI_MR3_DATA_F1_1                              0x00000382U
#define _reg_PI_MR11_DATA_F1_1                             0x00000383U
#define _reg_PI_MR12_DATA_F1_1                             0x00000384U
#define _reg_PI_MR14_DATA_F1_1                             0x00000385U
#define _reg_PI_MR22_DATA_F1_1                             0x00000386U
#define _reg_PI_MR1_DATA_F2_1                              0x00000387U
#define _reg_PI_MR2_DATA_F2_1                              0x00000388U
#define _reg_PI_MR3_DATA_F2_1                              0x00000389U
#define _reg_PI_MR11_DATA_F2_1                             0x0000038aU
#define _reg_PI_MR12_DATA_F2_1                             0x0000038bU
#define _reg_PI_MR14_DATA_F2_1                             0x0000038cU
#define _reg_PI_MR22_DATA_F2_1                             0x0000038dU
#define _reg_PI_MR13_DATA_1                                0x0000038eU
#define _reg_PI_MR1_DATA_F0_2                              0x0000038fU
#define _reg_PI_MR2_DATA_F0_2                              0x00000390U
#define _reg_PI_MR3_DATA_F0_2                              0x00000391U
#define _reg_PI_MR11_DATA_F0_2                             0x00000392U
#define _reg_PI_MR12_DATA_F0_2                             0x00000393U
#define _reg_PI_MR14_DATA_F0_2                             0x00000394U
#define _reg_PI_MR22_DATA_F0_2                             0x00000395U
#define _reg_PI_MR1_DATA_F1_2                              0x00000396U
#define _reg_PI_MR2_DATA_F1_2                              0x00000397U
#define _reg_PI_MR3_DATA_F1_2                              0x00000398U
#define _reg_PI_MR11_DATA_F1_2                             0x00000399U
#define _reg_PI_MR12_DATA_F1_2                             0x0000039aU
#define _reg_PI_MR14_DATA_F1_2                             0x0000039bU
#define _reg_PI_MR22_DATA_F1_2                             0x0000039cU
#define _reg_PI_MR1_DATA_F2_2                              0x0000039dU
#define _reg_PI_MR2_DATA_F2_2                              0x0000039eU
#define _reg_PI_MR3_DATA_F2_2                              0x0000039fU
#define _reg_PI_MR11_DATA_F2_2                             0x000003a0U
#define _reg_PI_MR12_DATA_F2_2                             0x000003a1U
#define _reg_PI_MR14_DATA_F2_2                             0x000003a2U
#define _reg_PI_MR22_DATA_F2_2                             0x000003a3U
#define _reg_PI_MR13_DATA_2                                0x000003a4U
#define _reg_PI_MR1_DATA_F0_3                              0x000003a5U
#define _reg_PI_MR2_DATA_F0_3                              0x000003a6U
#define _reg_PI_MR3_DATA_F0_3                              0x000003a7U
#define _reg_PI_MR11_DATA_F0_3                             0x000003a8U
#define _reg_PI_MR12_DATA_F0_3                             0x000003a9U
#define _reg_PI_MR14_DATA_F0_3                             0x000003aaU
#define _reg_PI_MR22_DATA_F0_3                             0x000003abU
#define _reg_PI_MR1_DATA_F1_3                              0x000003acU
#define _reg_PI_MR2_DATA_F1_3                              0x000003adU
#define _reg_PI_MR3_DATA_F1_3                              0x000003aeU
#define _reg_PI_MR11_DATA_F1_3                             0x000003afU
#define _reg_PI_MR12_DATA_F1_3                             0x000003b0U
#define _reg_PI_MR14_DATA_F1_3                             0x000003b1U
#define _reg_PI_MR22_DATA_F1_3                             0x000003b2U
#define _reg_PI_MR1_DATA_F2_3                              0x000003b3U
#define _reg_PI_MR2_DATA_F2_3                              0x000003b4U
#define _reg_PI_MR3_DATA_F2_3                              0x000003b5U
#define _reg_PI_MR11_DATA_F2_3                             0x000003b6U
#define _reg_PI_MR12_DATA_F2_3                             0x000003b7U
#define _reg_PI_MR14_DATA_F2_3                             0x000003b8U
#define _reg_PI_MR22_DATA_F2_3                             0x000003b9U
#define _reg_PI_MR13_DATA_3                                0x000003baU
#define _reg_PI_BANK_DIFF                                  0x000003bbU
#define _reg_PI_ROW_DIFF                                   0x000003bcU
#define _reg_PI_TFC_F0                                     0x000003bdU
#define _reg_PI_TFC_F1                                     0x000003beU
#define _reg_PI_TFC_F2                                     0x000003bfU
#define _reg_PI_TCCD                                       0x000003c0U
#define _reg_PI_TRTP_F0                                    0x000003c1U
#define _reg_PI_TRP_F0                                     0x000003c2U
#define _reg_PI_TRCD_F0                                    0x000003c3U
#define _reg_PI_TWTR_F0                                    0x000003c4U
#define _reg_PI_TWR_F0                                     0x000003c5U
#define _reg_PI_TRAS_MAX_F0                                0x000003c6U
#define _reg_PI_TRAS_MIN_F0                                0x000003c7U
#define _reg_PI_TDQSCK_MAX_F0                              0x000003c8U
#define _reg_PI_TCCDMW_F0                                  0x000003c9U
#define _reg_PI_TSR_F0                                     0x000003caU
#define _reg_PI_TMRD_F0                                    0x000003cbU
#define _reg_PI_TMRW_F0                                    0x000003ccU
#define _reg_PI_TMOD_F0                                    0x000003cdU
#define _reg_PI_TRTP_F1                                    0x000003ceU
#define _reg_PI_TRP_F1                                     0x000003cfU
#define _reg_PI_TRCD_F1                                    0x000003d0U
#define _reg_PI_TWTR_F1                                    0x000003d1U
#define _reg_PI_TWR_F1                                     0x000003d2U
#define _reg_PI_TRAS_MAX_F1                                0x000003d3U
#define _reg_PI_TRAS_MIN_F1                                0x000003d4U
#define _reg_PI_TDQSCK_MAX_F1                              0x000003d5U
#define _reg_PI_TCCDMW_F1                                  0x000003d6U
#define _reg_PI_TSR_F1                                     0x000003d7U
#define _reg_PI_TMRD_F1                                    0x000003d8U
#define _reg_PI_TMRW_F1                                    0x000003d9U
#define _reg_PI_TMOD_F1                                    0x000003daU
#define _reg_PI_TRTP_F2                                    0x000003dbU
#define _reg_PI_TRP_F2                                     0x000003dcU
#define _reg_PI_TRCD_F2                                    0x000003ddU
#define _reg_PI_TWTR_F2                                    0x000003deU
#define _reg_PI_TWR_F2                                     0x000003dfU
#define _reg_PI_TRAS_MAX_F2                                0x000003e0U
#define _reg_PI_TRAS_MIN_F2                                0x000003e1U
#define _reg_PI_TDQSCK_MAX_F2                              0x000003e2U
#define _reg_PI_TCCDMW_F2                                  0x000003e3U
#define _reg_PI_TSR_F2                                     0x000003e4U
#define _reg_PI_TMRD_F2                                    0x000003e5U
#define _reg_PI_TMRW_F2                                    0x000003e6U
#define _reg_PI_TMOD_F2                                    0x000003e7U
#define _reg_RESERVED_R4                                   0x000003e8U
#define _reg_RESERVED_R5                                   0x000003e9U
#define _reg_RESERVED_R6                                   0x000003eaU
#define _reg_RESERVED_R7                                   0x000003ebU
#define _reg_RESERVED_R8                                   0x000003ecU
#define _reg_RESERVED_R9                                   0x000003edU
#define _reg_RESERVED_R10                                  0x000003eeU
#define _reg_RESERVED_R11                                  0x000003efU
#define _reg_RESERVED_R12                                  0x000003f0U
#define _reg_RESERVED_R13                                  0x000003f1U
#define _reg_RESERVED_R14                                  0x000003f2U
#define _reg_RESERVED_R15                                  0x000003f3U
#define _reg_RESERVED_R16                                  0x000003f4U
#define _reg_RESERVED_R17                                  0x000003f5U
#define _reg_RESERVED_R18                                  0x000003f6U
#define _reg_RESERVED_R19                                  0x000003f7U
#define _reg_RESERVED_R20                                  0x000003f8U
#define _reg_RESERVED_R21                                  0x000003f9U
#define _reg_RESERVED_R22                                  0x000003faU
#define _reg_RESERVED_R23                                  0x000003fbU
#define _reg_PI_INT_STATUS                                 0x000003fcU
#define _reg_PI_INT_ACK                                    0x000003fdU
#define _reg_PI_INT_MASK                                   0x000003feU
#define _reg_PI_BIST_EXP_DATA_P0                           0x000003ffU
#define _reg_PI_BIST_EXP_DATA_P1                           0x00000400U
#define _reg_PI_BIST_EXP_DATA_P2                           0x00000401U
#define _reg_PI_BIST_EXP_DATA_P3                           0x00000402U
#define _reg_PI_BIST_FAIL_DATA_P0                          0x00000403U
#define _reg_PI_BIST_FAIL_DATA_P1                          0x00000404U
#define _reg_PI_BIST_FAIL_DATA_P2                          0x00000405U
#define _reg_PI_BIST_FAIL_DATA_P3                          0x00000406U
#define _reg_PI_BIST_FAIL_ADDR_P0                          0x00000407U
#define _reg_PI_BIST_FAIL_ADDR_P1                          0x00000408U
#define _reg_PI_BSTLEN                                     0x00000409U
#define _reg_PI_LONG_COUNT_MASK                            0x0000040aU
#define _reg_PI_CMD_SWAP_EN                                0x0000040bU
#define _reg_PI_CKE_MUX_0                                  0x0000040cU
#define _reg_PI_CKE_MUX_1                                  0x0000040dU
#define _reg_PI_CKE_MUX_2                                  0x0000040eU
#define _reg_PI_CKE_MUX_3                                  0x0000040fU
#define _reg_PI_CS_MUX_0                                   0x00000410U
#define _reg_PI_CS_MUX_1                                   0x00000411U
#define _reg_PI_CS_MUX_2                                   0x00000412U
#define _reg_PI_CS_MUX_3                                   0x00000413U
#define _reg_PI_RAS_N_MUX                                  0x00000414U
#define _reg_PI_CAS_N_MUX                                  0x00000415U
#define _reg_PI_WE_N_MUX                                   0x00000416U
#define _reg_PI_BANK_MUX_0                                 0x00000417U
#define _reg_PI_BANK_MUX_1                                 0x00000418U
#define _reg_PI_BANK_MUX_2                                 0x00000419U
#define _reg_PI_ODT_MUX_0                                  0x0000041aU
#define _reg_PI_ODT_MUX_1                                  0x0000041bU
#define _reg_PI_ODT_MUX_2                                  0x0000041cU
#define _reg_PI_ODT_MUX_3                                  0x0000041dU
#define _reg_PI_RESET_N_MUX_0                              0x0000041eU
#define _reg_PI_RESET_N_MUX_1                              0x0000041fU
#define _reg_PI_RESET_N_MUX_2                              0x00000420U
#define _reg_PI_RESET_N_MUX_3                              0x00000421U
#define _reg_PI_DATA_BYTE_SWAP_EN                          0x00000422U
#define _reg_PI_DATA_BYTE_SWAP_SLICE0                      0x00000423U
#define _reg_PI_DATA_BYTE_SWAP_SLICE1                      0x00000424U
#define _reg_PI_DATA_BYTE_SWAP_SLICE2                      0x00000425U
#define _reg_PI_DATA_BYTE_SWAP_SLICE3                      0x00000426U
#define _reg_PI_CTRLUPD_REQ_PER_AREF_EN                    0x00000427U
#define _reg_PI_TDFI_CTRLUPD_MIN                           0x00000428U
#define _reg_PI_TDFI_CTRLUPD_MAX_F0                        0x00000429U
#define _reg_PI_TDFI_CTRLUPD_INTERVAL_F0                   0x0000042aU
#define _reg_PI_TDFI_CTRLUPD_MAX_F1                        0x0000042bU
#define _reg_PI_TDFI_CTRLUPD_INTERVAL_F1                   0x0000042cU
#define _reg_PI_TDFI_CTRLUPD_MAX_F2                        0x0000042dU
#define _reg_PI_TDFI_CTRLUPD_INTERVAL_F2                   0x0000042eU
#define _reg_PI_UPDATE_ERROR_STATUS                        0x0000042fU
#define _reg_PI_BIST_GO                                    0x00000430U
#define _reg_PI_BIST_RESULT                                0x00000431U
#define _reg_PI_ADDR_SPACE                                 0x00000432U
#define _reg_PI_BIST_DATA_CHECK                            0x00000433U
#define _reg_PI_BIST_ADDR_CHECK                            0x00000434U
#define _reg_PI_BIST_START_ADDRESS_P0                      0x00000435U
#define _reg_PI_BIST_START_ADDRESS_P1                      0x00000436U
#define _reg_PI_BIST_DATA_MASK_P0                          0x00000437U
#define _reg_PI_BIST_DATA_MASK_P1                          0x00000438U
#define _reg_PI_BIST_ERR_COUNT                             0x00000439U
#define _reg_PI_BIST_ERR_STOP                              0x0000043aU
#define _reg_PI_BIST_ADDR_MASK_0_P0                        0x0000043bU
#define _reg_PI_BIST_ADDR_MASK_0_P1                        0x0000043cU
#define _reg_PI_BIST_ADDR_MASK_1_P0                        0x0000043dU
#define _reg_PI_BIST_ADDR_MASK_1_P1                        0x0000043eU
#define _reg_PI_BIST_ADDR_MASK_2_P0                        0x0000043fU
#define _reg_PI_BIST_ADDR_MASK_2_P1                        0x00000440U
#define _reg_PI_BIST_ADDR_MASK_3_P0                        0x00000441U
#define _reg_PI_BIST_ADDR_MASK_3_P1                        0x00000442U
#define _reg_PI_BIST_ADDR_MASK_4_P0                        0x00000443U
#define _reg_PI_BIST_ADDR_MASK_4_P1                        0x00000444U
#define _reg_PI_BIST_ADDR_MASK_5_P0                        0x00000445U
#define _reg_PI_BIST_ADDR_MASK_5_P1                        0x00000446U
#define _reg_PI_BIST_ADDR_MASK_6_P0                        0x00000447U
#define _reg_PI_BIST_ADDR_MASK_6_P1                        0x00000448U
#define _reg_PI_BIST_ADDR_MASK_7_P0                        0x00000449U
#define _reg_PI_BIST_ADDR_MASK_7_P1                        0x0000044aU
#define _reg_PI_BIST_ADDR_MASK_8_P0                        0x0000044bU
#define _reg_PI_BIST_ADDR_MASK_8_P1                        0x0000044cU
#define _reg_PI_BIST_ADDR_MASK_9_P0                        0x0000044dU
#define _reg_PI_BIST_ADDR_MASK_9_P1                        0x0000044eU
#define _reg_PI_BIST_MODE                                  0x0000044fU
#define _reg_PI_BIST_ADDR_MODE                             0x00000450U
#define _reg_PI_BIST_PAT_MODE                              0x00000451U
#define _reg_PI_BIST_USER_PAT_P0                           0x00000452U
#define _reg_PI_BIST_USER_PAT_P1                           0x00000453U
#define _reg_PI_BIST_USER_PAT_P2                           0x00000454U
#define _reg_PI_BIST_USER_PAT_P3                           0x00000455U
#define _reg_PI_BIST_PAT_NUM                               0x00000456U
#define _reg_PI_BIST_STAGE_0                               0x00000457U
#define _reg_PI_BIST_STAGE_1                               0x00000458U
#define _reg_PI_BIST_STAGE_2                               0x00000459U
#define _reg_PI_BIST_STAGE_3                               0x0000045aU
#define _reg_PI_BIST_STAGE_4                               0x0000045bU
#define _reg_PI_BIST_STAGE_5                               0x0000045cU
#define _reg_PI_BIST_STAGE_6                               0x0000045dU
#define _reg_PI_BIST_STAGE_7                               0x0000045eU
#define _reg_PI_COL_DIFF                                   0x0000045fU
#define _reg_PI_SELF_REFRESH_EN                            0x00000460U
#define _reg_PI_TXSR_F0                                    0x00000461U
#define _reg_PI_TXSR_F1                                    0x00000462U
#define _reg_PI_TXSR_F2                                    0x00000463U
#define _reg_PI_MONITOR_SRC_SEL_0                          0x00000464U
#define _reg_PI_MONITOR_CAP_SEL_0                          0x00000465U
#define _reg_PI_MONITOR_0                                  0x00000466U
#define _reg_PI_MONITOR_SRC_SEL_1                          0x00000467U
#define _reg_PI_MONITOR_CAP_SEL_1                          0x00000468U
#define _reg_PI_MONITOR_1                                  0x00000469U
#define _reg_PI_MONITOR_SRC_SEL_2                          0x0000046aU
#define _reg_PI_MONITOR_CAP_SEL_2                          0x0000046bU
#define _reg_PI_MONITOR_2                                  0x0000046cU
#define _reg_PI_MONITOR_SRC_SEL_3                          0x0000046dU
#define _reg_PI_MONITOR_CAP_SEL_3                          0x0000046eU
#define _reg_PI_MONITOR_3                                  0x0000046fU
#define _reg_PI_MONITOR_SRC_SEL_4                          0x00000470U
#define _reg_PI_MONITOR_CAP_SEL_4                          0x00000471U
#define _reg_PI_MONITOR_4                                  0x00000472U
#define _reg_PI_MONITOR_SRC_SEL_5                          0x00000473U
#define _reg_PI_MONITOR_CAP_SEL_5                          0x00000474U
#define _reg_PI_MONITOR_5                                  0x00000475U
#define _reg_PI_MONITOR_SRC_SEL_6                          0x00000476U
#define _reg_PI_MONITOR_CAP_SEL_6                          0x00000477U
#define _reg_PI_MONITOR_6                                  0x00000478U
#define _reg_PI_MONITOR_SRC_SEL_7                          0x00000479U
#define _reg_PI_MONITOR_CAP_SEL_7                          0x0000047aU
#define _reg_PI_MONITOR_7                                  0x0000047bU
#define _reg_PI_MONITOR_STROBE                             0x0000047cU
#define _reg_PI_DLL_LOCK                                   0x0000047dU
#define _reg_PI_FREQ_NUMBER_STATUS                         0x0000047eU
#define _reg_RESERVED_R24                                  0x0000047fU
#define _reg_PI_PHYMSTR_TYPE                               0x00000480U
#define _reg_PI_POWER_REDUC_EN                             0x00000481U
#define _reg_RESERVED_R25                                  0x00000482U
#define _reg_RESERVED_R26                                  0x00000483U
#define _reg_RESERVED_R27                                  0x00000484U
#define _reg_RESERVED_R28                                  0x00000485U
#define _reg_RESERVED_R29                                  0x00000486U
#define _reg_RESERVED_R30                                  0x00000487U
#define _reg_RESERVED_R31                                  0x00000488U
#define _reg_RESERVED_R32                                  0x00000489U
#define _reg_RESERVED_R33                                  0x0000048aU
#define _reg_RESERVED_R34                                  0x0000048bU
#define _reg_RESERVED_R35                                  0x0000048cU
#define _reg_RESERVED_R36                                  0x0000048dU
#define _reg_RESERVED_R37                                  0x0000048eU
#define _reg_RESERVED_R38                                  0x0000048fU
#define _reg_RESERVED_R39                                  0x00000490U
#define _reg_PI_WRLVL_MAX_STROBE_PEND                      0x00000491U
#define _reg_PI_TSDO_F0                                    0x00000492U
#define _reg_PI_TSDO_F1                                    0x00000493U
#define _reg_PI_TSDO_F2                                    0x00000494U

#define DDR_REGDEF_ADR(regdef) ((regdef)&0xffff)
#define DDR_REGDEF_LEN(regdef) (((regdef)>>16)&0xff)
#define DDR_REGDEF_LSB(regdef) (((regdef)>>24)&0xff)

const static uint32_t DDR_REGDEF_TBL[4][1173] = {
	{
/*0000*/ 0xffffffffU,
/*0001*/ 0xffffffffU,
/*0002*/ 0x000b0400U,
/*0003*/ 0xffffffffU,
/*0004*/ 0xffffffffU,
/*0005*/ 0x10010400U,
/*0006*/ 0x18050400U,
/*0007*/ 0x00050401U,
/*0008*/ 0x08050401U,
/*0009*/ 0x10050401U,
/*000a*/ 0x18050401U,
/*000b*/ 0x00050402U,
/*000c*/ 0x08050402U,
/*000d*/ 0x10050402U,
/*000e*/ 0x18050402U,
/*000f*/ 0x00040403U,
/*0010*/ 0x08030403U,
/*0011*/ 0x00180404U,
/*0012*/ 0x18030404U,
/*0013*/ 0x00180405U,
/*0014*/ 0x18020405U,
/*0015*/ 0x00010406U,
/*0016*/ 0x08020406U,
/*0017*/ 0x10010406U,
/*0018*/ 0x18010406U,
/*0019*/ 0x00020407U,
/*001a*/ 0x08040407U,
/*001b*/ 0x10040407U,
/*001c*/ 0x18040407U,
/*001d*/ 0x000a0408U,
/*001e*/ 0x10040408U,
/*001f*/ 0xffffffffU,
/*0020*/ 0xffffffffU,
/*0021*/ 0x18070408U,
/*0022*/ 0xffffffffU,
/*0023*/ 0xffffffffU,
/*0024*/ 0xffffffffU,
/*0025*/ 0xffffffffU,
/*0026*/ 0xffffffffU,
/*0027*/ 0xffffffffU,
/*0028*/ 0x000a0409U,
/*0029*/ 0x10040409U,
/*002a*/ 0x18010409U,
/*002b*/ 0x0001040aU,
/*002c*/ 0x0802040aU,
/*002d*/ 0x1009040aU,
/*002e*/ 0x0009040bU,
/*002f*/ 0x1002040bU,
/*0030*/ 0x0020040cU,
/*0031*/ 0xffffffffU,
/*0032*/ 0x0001040dU,
/*0033*/ 0xffffffffU,
/*0034*/ 0xffffffffU,
/*0035*/ 0xffffffffU,
/*0036*/ 0xffffffffU,
/*0037*/ 0x0020040eU,
/*0038*/ 0x0020040fU,
/*0039*/ 0x00200410U,
/*003a*/ 0x00200411U,
/*003b*/ 0x00030412U,
/*003c*/ 0x08010412U,
/*003d*/ 0x10030412U,
/*003e*/ 0x18030412U,
/*003f*/ 0x00040413U,
/*0040*/ 0x08040413U,
/*0041*/ 0x10040413U,
/*0042*/ 0x18040413U,
/*0043*/ 0x00010414U,
/*0044*/ 0x08010414U,
/*0045*/ 0x10060414U,
/*0046*/ 0x18040414U,
/*0047*/ 0xffffffffU,
/*0048*/ 0x00060415U,
/*0049*/ 0x08040415U,
/*004a*/ 0x10060415U,
/*004b*/ 0x18040415U,
/*004c*/ 0x00020416U,
/*004d*/ 0x08050416U,
/*004e*/ 0x10080416U,
/*004f*/ 0x00200417U,
/*0050*/ 0x00060418U,
/*0051*/ 0x08030418U,
/*0052*/ 0x100b0418U,
/*0053*/ 0x00040419U,
/*0054*/ 0x08040419U,
/*0055*/ 0x10040419U,
/*0056*/ 0xffffffffU,
/*0057*/ 0x18010419U,
/*0058*/ 0x0009041aU,
/*0059*/ 0x0020041bU,
/*005a*/ 0x0020041cU,
/*005b*/ 0x0020041dU,
/*005c*/ 0x0020041eU,
/*005d*/ 0x0010041fU,
/*005e*/ 0x00200420U,
/*005f*/ 0x00010421U,
/*0060*/ 0x08060421U,
/*0061*/ 0x10080421U,
/*0062*/ 0x00200422U,
/*0063*/ 0xffffffffU,
/*0064*/ 0x000a0423U,
/*0065*/ 0x10060423U,
/*0066*/ 0x18070423U,
/*0067*/ 0x00080424U,
/*0068*/ 0x08080424U,
/*0069*/ 0x100a0424U,
/*006a*/ 0x00070425U,
/*006b*/ 0x08080425U,
/*006c*/ 0x10080425U,
/*006d*/ 0x18030425U,
/*006e*/ 0x000a0426U,
/*006f*/ 0x100a0426U,
/*0070*/ 0x00110427U,
/*0071*/ 0x00090428U,
/*0072*/ 0x10090428U,
/*0073*/ 0x00100429U,
/*0074*/ 0x100e0429U,
/*0075*/ 0x000e042aU,
/*0076*/ 0x100c042aU,
/*0077*/ 0x000a042bU,
/*0078*/ 0x100a042bU,
/*0079*/ 0x0002042cU,
/*007a*/ 0x0020042dU,
/*007b*/ 0x000b042eU,
/*007c*/ 0x100b042eU,
/*007d*/ 0x0020042fU,
/*007e*/ 0x00120430U,
/*007f*/ 0x00200431U,
/*0080*/ 0x00200432U,
/*0081*/ 0xffffffffU,
/*0082*/ 0xffffffffU,
/*0083*/ 0x00010433U,
/*0084*/ 0x08010433U,
/*0085*/ 0x10080433U,
/*0086*/ 0x000c0434U,
/*0087*/ 0x100c0434U,
/*0088*/ 0x000c0435U,
/*0089*/ 0x100c0435U,
/*008a*/ 0x000c0436U,
/*008b*/ 0x100c0436U,
/*008c*/ 0x000c0437U,
/*008d*/ 0x100c0437U,
/*008e*/ 0x000c0438U,
/*008f*/ 0x100c0438U,
/*0090*/ 0x000c0439U,
/*0091*/ 0x100b0439U,
/*0092*/ 0xffffffffU,
/*0093*/ 0xffffffffU,
/*0094*/ 0x000b043aU,
/*0095*/ 0x100b043aU,
/*0096*/ 0x000b043bU,
/*0097*/ 0x100b043bU,
/*0098*/ 0x000b043cU,
/*0099*/ 0x100b043cU,
/*009a*/ 0x000b043dU,
/*009b*/ 0x100b043dU,
/*009c*/ 0x000b043eU,
/*009d*/ 0x100a043eU,
/*009e*/ 0xffffffffU,
/*009f*/ 0x000a043fU,
/*00a0*/ 0x100a043fU,
/*00a1*/ 0x000a0440U,
/*00a2*/ 0x100a0440U,
/*00a3*/ 0x000a0441U,
/*00a4*/ 0x100a0441U,
/*00a5*/ 0x000a0442U,
/*00a6*/ 0x100a0442U,
/*00a7*/ 0xffffffffU,
/*00a8*/ 0x000a0443U,
/*00a9*/ 0x100a0443U,
/*00aa*/ 0x000a0444U,
/*00ab*/ 0x100a0444U,
/*00ac*/ 0x000a0445U,
/*00ad*/ 0x100a0445U,
/*00ae*/ 0x000a0446U,
/*00af*/ 0x100a0446U,
/*00b0*/ 0x000a0447U,
/*00b1*/ 0x100a0447U,
/*00b2*/ 0x000a0448U,
/*00b3*/ 0x100a0448U,
/*00b4*/ 0x000a0449U,
/*00b5*/ 0x100a0449U,
/*00b6*/ 0x000a044aU,
/*00b7*/ 0x100a044aU,
/*00b8*/ 0x000a044bU,
/*00b9*/ 0x100a044bU,
/*00ba*/ 0x000a044cU,
/*00bb*/ 0x1004044cU,
/*00bc*/ 0x1803044cU,
/*00bd*/ 0x000a044dU,
/*00be*/ 0x100a044dU,
/*00bf*/ 0x0001044eU,
/*00c0*/ 0x080a044eU,
/*00c1*/ 0x1804044eU,
/*00c2*/ 0x000b044fU,
/*00c3*/ 0x100a044fU,
/*00c4*/ 0xffffffffU,
/*00c5*/ 0x00080450U,
/*00c6*/ 0x08080450U,
/*00c7*/ 0x10080450U,
/*00c8*/ 0x18080450U,
/*00c9*/ 0x00080451U,
/*00ca*/ 0xffffffffU,
/*00cb*/ 0x08080451U,
/*00cc*/ 0x10010451U,
/*00cd*/ 0x18080451U,
/*00ce*/ 0x00080452U,
/*00cf*/ 0x08020452U,
/*00d0*/ 0x10020452U,
/*00d1*/ 0x18040452U,
/*00d2*/ 0x00040453U,
/*00d3*/ 0xffffffffU,
/*00d4*/ 0x08040453U,
/*00d5*/ 0x100a0453U,
/*00d6*/ 0x00060454U,
/*00d7*/ 0x08080454U,
/*00d8*/ 0xffffffffU,
/*00d9*/ 0x10040454U,
/*00da*/ 0x18040454U,
/*00db*/ 0x00050455U,
/*00dc*/ 0x08040455U,
/*00dd*/ 0x10050455U,
/*00de*/ 0x000a0456U,
/*00df*/ 0x100a0456U,
/*00e0*/ 0x00080457U,
/*00e1*/ 0xffffffffU,
/*00e2*/ 0x08040457U,
/*00e3*/ 0xffffffffU,
/*00e4*/ 0xffffffffU,
/*00e5*/ 0x00050600U,
/*00e6*/ 0x08050600U,
/*00e7*/ 0x10050600U,
/*00e8*/ 0x18050600U,
/*00e9*/ 0x00050601U,
/*00ea*/ 0x08050601U,
/*00eb*/ 0x100b0601U,
/*00ec*/ 0x00010602U,
/*00ed*/ 0x08030602U,
/*00ee*/ 0x00200603U,
/*00ef*/ 0xffffffffU,
/*00f0*/ 0x00030604U,
/*00f1*/ 0x080a0604U,
/*00f2*/ 0xffffffffU,
/*00f3*/ 0xffffffffU,
/*00f4*/ 0x18030604U,
/*00f5*/ 0x00030605U,
/*00f6*/ 0x08010605U,
/*00f7*/ 0x10010605U,
/*00f8*/ 0x18060605U,
/*00f9*/ 0xffffffffU,
/*00fa*/ 0xffffffffU,
/*00fb*/ 0xffffffffU,
/*00fc*/ 0x00020606U,
/*00fd*/ 0x08030606U,
/*00fe*/ 0x10010606U,
/*00ff*/ 0x000f0607U,
/*0100*/ 0x00200608U,
/*0101*/ 0x00200609U,
/*0102*/ 0x000b060aU,
/*0103*/ 0x100b060aU,
/*0104*/ 0x000b060bU,
/*0105*/ 0xffffffffU,
/*0106*/ 0xffffffffU,
/*0107*/ 0x0018060cU,
/*0108*/ 0x0018060dU,
/*0109*/ 0x0018060eU,
/*010a*/ 0x0018060fU,
/*010b*/ 0x1804060fU,
/*010c*/ 0x00050610U,
/*010d*/ 0x08020610U,
/*010e*/ 0x10040610U,
/*010f*/ 0x18040610U,
/*0110*/ 0x00010611U,
/*0111*/ 0x08010611U,
/*0112*/ 0x10010611U,
/*0113*/ 0x18030611U,
/*0114*/ 0x00200612U,
/*0115*/ 0x00200613U,
/*0116*/ 0x00010614U,
/*0117*/ 0x08140614U,
/*0118*/ 0x00140615U,
/*0119*/ 0x00140616U,
/*011a*/ 0x00140617U,
/*011b*/ 0x00140618U,
/*011c*/ 0x00140619U,
/*011d*/ 0x0014061aU,
/*011e*/ 0x0014061bU,
/*011f*/ 0x0018061cU,
/*0120*/ 0x000a061dU,
/*0121*/ 0x1006061dU,
/*0122*/ 0x1806061dU,
/*0123*/ 0x0006061eU,
/*0124*/ 0xffffffffU,
/*0125*/ 0xffffffffU,
/*0126*/ 0x0008061fU,
/*0127*/ 0x080b061fU,
/*0128*/ 0x000b0620U,
/*0129*/ 0x100b0620U,
/*012a*/ 0x000b0621U,
/*012b*/ 0x100b0621U,
/*012c*/ 0x000b0622U,
/*012d*/ 0x10040622U,
/*012e*/ 0x000a0623U,
/*012f*/ 0x10060623U,
/*0130*/ 0x18080623U,
/*0131*/ 0xffffffffU,
/*0132*/ 0x00040624U,
/*0133*/ 0xffffffffU,
/*0134*/ 0xffffffffU,
/*0135*/ 0x00010700U,
/*0136*/ 0x08020700U,
/*0137*/ 0x10050700U,
/*0138*/ 0x18050700U,
/*0139*/ 0x00050701U,
/*013a*/ 0x08050701U,
/*013b*/ 0x100b0701U,
/*013c*/ 0x00050702U,
/*013d*/ 0x08010702U,
/*013e*/ 0x10010702U,
/*013f*/ 0xffffffffU,
/*0140*/ 0x18010702U,
/*0141*/ 0x00010703U,
/*0142*/ 0x08040703U,
/*0143*/ 0x100b0703U,
/*0144*/ 0x000b0704U,
/*0145*/ 0xffffffffU,
/*0146*/ 0x10040704U,
/*0147*/ 0x000b0705U,
/*0148*/ 0x10040705U,
/*0149*/ 0x18010705U,
/*014a*/ 0x00010706U,
/*014b*/ 0x08010706U,
/*014c*/ 0x00200707U,
/*014d*/ 0x00200708U,
/*014e*/ 0x00080709U,
/*014f*/ 0x080a0709U,
/*0150*/ 0x18050709U,
/*0151*/ 0x000a070aU,
/*0152*/ 0x1003070aU,
/*0153*/ 0x1803070aU,
/*0154*/ 0x0001070bU,
/*0155*/ 0x0802070bU,
/*0156*/ 0x1001070bU,
/*0157*/ 0x1801070bU,
/*0158*/ 0x0001070cU,
/*0159*/ 0x0802070cU,
/*015a*/ 0xffffffffU,
/*015b*/ 0xffffffffU,
/*015c*/ 0xffffffffU,
/*015d*/ 0xffffffffU,
/*015e*/ 0xffffffffU,
/*015f*/ 0xffffffffU,
/*0160*/ 0xffffffffU,
/*0161*/ 0xffffffffU,
/*0162*/ 0xffffffffU,
/*0163*/ 0xffffffffU,
/*0164*/ 0xffffffffU,
/*0165*/ 0xffffffffU,
/*0166*/ 0x1001070cU,
/*0167*/ 0x1801070cU,
/*0168*/ 0x000d070dU,
/*0169*/ 0xffffffffU,
/*016a*/ 0xffffffffU,
/*016b*/ 0x0005070eU,
/*016c*/ 0x0001070fU,
/*016d*/ 0x080e070fU,
/*016e*/ 0x000e0710U,
/*016f*/ 0x100e0710U,
/*0170*/ 0x000e0711U,
/*0171*/ 0x100e0711U,
/*0172*/ 0x00040712U,
/*0173*/ 0xffffffffU,
/*0174*/ 0xffffffffU,
/*0175*/ 0xffffffffU,
/*0176*/ 0xffffffffU,
/*0177*/ 0x080b0712U,
/*0178*/ 0x000b0713U,
/*0179*/ 0x100b0713U,
/*017a*/ 0x000b0714U,
/*017b*/ 0xffffffffU,
/*017c*/ 0xffffffffU,
/*017d*/ 0xffffffffU,
/*017e*/ 0xffffffffU,
/*017f*/ 0x000d0715U,
/*0180*/ 0xffffffffU,
/*0181*/ 0xffffffffU,
/*0182*/ 0x10100715U,
/*0183*/ 0x00080716U,
/*0184*/ 0xffffffffU,
/*0185*/ 0x08100716U,
/*0186*/ 0x00100717U,
/*0187*/ 0x10100717U,
/*0188*/ 0x00100718U,
/*0189*/ 0x10100718U,
/*018a*/ 0x00030719U,
/*018b*/ 0x08040719U,
/*018c*/ 0x10010719U,
/*018d*/ 0x18040719U,
/*018e*/ 0xffffffffU,
/*018f*/ 0xffffffffU,
/*0190*/ 0x0001071aU,
/*0191*/ 0x0812071aU,
/*0192*/ 0x000a071bU,
/*0193*/ 0x100c071bU,
/*0194*/ 0x0012071cU,
/*0195*/ 0x0014071dU,
/*0196*/ 0x0012071eU,
/*0197*/ 0x0011071fU,
/*0198*/ 0x00110720U,
/*0199*/ 0x00120721U,
/*019a*/ 0x00120722U,
/*019b*/ 0x00120723U,
/*019c*/ 0x00120724U,
/*019d*/ 0x00120725U,
/*019e*/ 0x00120726U,
/*019f*/ 0x00120727U,
/*01a0*/ 0x00120728U,
/*01a1*/ 0xffffffffU,
/*01a2*/ 0xffffffffU,
/*01a3*/ 0x00190729U,
/*01a4*/ 0x0019072aU,
/*01a5*/ 0x0020072bU,
/*01a6*/ 0x0017072cU,
/*01a7*/ 0x1808072cU,
/*01a8*/ 0x0001072dU,
/*01a9*/ 0x0801072dU,
/*01aa*/ 0x0020072eU,
/*01ab*/ 0x0008072fU,
/*01ac*/ 0xffffffffU,
/*01ad*/ 0x0803072fU,
/*01ae*/ 0x00180730U,
/*01af*/ 0x00180731U,
/*01b0*/ 0xffffffffU,
/*01b1*/ 0xffffffffU,
/*01b2*/ 0xffffffffU,
/*01b3*/ 0xffffffffU,
/*01b4*/ 0xffffffffU,
/*01b5*/ 0xffffffffU,
/*01b6*/ 0xffffffffU,
/*01b7*/ 0xffffffffU,
/*01b8*/ 0xffffffffU,
/*01b9*/ 0xffffffffU,
/*01ba*/ 0xffffffffU,
/*01bb*/ 0xffffffffU,
/*01bc*/ 0xffffffffU,
/*01bd*/ 0xffffffffU,
/*01be*/ 0xffffffffU,
/*01bf*/ 0x00100732U,
/*01c0*/ 0x10010732U,
/*01c1*/ 0x18010732U,
/*01c2*/ 0x00050733U,
/*01c3*/ 0x00200734U,
/*01c4*/ 0x00090735U,
/*01c5*/ 0xffffffffU,
/*01c6*/ 0xffffffffU,
/*01c7*/ 0x00200736U,
/*01c8*/ 0x00040737U,
/*01c9*/ 0x08100737U,
/*01ca*/ 0x18060737U,
/*01cb*/ 0x00100738U,
/*01cc*/ 0xffffffffU,
/*01cd*/ 0xffffffffU,
/*01ce*/ 0xffffffffU,
/*01cf*/ 0xffffffffU,
/*01d0*/ 0xffffffffU,
/*01d1*/ 0xffffffffU,
/*01d2*/ 0xffffffffU,
/*01d3*/ 0xffffffffU,
/*01d4*/ 0x00200739U,
/*01d5*/ 0x000b073aU,
/*01d6*/ 0xffffffffU,
/*01d7*/ 0xffffffffU,
/*01d8*/ 0xffffffffU,
/*01d9*/ 0xffffffffU,
/*01da*/ 0xffffffffU,
/*01db*/ 0xffffffffU,
/*01dc*/ 0xffffffffU,
/*01dd*/ 0xffffffffU,
/*01de*/ 0x00010200U,
/*01df*/ 0x08040200U,
/*01e0*/ 0x10100200U,
/*01e1*/ 0x00010201U,
/*01e2*/ 0x08010201U,
/*01e3*/ 0xffffffffU,
/*01e4*/ 0xffffffffU,
/*01e5*/ 0x10100201U,
/*01e6*/ 0xffffffffU,
/*01e7*/ 0xffffffffU,
/*01e8*/ 0xffffffffU,
/*01e9*/ 0xffffffffU,
/*01ea*/ 0xffffffffU,
/*01eb*/ 0xffffffffU,
/*01ec*/ 0xffffffffU,
/*01ed*/ 0xffffffffU,
/*01ee*/ 0xffffffffU,
/*01ef*/ 0x00200202U,
/*01f0*/ 0x00100203U,
/*01f1*/ 0x00200204U,
/*01f2*/ 0x00100205U,
/*01f3*/ 0x00200206U,
/*01f4*/ 0x00100207U,
/*01f5*/ 0x10100207U,
/*01f6*/ 0x00200208U,
/*01f7*/ 0x00200209U,
/*01f8*/ 0x0020020aU,
/*01f9*/ 0x0020020bU,
/*01fa*/ 0x0010020cU,
/*01fb*/ 0x0020020dU,
/*01fc*/ 0x0020020eU,
/*01fd*/ 0x0020020fU,
/*01fe*/ 0x00200210U,
/*01ff*/ 0x00100211U,
/*0200*/ 0x00200212U,
/*0201*/ 0x00200213U,
/*0202*/ 0x00200214U,
/*0203*/ 0x00200215U,
/*0204*/ 0x00090216U,
/*0205*/ 0x10010216U,
/*0206*/ 0x00200217U,
/*0207*/ 0x00050218U,
/*0208*/ 0x08010218U,
/*0209*/ 0x10080218U,
/*020a*/ 0x18080218U,
/*020b*/ 0x001c0219U,
/*020c*/ 0x001c021aU,
/*020d*/ 0x001c021bU,
/*020e*/ 0x001c021cU,
/*020f*/ 0x001c021dU,
/*0210*/ 0x001c021eU,
/*0211*/ 0x001c021fU,
/*0212*/ 0x001c0220U,
/*0213*/ 0x001c0221U,
/*0214*/ 0x001c0222U,
/*0215*/ 0x001c0223U,
/*0216*/ 0x001c0224U,
/*0217*/ 0x001c0225U,
/*0218*/ 0x001c0226U,
/*0219*/ 0x001c0227U,
/*021a*/ 0x001c0228U,
/*021b*/ 0x00010229U,
/*021c*/ 0x08010229U,
/*021d*/ 0x10010229U,
/*021e*/ 0x18040229U,
/*021f*/ 0x0008022aU,
/*0220*/ 0x0808022aU,
/*0221*/ 0x1008022aU,
/*0222*/ 0x1804022aU,
/*0223*/ 0x0006022bU,
/*0224*/ 0xffffffffU,
/*0225*/ 0x0807022bU,
/*0226*/ 0x1006022bU,
/*0227*/ 0xffffffffU,
/*0228*/ 0x1807022bU,
/*0229*/ 0x0006022cU,
/*022a*/ 0xffffffffU,
/*022b*/ 0x0807022cU,
/*022c*/ 0x1002022cU,
/*022d*/ 0x1801022cU,
/*022e*/ 0xffffffffU,
/*022f*/ 0x000a022dU,
/*0230*/ 0x1010022dU,
/*0231*/ 0x000a022eU,
/*0232*/ 0x1010022eU,
/*0233*/ 0x000a022fU,
/*0234*/ 0x1010022fU,
/*0235*/ 0xffffffffU,
/*0236*/ 0x00100230U,
/*0237*/ 0xffffffffU,
/*0238*/ 0xffffffffU,
/*0239*/ 0x10010230U,
/*023a*/ 0x18010230U,
/*023b*/ 0x00010231U,
/*023c*/ 0x08010231U,
/*023d*/ 0x10010231U,
/*023e*/ 0x18010231U,
/*023f*/ 0x00020232U,
/*0240*/ 0x08020232U,
/*0241*/ 0x10020232U,
/*0242*/ 0x18020232U,
/*0243*/ 0x00020233U,
/*0244*/ 0x08030233U,
/*0245*/ 0x10010233U,
/*0246*/ 0x18010233U,
/*0247*/ 0x00010234U,
/*0248*/ 0x08010234U,
/*0249*/ 0xffffffffU,
/*024a*/ 0x10020234U,
/*024b*/ 0x18010234U,
/*024c*/ 0x00010235U,
/*024d*/ 0xffffffffU,
/*024e*/ 0x08020235U,
/*024f*/ 0x10010235U,
/*0250*/ 0x18010235U,
/*0251*/ 0xffffffffU,
/*0252*/ 0x00020236U,
/*0253*/ 0x08010236U,
/*0254*/ 0x10010236U,
/*0255*/ 0xffffffffU,
/*0256*/ 0x18020236U,
/*0257*/ 0x00070237U,
/*0258*/ 0x08010237U,
/*0259*/ 0x10010237U,
/*025a*/ 0x18010237U,
/*025b*/ 0x00010238U,
/*025c*/ 0x08010238U,
/*025d*/ 0x10010238U,
/*025e*/ 0xffffffffU,
/*025f*/ 0x18010238U,
/*0260*/ 0x00040239U,
/*0261*/ 0x08040239U,
/*0262*/ 0x10040239U,
/*0263*/ 0x18010239U,
/*0264*/ 0x0002023aU,
/*0265*/ 0x0806023aU,
/*0266*/ 0x1006023aU,
/*0267*/ 0xffffffffU,
/*0268*/ 0xffffffffU,
/*0269*/ 0xffffffffU,
/*026a*/ 0x1802023aU,
/*026b*/ 0x0010023bU,
/*026c*/ 0x1001023bU,
/*026d*/ 0x1801023bU,
/*026e*/ 0xffffffffU,
/*026f*/ 0x0004023cU,
/*0270*/ 0x0801023cU,
/*0271*/ 0x1004023cU,
/*0272*/ 0x1802023cU,
/*0273*/ 0x0008023dU,
/*0274*/ 0xffffffffU,
/*0275*/ 0xffffffffU,
/*0276*/ 0xffffffffU,
/*0277*/ 0x080a023dU,
/*0278*/ 0x0020023eU,
/*0279*/ 0x0020023fU,
/*027a*/ 0x00050240U,
/*027b*/ 0x08010240U,
/*027c*/ 0x10050240U,
/*027d*/ 0x18080240U,
/*027e*/ 0x00010241U,
/*027f*/ 0x08080241U,
/*0280*/ 0x10010241U,
/*0281*/ 0x18080241U,
/*0282*/ 0x00010242U,
/*0283*/ 0x08040242U,
/*0284*/ 0x10040242U,
/*0285*/ 0x18040242U,
/*0286*/ 0x00040243U,
/*0287*/ 0x08040243U,
/*0288*/ 0x10040243U,
/*0289*/ 0x18040243U,
/*028a*/ 0x00040244U,
/*028b*/ 0x08040244U,
/*028c*/ 0x10040244U,
/*028d*/ 0x18010244U,
/*028e*/ 0x00040245U,
/*028f*/ 0x08040245U,
/*0290*/ 0x10040245U,
/*0291*/ 0x18040245U,
/*0292*/ 0x00040246U,
/*0293*/ 0x08040246U,
/*0294*/ 0x10060246U,
/*0295*/ 0x18060246U,
/*0296*/ 0x00060247U,
/*0297*/ 0x08060247U,
/*0298*/ 0x10060247U,
/*0299*/ 0x18060247U,
/*029a*/ 0xffffffffU,
/*029b*/ 0x00010248U,
/*029c*/ 0x08010248U,
/*029d*/ 0x10020248U,
/*029e*/ 0xffffffffU,
/*029f*/ 0xffffffffU,
/*02a0*/ 0xffffffffU,
/*02a1*/ 0xffffffffU,
/*02a2*/ 0xffffffffU,
/*02a3*/ 0xffffffffU,
/*02a4*/ 0xffffffffU,
/*02a5*/ 0xffffffffU,
/*02a6*/ 0x18040248U,
/*02a7*/ 0x00040249U,
/*02a8*/ 0x08010249U,
/*02a9*/ 0x10010249U,
/*02aa*/ 0xffffffffU,
/*02ab*/ 0x18010249U,
/*02ac*/ 0x0001024aU,
/*02ad*/ 0xffffffffU,
/*02ae*/ 0x0801024aU,
/*02af*/ 0x1001024aU,
/*02b0*/ 0x1801024aU,
/*02b1*/ 0x0004024bU,
/*02b2*/ 0x0804024bU,
/*02b3*/ 0x100a024bU,
/*02b4*/ 0x0020024cU,
/*02b5*/ 0x0004024dU,
/*02b6*/ 0x0808024dU,
/*02b7*/ 0xffffffffU,
/*02b8*/ 0xffffffffU,
/*02b9*/ 0xffffffffU,
/*02ba*/ 0xffffffffU,
/*02bb*/ 0xffffffffU,
/*02bc*/ 0xffffffffU,
/*02bd*/ 0x1002024dU,
/*02be*/ 0x1802024dU,
/*02bf*/ 0x0020024eU,
/*02c0*/ 0x0002024fU,
/*02c1*/ 0x0810024fU,
/*02c2*/ 0x00100250U,
/*02c3*/ 0x10040250U,
/*02c4*/ 0x18040250U,
/*02c5*/ 0x00050251U,
/*02c6*/ 0x08050251U,
/*02c7*/ 0xffffffffU,
/*02c8*/ 0xffffffffU,
/*02c9*/ 0xffffffffU,
/*02ca*/ 0xffffffffU,
/*02cb*/ 0x10010251U,
/*02cc*/ 0x18010251U,
/*02cd*/ 0x00070252U,
/*02ce*/ 0x08070252U,
/*02cf*/ 0x10070252U,
/*02d0*/ 0x18070252U,
/*02d1*/ 0x00070253U,
/*02d2*/ 0x08070253U,
/*02d3*/ 0x10070253U,
/*02d4*/ 0x18070253U,
/*02d5*/ 0x00070254U,
/*02d6*/ 0x08070254U,
/*02d7*/ 0x10070254U,
/*02d8*/ 0xffffffffU,
/*02d9*/ 0xffffffffU,
/*02da*/ 0xffffffffU,
/*02db*/ 0xffffffffU,
/*02dc*/ 0xffffffffU,
/*02dd*/ 0xffffffffU,
/*02de*/ 0x18030254U,
/*02df*/ 0x00010255U,
/*02e0*/ 0x08020255U,
/*02e1*/ 0x10010255U,
/*02e2*/ 0x18040255U,
/*02e3*/ 0x00020256U,
/*02e4*/ 0x08010256U,
/*02e5*/ 0x10010256U,
/*02e6*/ 0xffffffffU,
/*02e7*/ 0x18010256U,
/*02e8*/ 0x00040257U,
/*02e9*/ 0x08080257U,
/*02ea*/ 0x100a0257U,
/*02eb*/ 0x000a0258U,
/*02ec*/ 0x100a0258U,
/*02ed*/ 0x000a0259U,
/*02ee*/ 0x100a0259U,
/*02ef*/ 0x000a025aU,
/*02f0*/ 0x0020025bU,
/*02f1*/ 0x0020025cU,
/*02f2*/ 0x0001025dU,
/*02f3*/ 0xffffffffU,
/*02f4*/ 0xffffffffU,
/*02f5*/ 0xffffffffU,
/*02f6*/ 0x0802025dU,
/*02f7*/ 0x1002025dU,
/*02f8*/ 0x0010025eU,
/*02f9*/ 0x1005025eU,
/*02fa*/ 0x1806025eU,
/*02fb*/ 0x0005025fU,
/*02fc*/ 0x0805025fU,
/*02fd*/ 0x100e025fU,
/*02fe*/ 0x00050260U,
/*02ff*/ 0x080e0260U,
/*0300*/ 0x18050260U,
/*0301*/ 0x000e0261U,
/*0302*/ 0x10050261U,
/*0303*/ 0x18010261U,
/*0304*/ 0x00050262U,
/*0305*/ 0x08050262U,
/*0306*/ 0x100a0262U,
/*0307*/ 0x000a0263U,
/*0308*/ 0x10050263U,
/*0309*/ 0x18050263U,
/*030a*/ 0x000a0264U,
/*030b*/ 0x100a0264U,
/*030c*/ 0x00050265U,
/*030d*/ 0x08050265U,
/*030e*/ 0x100a0265U,
/*030f*/ 0x000a0266U,
/*0310*/ 0xffffffffU,
/*0311*/ 0xffffffffU,
/*0312*/ 0xffffffffU,
/*0313*/ 0xffffffffU,
/*0314*/ 0xffffffffU,
/*0315*/ 0xffffffffU,
/*0316*/ 0x10070266U,
/*0317*/ 0x18070266U,
/*0318*/ 0x00040267U,
/*0319*/ 0x08040267U,
/*031a*/ 0xffffffffU,
/*031b*/ 0xffffffffU,
/*031c*/ 0xffffffffU,
/*031d*/ 0x10040267U,
/*031e*/ 0x18080267U,
/*031f*/ 0x00080268U,
/*0320*/ 0x08040268U,
/*0321*/ 0xffffffffU,
/*0322*/ 0xffffffffU,
/*0323*/ 0xffffffffU,
/*0324*/ 0x10040268U,
/*0325*/ 0xffffffffU,
/*0326*/ 0xffffffffU,
/*0327*/ 0xffffffffU,
/*0328*/ 0x18040268U,
/*0329*/ 0xffffffffU,
/*032a*/ 0xffffffffU,
/*032b*/ 0xffffffffU,
/*032c*/ 0x00040269U,
/*032d*/ 0x08050269U,
/*032e*/ 0x10070269U,
/*032f*/ 0x18080269U,
/*0330*/ 0x0010026aU,
/*0331*/ 0x1008026aU,
/*0332*/ 0x0010026bU,
/*0333*/ 0x1008026bU,
/*0334*/ 0x0010026cU,
/*0335*/ 0x1008026cU,
/*0336*/ 0x1808026cU,
/*0337*/ 0x0001026dU,
/*0338*/ 0x0801026dU,
/*0339*/ 0x1006026dU,
/*033a*/ 0x1806026dU,
/*033b*/ 0x0006026eU,
/*033c*/ 0xffffffffU,
/*033d*/ 0x0801026eU,
/*033e*/ 0x1003026eU,
/*033f*/ 0xffffffffU,
/*0340*/ 0xffffffffU,
/*0341*/ 0xffffffffU,
/*0342*/ 0x000a026fU,
/*0343*/ 0x100a026fU,
/*0344*/ 0x00040270U,
/*0345*/ 0x08010270U,
/*0346*/ 0x10040270U,
/*0347*/ 0xffffffffU,
/*0348*/ 0xffffffffU,
/*0349*/ 0xffffffffU,
/*034a*/ 0xffffffffU,
/*034b*/ 0xffffffffU,
/*034c*/ 0xffffffffU,
/*034d*/ 0x18070270U,
/*034e*/ 0x00070271U,
/*034f*/ 0x08050271U,
/*0350*/ 0x10050271U,
/*0351*/ 0xffffffffU,
/*0352*/ 0xffffffffU,
/*0353*/ 0xffffffffU,
/*0354*/ 0x18040271U,
/*0355*/ 0x00010272U,
/*0356*/ 0x08010272U,
/*0357*/ 0x10020272U,
/*0358*/ 0x18080272U,
/*0359*/ 0x00200273U,
/*035a*/ 0x00200274U,
/*035b*/ 0x00100275U,
/*035c*/ 0xffffffffU,
/*035d*/ 0xffffffffU,
/*035e*/ 0xffffffffU,
/*035f*/ 0x10020275U,
/*0360*/ 0x18010275U,
/*0361*/ 0xffffffffU,
/*0362*/ 0x00020276U,
/*0363*/ 0x08080276U,
/*0364*/ 0x10080276U,
/*0365*/ 0x18080276U,
/*0366*/ 0x00080277U,
/*0367*/ 0x08080277U,
/*0368*/ 0x10080277U,
/*0369*/ 0xffffffffU,
/*036a*/ 0x18080277U,
/*036b*/ 0x00080278U,
/*036c*/ 0x08080278U,
/*036d*/ 0x10080278U,
/*036e*/ 0x18080278U,
/*036f*/ 0x00080279U,
/*0370*/ 0xffffffffU,
/*0371*/ 0x08080279U,
/*0372*/ 0x10080279U,
/*0373*/ 0x18080279U,
/*0374*/ 0x0008027aU,
/*0375*/ 0x0808027aU,
/*0376*/ 0x1008027aU,
/*0377*/ 0xffffffffU,
/*0378*/ 0x1808027aU,
/*0379*/ 0x0008027bU,
/*037a*/ 0x0808027bU,
/*037b*/ 0x1008027bU,
/*037c*/ 0x1808027bU,
/*037d*/ 0x0008027cU,
/*037e*/ 0x0808027cU,
/*037f*/ 0xffffffffU,
/*0380*/ 0x1008027cU,
/*0381*/ 0x1808027cU,
/*0382*/ 0x0008027dU,
/*0383*/ 0x0808027dU,
/*0384*/ 0x1008027dU,
/*0385*/ 0x1808027dU,
/*0386*/ 0xffffffffU,
/*0387*/ 0x0008027eU,
/*0388*/ 0x0808027eU,
/*0389*/ 0x1008027eU,
/*038a*/ 0x1808027eU,
/*038b*/ 0x0008027fU,
/*038c*/ 0x0808027fU,
/*038d*/ 0xffffffffU,
/*038e*/ 0x1008027fU,
/*038f*/ 0x1808027fU,
/*0390*/ 0x00080280U,
/*0391*/ 0x08080280U,
/*0392*/ 0x10080280U,
/*0393*/ 0x18080280U,
/*0394*/ 0x00080281U,
/*0395*/ 0xffffffffU,
/*0396*/ 0x08080281U,
/*0397*/ 0x10080281U,
/*0398*/ 0x18080281U,
/*0399*/ 0x00080282U,
/*039a*/ 0x08080282U,
/*039b*/ 0x10080282U,
/*039c*/ 0xffffffffU,
/*039d*/ 0x18080282U,
/*039e*/ 0x00080283U,
/*039f*/ 0x08080283U,
/*03a0*/ 0x10080283U,
/*03a1*/ 0x18080283U,
/*03a2*/ 0x00080284U,
/*03a3*/ 0xffffffffU,
/*03a4*/ 0x08080284U,
/*03a5*/ 0x10080284U,
/*03a6*/ 0x18080284U,
/*03a7*/ 0x00080285U,
/*03a8*/ 0x08080285U,
/*03a9*/ 0x10080285U,
/*03aa*/ 0x18080285U,
/*03ab*/ 0xffffffffU,
/*03ac*/ 0x00080286U,
/*03ad*/ 0x08080286U,
/*03ae*/ 0x10080286U,
/*03af*/ 0x18080286U,
/*03b0*/ 0x00080287U,
/*03b1*/ 0x08080287U,
/*03b2*/ 0xffffffffU,
/*03b3*/ 0x10080287U,
/*03b4*/ 0x18080287U,
/*03b5*/ 0x00080288U,
/*03b6*/ 0x08080288U,
/*03b7*/ 0x10080288U,
/*03b8*/ 0x18080288U,
/*03b9*/ 0xffffffffU,
/*03ba*/ 0x00080289U,
/*03bb*/ 0x08020289U,
/*03bc*/ 0x10030289U,
/*03bd*/ 0x000a028aU,
/*03be*/ 0x100a028aU,
/*03bf*/ 0x000a028bU,
/*03c0*/ 0x1005028bU,
/*03c1*/ 0x1804028bU,
/*03c2*/ 0x0008028cU,
/*03c3*/ 0x0808028cU,
/*03c4*/ 0x1006028cU,
/*03c5*/ 0x1806028cU,
/*03c6*/ 0x0011028dU,
/*03c7*/ 0x1808028dU,
/*03c8*/ 0x0004028eU,
/*03c9*/ 0x0806028eU,
/*03ca*/ 0xffffffffU,
/*03cb*/ 0x1006028eU,
/*03cc*/ 0x1808028eU,
/*03cd*/ 0xffffffffU,
/*03ce*/ 0x0004028fU,
/*03cf*/ 0x0808028fU,
/*03d0*/ 0x1008028fU,
/*03d1*/ 0x1806028fU,
/*03d2*/ 0x00060290U,
/*03d3*/ 0x08110290U,
/*03d4*/ 0x00080291U,
/*03d5*/ 0x08040291U,
/*03d6*/ 0x10060291U,
/*03d7*/ 0xffffffffU,
/*03d8*/ 0x18060291U,
/*03d9*/ 0x00080292U,
/*03da*/ 0xffffffffU,
/*03db*/ 0x08040292U,
/*03dc*/ 0x10080292U,
/*03dd*/ 0x18080292U,
/*03de*/ 0x00060293U,
/*03df*/ 0x08060293U,
/*03e0*/ 0x00110294U,
/*03e1*/ 0x18080294U,
/*03e2*/ 0x00040295U,
/*03e3*/ 0x08060295U,
/*03e4*/ 0xffffffffU,
/*03e5*/ 0x10060295U,
/*03e6*/ 0x18080295U,
/*03e7*/ 0xffffffffU,
/*03e8*/ 0x00040296U,
/*03e9*/ 0x08040296U,
/*03ea*/ 0x10040296U,
/*03eb*/ 0x18040296U,
/*03ec*/ 0x00040297U,
/*03ed*/ 0x08040297U,
/*03ee*/ 0x10040297U,
/*03ef*/ 0x18040297U,
/*03f0*/ 0x00040298U,
/*03f1*/ 0x08040298U,
/*03f2*/ 0x10040298U,
/*03f3*/ 0x18040298U,
/*03f4*/ 0x00040299U,
/*03f5*/ 0x08040299U,
/*03f6*/ 0x10040299U,
/*03f7*/ 0x18040299U,
/*03f8*/ 0x0004029aU,
/*03f9*/ 0x0804029aU,
/*03fa*/ 0x1004029aU,
/*03fb*/ 0x1804029aU,
/*03fc*/ 0x0011029bU,
/*03fd*/ 0x0010029cU,
/*03fe*/ 0x0011029dU,
/*03ff*/ 0x0020029eU,
/*0400*/ 0x0020029fU,
/*0401*/ 0x002002a0U,
/*0402*/ 0x002002a1U,
/*0403*/ 0x002002a2U,
/*0404*/ 0x002002a3U,
/*0405*/ 0x002002a4U,
/*0406*/ 0x002002a5U,
/*0407*/ 0x002002a6U,
/*0408*/ 0x000202a7U,
/*0409*/ 0x080502a7U,
/*040a*/ 0x100502a7U,
/*040b*/ 0xffffffffU,
/*040c*/ 0xffffffffU,
/*040d*/ 0xffffffffU,
/*040e*/ 0xffffffffU,
/*040f*/ 0xffffffffU,
/*0410*/ 0xffffffffU,
/*0411*/ 0xffffffffU,
/*0412*/ 0xffffffffU,
/*0413*/ 0xffffffffU,
/*0414*/ 0xffffffffU,
/*0415*/ 0xffffffffU,
/*0416*/ 0xffffffffU,
/*0417*/ 0xffffffffU,
/*0418*/ 0xffffffffU,
/*0419*/ 0xffffffffU,
/*041a*/ 0xffffffffU,
/*041b*/ 0xffffffffU,
/*041c*/ 0xffffffffU,
/*041d*/ 0xffffffffU,
/*041e*/ 0xffffffffU,
/*041f*/ 0xffffffffU,
/*0420*/ 0xffffffffU,
/*0421*/ 0xffffffffU,
/*0422*/ 0xffffffffU,
/*0423*/ 0xffffffffU,
/*0424*/ 0xffffffffU,
/*0425*/ 0xffffffffU,
/*0426*/ 0xffffffffU,
/*0427*/ 0x180102a7U,
/*0428*/ 0x000402a8U,
/*0429*/ 0x081002a8U,
/*042a*/ 0x002002a9U,
/*042b*/ 0x001002aaU,
/*042c*/ 0x002002abU,
/*042d*/ 0x001002acU,
/*042e*/ 0x002002adU,
/*042f*/ 0x000702aeU,
/*0430*/ 0x080102aeU,
/*0431*/ 0x100202aeU,
/*0432*/ 0x180602aeU,
/*0433*/ 0x000102afU,
/*0434*/ 0x080102afU,
/*0435*/ 0x002002b0U,
/*0436*/ 0x000202b1U,
/*0437*/ 0x002002b2U,
/*0438*/ 0x002002b3U,
/*0439*/ 0xffffffffU,
/*043a*/ 0xffffffffU,
/*043b*/ 0xffffffffU,
/*043c*/ 0xffffffffU,
/*043d*/ 0xffffffffU,
/*043e*/ 0xffffffffU,
/*043f*/ 0xffffffffU,
/*0440*/ 0xffffffffU,
/*0441*/ 0xffffffffU,
/*0442*/ 0xffffffffU,
/*0443*/ 0xffffffffU,
/*0444*/ 0xffffffffU,
/*0445*/ 0xffffffffU,
/*0446*/ 0xffffffffU,
/*0447*/ 0xffffffffU,
/*0448*/ 0xffffffffU,
/*0449*/ 0xffffffffU,
/*044a*/ 0xffffffffU,
/*044b*/ 0xffffffffU,
/*044c*/ 0xffffffffU,
/*044d*/ 0xffffffffU,
/*044e*/ 0xffffffffU,
/*044f*/ 0xffffffffU,
/*0450*/ 0xffffffffU,
/*0451*/ 0xffffffffU,
/*0452*/ 0xffffffffU,
/*0453*/ 0xffffffffU,
/*0454*/ 0xffffffffU,
/*0455*/ 0xffffffffU,
/*0456*/ 0xffffffffU,
/*0457*/ 0xffffffffU,
/*0458*/ 0xffffffffU,
/*0459*/ 0xffffffffU,
/*045a*/ 0xffffffffU,
/*045b*/ 0xffffffffU,
/*045c*/ 0xffffffffU,
/*045d*/ 0xffffffffU,
/*045e*/ 0xffffffffU,
/*045f*/ 0x000402b4U,
/*0460*/ 0xffffffffU,
/*0461*/ 0xffffffffU,
/*0462*/ 0xffffffffU,
/*0463*/ 0xffffffffU,
/*0464*/ 0xffffffffU,
/*0465*/ 0xffffffffU,
/*0466*/ 0xffffffffU,
/*0467*/ 0xffffffffU,
/*0468*/ 0xffffffffU,
/*0469*/ 0xffffffffU,
/*046a*/ 0xffffffffU,
/*046b*/ 0xffffffffU,
/*046c*/ 0xffffffffU,
/*046d*/ 0xffffffffU,
/*046e*/ 0xffffffffU,
/*046f*/ 0xffffffffU,
/*0470*/ 0xffffffffU,
/*0471*/ 0xffffffffU,
/*0472*/ 0xffffffffU,
/*0473*/ 0xffffffffU,
/*0474*/ 0xffffffffU,
/*0475*/ 0xffffffffU,
/*0476*/ 0xffffffffU,
/*0477*/ 0xffffffffU,
/*0478*/ 0xffffffffU,
/*0479*/ 0xffffffffU,
/*047a*/ 0xffffffffU,
/*047b*/ 0xffffffffU,
/*047c*/ 0xffffffffU,
/*047d*/ 0xffffffffU,
/*047e*/ 0xffffffffU,
/*047f*/ 0xffffffffU,
/*0480*/ 0xffffffffU,
/*0481*/ 0xffffffffU,
/*0482*/ 0xffffffffU,
/*0483*/ 0xffffffffU,
/*0484*/ 0xffffffffU,
/*0485*/ 0xffffffffU,
/*0486*/ 0xffffffffU,
/*0487*/ 0xffffffffU,
/*0488*/ 0xffffffffU,
/*0489*/ 0xffffffffU,
/*048a*/ 0xffffffffU,
/*048b*/ 0xffffffffU,
/*048c*/ 0xffffffffU,
/*048d*/ 0xffffffffU,
/*048e*/ 0xffffffffU,
/*048f*/ 0xffffffffU,
/*0490*/ 0xffffffffU,
/*0491*/ 0xffffffffU,
/*0492*/ 0xffffffffU,
/*0493*/ 0xffffffffU,
/*0494*/ 0xffffffffU,
	 },
	{
/*0000*/ 0x00200800U,
/*0001*/ 0x00040801U,
/*0002*/ 0x080b0801U,
/*0003*/ 0xffffffffU,
/*0004*/ 0xffffffffU,
/*0005*/ 0x18010801U,
/*0006*/ 0x00050802U,
/*0007*/ 0x08050802U,
/*0008*/ 0x10050802U,
/*0009*/ 0x18050802U,
/*000a*/ 0x00050803U,
/*000b*/ 0x08050803U,
/*000c*/ 0x10050803U,
/*000d*/ 0x18050803U,
/*000e*/ 0x00050804U,
/*000f*/ 0x08040804U,
/*0010*/ 0x10030804U,
/*0011*/ 0x00180805U,
/*0012*/ 0x18030805U,
/*0013*/ 0x00180806U,
/*0014*/ 0x18020806U,
/*0015*/ 0x00010807U,
/*0016*/ 0x08020807U,
/*0017*/ 0x10010807U,
/*0018*/ 0x18010807U,
/*0019*/ 0x00020808U,
/*001a*/ 0x08040808U,
/*001b*/ 0x10040808U,
/*001c*/ 0x18040808U,
/*001d*/ 0x000a0809U,
/*001e*/ 0x10040809U,
/*001f*/ 0xffffffffU,
/*0020*/ 0xffffffffU,
/*0021*/ 0x18070809U,
/*0022*/ 0xffffffffU,
/*0023*/ 0xffffffffU,
/*0024*/ 0xffffffffU,
/*0025*/ 0xffffffffU,
/*0026*/ 0xffffffffU,
/*0027*/ 0xffffffffU,
/*0028*/ 0x000a080aU,
/*0029*/ 0x1005080aU,
/*002a*/ 0x1801080aU,
/*002b*/ 0x0001080bU,
/*002c*/ 0x0802080bU,
/*002d*/ 0x1009080bU,
/*002e*/ 0x0009080cU,
/*002f*/ 0x1002080cU,
/*0030*/ 0x0020080dU,
/*0031*/ 0xffffffffU,
/*0032*/ 0x0001080eU,
/*0033*/ 0xffffffffU,
/*0034*/ 0xffffffffU,
/*0035*/ 0xffffffffU,
/*0036*/ 0xffffffffU,
/*0037*/ 0x0020080fU,
/*0038*/ 0x00200810U,
/*0039*/ 0x00200811U,
/*003a*/ 0x00200812U,
/*003b*/ 0x00030813U,
/*003c*/ 0x08010813U,
/*003d*/ 0x10030813U,
/*003e*/ 0x18030813U,
/*003f*/ 0x00040814U,
/*0040*/ 0x08040814U,
/*0041*/ 0x10040814U,
/*0042*/ 0x18040814U,
/*0043*/ 0x00010815U,
/*0044*/ 0x08010815U,
/*0045*/ 0x10060815U,
/*0046*/ 0x18040815U,
/*0047*/ 0xffffffffU,
/*0048*/ 0x00060816U,
/*0049*/ 0x08040816U,
/*004a*/ 0x10060816U,
/*004b*/ 0x18040816U,
/*004c*/ 0x00020817U,
/*004d*/ 0x08050817U,
/*004e*/ 0x10080817U,
/*004f*/ 0x00200818U,
/*0050*/ 0x00060819U,
/*0051*/ 0x08030819U,
/*0052*/ 0x100b0819U,
/*0053*/ 0x0004081aU,
/*0054*/ 0x0804081aU,
/*0055*/ 0x1004081aU,
/*0056*/ 0xffffffffU,
/*0057*/ 0x1801081aU,
/*0058*/ 0x0009081bU,
/*0059*/ 0x0020081cU,
/*005a*/ 0x0020081dU,
/*005b*/ 0x0020081eU,
/*005c*/ 0x0020081fU,
/*005d*/ 0x00100820U,
/*005e*/ 0xffffffffU,
/*005f*/ 0x10010820U,
/*0060*/ 0x18060820U,
/*0061*/ 0x00080821U,
/*0062*/ 0x00200822U,
/*0063*/ 0xffffffffU,
/*0064*/ 0x000a0823U,
/*0065*/ 0x10060823U,
/*0066*/ 0x18070823U,
/*0067*/ 0x00080824U,
/*0068*/ 0x08080824U,
/*0069*/ 0x100a0824U,
/*006a*/ 0x00070825U,
/*006b*/ 0x08080825U,
/*006c*/ 0x10080825U,
/*006d*/ 0x18030825U,
/*006e*/ 0x000a0826U,
/*006f*/ 0x100a0826U,
/*0070*/ 0x00110827U,
/*0071*/ 0x00090828U,
/*0072*/ 0x10090828U,
/*0073*/ 0x00100829U,
/*0074*/ 0x100e0829U,
/*0075*/ 0x000e082aU,
/*0076*/ 0x100c082aU,
/*0077*/ 0x000a082bU,
/*0078*/ 0x100a082bU,
/*0079*/ 0x0002082cU,
/*007a*/ 0x0020082dU,
/*007b*/ 0x000b082eU,
/*007c*/ 0x100b082eU,
/*007d*/ 0x0020082fU,
/*007e*/ 0x00120830U,
/*007f*/ 0x00200831U,
/*0080*/ 0x00200832U,
/*0081*/ 0xffffffffU,
/*0082*/ 0xffffffffU,
/*0083*/ 0x00010833U,
/*0084*/ 0x08010833U,
/*0085*/ 0x10080833U,
/*0086*/ 0x000c0834U,
/*0087*/ 0x100c0834U,
/*0088*/ 0x000c0835U,
/*0089*/ 0x100c0835U,
/*008a*/ 0x000c0836U,
/*008b*/ 0x100c0836U,
/*008c*/ 0x000c0837U,
/*008d*/ 0x100c0837U,
/*008e*/ 0x000c0838U,
/*008f*/ 0x100c0838U,
/*0090*/ 0x000c0839U,
/*0091*/ 0x100b0839U,
/*0092*/ 0xffffffffU,
/*0093*/ 0xffffffffU,
/*0094*/ 0x000b083aU,
/*0095*/ 0x100b083aU,
/*0096*/ 0x000b083bU,
/*0097*/ 0x100b083bU,
/*0098*/ 0x000b083cU,
/*0099*/ 0x100b083cU,
/*009a*/ 0x000b083dU,
/*009b*/ 0x100b083dU,
/*009c*/ 0x000b083eU,
/*009d*/ 0x100a083eU,
/*009e*/ 0xffffffffU,
/*009f*/ 0x000a083fU,
/*00a0*/ 0x100a083fU,
/*00a1*/ 0x000a0840U,
/*00a2*/ 0x100a0840U,
/*00a3*/ 0x000a0841U,
/*00a4*/ 0x100a0841U,
/*00a5*/ 0x000a0842U,
/*00a6*/ 0x100a0842U,
/*00a7*/ 0x000a0843U,
/*00a8*/ 0x100a0843U,
/*00a9*/ 0x000a0844U,
/*00aa*/ 0x100a0844U,
/*00ab*/ 0x000a0845U,
/*00ac*/ 0x100a0845U,
/*00ad*/ 0x000a0846U,
/*00ae*/ 0x100a0846U,
/*00af*/ 0x000a0847U,
/*00b0*/ 0x100a0847U,
/*00b1*/ 0x000a0848U,
/*00b2*/ 0x100a0848U,
/*00b3*/ 0x000a0849U,
/*00b4*/ 0x100a0849U,
/*00b5*/ 0x000a084aU,
/*00b6*/ 0x100a084aU,
/*00b7*/ 0x000a084bU,
/*00b8*/ 0x100a084bU,
/*00b9*/ 0x000a084cU,
/*00ba*/ 0x100a084cU,
/*00bb*/ 0x0004084dU,
/*00bc*/ 0x0803084dU,
/*00bd*/ 0x100a084dU,
/*00be*/ 0x000a084eU,
/*00bf*/ 0x1001084eU,
/*00c0*/ 0x000a084fU,
/*00c1*/ 0x1004084fU,
/*00c2*/ 0x000b0850U,
/*00c3*/ 0x100a0850U,
/*00c4*/ 0xffffffffU,
/*00c5*/ 0x00080851U,
/*00c6*/ 0x08080851U,
/*00c7*/ 0x10080851U,
/*00c8*/ 0x18080851U,
/*00c9*/ 0x00080852U,
/*00ca*/ 0xffffffffU,
/*00cb*/ 0x08080852U,
/*00cc*/ 0x10010852U,
/*00cd*/ 0x18080852U,
/*00ce*/ 0x00080853U,
/*00cf*/ 0x08020853U,
/*00d0*/ 0x10020853U,
/*00d1*/ 0x18040853U,
/*00d2*/ 0x00040854U,
/*00d3*/ 0xffffffffU,
/*00d4*/ 0x08040854U,
/*00d5*/ 0x100a0854U,
/*00d6*/ 0x00060855U,
/*00d7*/ 0x08080855U,
/*00d8*/ 0xffffffffU,
/*00d9*/ 0x10040855U,
/*00da*/ 0x18040855U,
/*00db*/ 0x00050856U,
/*00dc*/ 0x08040856U,
/*00dd*/ 0x10050856U,
/*00de*/ 0x000a0857U,
/*00df*/ 0x100a0857U,
/*00e0*/ 0x00080858U,
/*00e1*/ 0xffffffffU,
/*00e2*/ 0x08040858U,
/*00e3*/ 0xffffffffU,
/*00e4*/ 0xffffffffU,
/*00e5*/ 0x00050a00U,
/*00e6*/ 0x08050a00U,
/*00e7*/ 0x10050a00U,
/*00e8*/ 0x18050a00U,
/*00e9*/ 0x00050a01U,
/*00ea*/ 0x08050a01U,
/*00eb*/ 0x100b0a01U,
/*00ec*/ 0x00010a02U,
/*00ed*/ 0x08030a02U,
/*00ee*/ 0x00200a03U,
/*00ef*/ 0xffffffffU,
/*00f0*/ 0x00030a04U,
/*00f1*/ 0x080a0a04U,
/*00f2*/ 0xffffffffU,
/*00f3*/ 0xffffffffU,
/*00f4*/ 0x18030a04U,
/*00f5*/ 0x00030a05U,
/*00f6*/ 0x08010a05U,
/*00f7*/ 0x10010a05U,
/*00f8*/ 0x18060a05U,
/*00f9*/ 0xffffffffU,
/*00fa*/ 0xffffffffU,
/*00fb*/ 0xffffffffU,
/*00fc*/ 0x00020a06U,
/*00fd*/ 0x08030a06U,
/*00fe*/ 0x10010a06U,
/*00ff*/ 0x000f0a07U,
/*0100*/ 0x00200a08U,
/*0101*/ 0x00200a09U,
/*0102*/ 0x000b0a0aU,
/*0103*/ 0x100b0a0aU,
/*0104*/ 0x000b0a0bU,
/*0105*/ 0xffffffffU,
/*0106*/ 0xffffffffU,
/*0107*/ 0x00180a0cU,
/*0108*/ 0x00180a0dU,
/*0109*/ 0x00180a0eU,
/*010a*/ 0x00180a0fU,
/*010b*/ 0x18040a0fU,
/*010c*/ 0x00020a10U,
/*010d*/ 0x08020a10U,
/*010e*/ 0x10040a10U,
/*010f*/ 0x18040a10U,
/*0110*/ 0x00010a11U,
/*0111*/ 0x08010a11U,
/*0112*/ 0x10010a11U,
/*0113*/ 0x18030a11U,
/*0114*/ 0x00200a12U,
/*0115*/ 0x00200a13U,
/*0116*/ 0xffffffffU,
/*0117*/ 0x00140a14U,
/*0118*/ 0x00140a15U,
/*0119*/ 0x00140a16U,
/*011a*/ 0x00140a17U,
/*011b*/ 0x00140a18U,
/*011c*/ 0x00140a19U,
/*011d*/ 0x00140a1aU,
/*011e*/ 0x00140a1bU,
/*011f*/ 0x001e0a1cU,
/*0120*/ 0x000a0a1dU,
/*0121*/ 0x10060a1dU,
/*0122*/ 0x18060a1dU,
/*0123*/ 0x00060a1eU,
/*0124*/ 0xffffffffU,
/*0125*/ 0x08060a1eU,
/*0126*/ 0x00080a1fU,
/*0127*/ 0x080b0a1fU,
/*0128*/ 0x000b0a20U,
/*0129*/ 0x100b0a20U,
/*012a*/ 0x000b0a21U,
/*012b*/ 0x100b0a21U,
/*012c*/ 0x000b0a22U,
/*012d*/ 0x10040a22U,
/*012e*/ 0x000a0a23U,
/*012f*/ 0x10060a23U,
/*0130*/ 0x18080a23U,
/*0131*/ 0xffffffffU,
/*0132*/ 0x00040a24U,
/*0133*/ 0xffffffffU,
/*0134*/ 0xffffffffU,
/*0135*/ 0x00010b80U,
/*0136*/ 0x08020b80U,
/*0137*/ 0x10050b80U,
/*0138*/ 0x18050b80U,
/*0139*/ 0x00050b81U,
/*013a*/ 0x08050b81U,
/*013b*/ 0x100b0b81U,
/*013c*/ 0x00050b82U,
/*013d*/ 0x08010b82U,
/*013e*/ 0x10010b82U,
/*013f*/ 0xffffffffU,
/*0140*/ 0x18010b82U,
/*0141*/ 0x00010b83U,
/*0142*/ 0x08040b83U,
/*0143*/ 0x100b0b83U,
/*0144*/ 0x000b0b84U,
/*0145*/ 0xffffffffU,
/*0146*/ 0x10040b84U,
/*0147*/ 0x000b0b85U,
/*0148*/ 0x10040b85U,
/*0149*/ 0x18010b85U,
/*014a*/ 0x00010b86U,
/*014b*/ 0x08010b86U,
/*014c*/ 0x00200b87U,
/*014d*/ 0x00200b88U,
/*014e*/ 0x00080b89U,
/*014f*/ 0x080a0b89U,
/*0150*/ 0x18050b89U,
/*0151*/ 0x000a0b8aU,
/*0152*/ 0x10030b8aU,
/*0153*/ 0x18030b8aU,
/*0154*/ 0x00010b8bU,
/*0155*/ 0x08020b8bU,
/*0156*/ 0x10010b8bU,
/*0157*/ 0x18010b8bU,
/*0158*/ 0x00010b8cU,
/*0159*/ 0x08030b8cU,
/*015a*/ 0xffffffffU,
/*015b*/ 0x10040b8cU,
/*015c*/ 0x18040b8cU,
/*015d*/ 0x00040b8dU,
/*015e*/ 0x08040b8dU,
/*015f*/ 0xffffffffU,
/*0160*/ 0xffffffffU,
/*0161*/ 0xffffffffU,
/*0162*/ 0xffffffffU,
/*0163*/ 0xffffffffU,
/*0164*/ 0xffffffffU,
/*0165*/ 0xffffffffU,
/*0166*/ 0xffffffffU,
/*0167*/ 0xffffffffU,
/*0168*/ 0x000d0b8eU,
/*0169*/ 0x100d0b8eU,
/*016a*/ 0x000d0b8fU,
/*016b*/ 0x00050b90U,
/*016c*/ 0x00010b91U,
/*016d*/ 0x080e0b91U,
/*016e*/ 0x000e0b92U,
/*016f*/ 0x100e0b92U,
/*0170*/ 0x000e0b93U,
/*0171*/ 0x100e0b93U,
/*0172*/ 0x00040b94U,
/*0173*/ 0x08040b94U,
/*0174*/ 0x10040b94U,
/*0175*/ 0x18040b94U,
/*0176*/ 0x00040b95U,
/*0177*/ 0x080b0b95U,
/*0178*/ 0x000b0b96U,
/*0179*/ 0x100b0b96U,
/*017a*/ 0x000b0b97U,
/*017b*/ 0xffffffffU,
/*017c*/ 0xffffffffU,
/*017d*/ 0xffffffffU,
/*017e*/ 0xffffffffU,
/*017f*/ 0x000d0b98U,
/*0180*/ 0x100d0b98U,
/*0181*/ 0x000d0b99U,
/*0182*/ 0x10100b99U,
/*0183*/ 0x10080b8dU,
/*0184*/ 0x18080b8dU,
/*0185*/ 0x00100b9aU,
/*0186*/ 0x10100b9aU,
/*0187*/ 0x00100b9bU,
/*0188*/ 0x10100b9bU,
/*0189*/ 0x00100b9cU,
/*018a*/ 0x10030b9cU,
/*018b*/ 0x18040b9cU,
/*018c*/ 0x00010b9dU,
/*018d*/ 0x08040b9dU,
/*018e*/ 0xffffffffU,
/*018f*/ 0xffffffffU,
/*0190*/ 0x10010b9dU,
/*0191*/ 0x00140b9eU,
/*0192*/ 0x000a0b9fU,
/*0193*/ 0x100c0b9fU,
/*0194*/ 0x00120ba0U,
/*0195*/ 0x00140ba1U,
/*0196*/ 0x00120ba2U,
/*0197*/ 0x00110ba3U,
/*0198*/ 0x00110ba4U,
/*0199*/ 0x00120ba5U,
/*019a*/ 0x00120ba6U,
/*019b*/ 0x00120ba7U,
/*019c*/ 0x00120ba8U,
/*019d*/ 0x00120ba9U,
/*019e*/ 0x00120baaU,
/*019f*/ 0x00120babU,
/*01a0*/ 0x00120bacU,
/*01a1*/ 0xffffffffU,
/*01a2*/ 0xffffffffU,
/*01a3*/ 0x00190badU,
/*01a4*/ 0x00190baeU,
/*01a5*/ 0x00200bafU,
/*01a6*/ 0x00170bb0U,
/*01a7*/ 0x18080bb0U,
/*01a8*/ 0x00010bb1U,
/*01a9*/ 0x08010bb1U,
/*01aa*/ 0x00200bb2U,
/*01ab*/ 0x00080bb3U,
/*01ac*/ 0xffffffffU,
/*01ad*/ 0x08030bb3U,
/*01ae*/ 0x00180bb4U,
/*01af*/ 0x00180bb5U,
/*01b0*/ 0xffffffffU,
/*01b1*/ 0xffffffffU,
/*01b2*/ 0xffffffffU,
/*01b3*/ 0xffffffffU,
/*01b4*/ 0xffffffffU,
/*01b5*/ 0xffffffffU,
/*01b6*/ 0xffffffffU,
/*01b7*/ 0xffffffffU,
/*01b8*/ 0xffffffffU,
/*01b9*/ 0xffffffffU,
/*01ba*/ 0xffffffffU,
/*01bb*/ 0xffffffffU,
/*01bc*/ 0xffffffffU,
/*01bd*/ 0xffffffffU,
/*01be*/ 0xffffffffU,
/*01bf*/ 0x00100bb6U,
/*01c0*/ 0x10010bb6U,
/*01c1*/ 0x18010bb6U,
/*01c2*/ 0x00050bb7U,
/*01c3*/ 0x00200bb8U,
/*01c4*/ 0x00090bb9U,
/*01c5*/ 0xffffffffU,
/*01c6*/ 0xffffffffU,
/*01c7*/ 0x00200bbaU,
/*01c8*/ 0x00040bbbU,
/*01c9*/ 0x08100bbbU,
/*01ca*/ 0x18060bbbU,
/*01cb*/ 0x00100bbcU,
/*01cc*/ 0xffffffffU,
/*01cd*/ 0x10080bbcU,
/*01ce*/ 0xffffffffU,
/*01cf*/ 0xffffffffU,
/*01d0*/ 0xffffffffU,
/*01d1*/ 0x18030bbcU,
/*01d2*/ 0x00020bbdU,
/*01d3*/ 0xffffffffU,
/*01d4*/ 0x00200bbeU,
/*01d5*/ 0x000b0bbfU,
/*01d6*/ 0xffffffffU,
/*01d7*/ 0xffffffffU,
/*01d8*/ 0xffffffffU,
/*01d9*/ 0x10020bbfU,
/*01da*/ 0xffffffffU,
/*01db*/ 0xffffffffU,
/*01dc*/ 0xffffffffU,
/*01dd*/ 0xffffffffU,
/*01de*/ 0x00010200U,
/*01df*/ 0x08040200U,
/*01e0*/ 0x10100200U,
/*01e1*/ 0x00010201U,
/*01e2*/ 0x08010201U,
/*01e3*/ 0xffffffffU,
/*01e4*/ 0xffffffffU,
/*01e5*/ 0x10100201U,
/*01e6*/ 0xffffffffU,
/*01e7*/ 0xffffffffU,
/*01e8*/ 0xffffffffU,
/*01e9*/ 0xffffffffU,
/*01ea*/ 0xffffffffU,
/*01eb*/ 0xffffffffU,
/*01ec*/ 0xffffffffU,
/*01ed*/ 0xffffffffU,
/*01ee*/ 0xffffffffU,
/*01ef*/ 0x00200202U,
/*01f0*/ 0x00100203U,
/*01f1*/ 0x00200204U,
/*01f2*/ 0x00100205U,
/*01f3*/ 0x00200206U,
/*01f4*/ 0x00100207U,
/*01f5*/ 0x10100207U,
/*01f6*/ 0x00200208U,
/*01f7*/ 0x00200209U,
/*01f8*/ 0x0020020aU,
/*01f9*/ 0x0020020bU,
/*01fa*/ 0x0010020cU,
/*01fb*/ 0x0020020dU,
/*01fc*/ 0x0020020eU,
/*01fd*/ 0x0020020fU,
/*01fe*/ 0x00200210U,
/*01ff*/ 0x00100211U,
/*0200*/ 0x00200212U,
/*0201*/ 0x00200213U,
/*0202*/ 0x00200214U,
/*0203*/ 0x00200215U,
/*0204*/ 0x00090216U,
/*0205*/ 0x10010216U,
/*0206*/ 0x00200217U,
/*0207*/ 0x00050218U,
/*0208*/ 0x08010218U,
/*0209*/ 0x10080218U,
/*020a*/ 0x18080218U,
/*020b*/ 0x001e0219U,
/*020c*/ 0x001e021aU,
/*020d*/ 0x001e021bU,
/*020e*/ 0x001e021cU,
/*020f*/ 0x001e021dU,
/*0210*/ 0x001e021eU,
/*0211*/ 0x001e021fU,
/*0212*/ 0x001e0220U,
/*0213*/ 0x001e0221U,
/*0214*/ 0x001e0222U,
/*0215*/ 0x001e0223U,
/*0216*/ 0x001e0224U,
/*0217*/ 0x001e0225U,
/*0218*/ 0x001e0226U,
/*0219*/ 0x001e0227U,
/*021a*/ 0x001e0228U,
/*021b*/ 0x00010229U,
/*021c*/ 0x08010229U,
/*021d*/ 0x10010229U,
/*021e*/ 0x18040229U,
/*021f*/ 0x0008022aU,
/*0220*/ 0x0808022aU,
/*0221*/ 0x1008022aU,
/*0222*/ 0x1804022aU,
/*0223*/ 0x0005022bU,
/*0224*/ 0x0806022bU,
/*0225*/ 0x1007022bU,
/*0226*/ 0x1805022bU,
/*0227*/ 0x0006022cU,
/*0228*/ 0x0807022cU,
/*0229*/ 0x1005022cU,
/*022a*/ 0x1806022cU,
/*022b*/ 0x0007022dU,
/*022c*/ 0x0802022dU,
/*022d*/ 0x1001022dU,
/*022e*/ 0xffffffffU,
/*022f*/ 0x000a022eU,
/*0230*/ 0x1010022eU,
/*0231*/ 0x000a022fU,
/*0232*/ 0x1010022fU,
/*0233*/ 0x000a0230U,
/*0234*/ 0x10100230U,
/*0235*/ 0xffffffffU,
/*0236*/ 0x00100231U,
/*0237*/ 0xffffffffU,
/*0238*/ 0xffffffffU,
/*0239*/ 0x10010231U,
/*023a*/ 0x18010231U,
/*023b*/ 0x00010232U,
/*023c*/ 0x08010232U,
/*023d*/ 0x10010232U,
/*023e*/ 0x18010232U,
/*023f*/ 0x00020233U,
/*0240*/ 0x08020233U,
/*0241*/ 0x10020233U,
/*0242*/ 0x18020233U,
/*0243*/ 0x00020234U,
/*0244*/ 0x08030234U,
/*0245*/ 0x10010234U,
/*0246*/ 0x18010234U,
/*0247*/ 0x00010235U,
/*0248*/ 0x08010235U,
/*0249*/ 0xffffffffU,
/*024a*/ 0x10020235U,
/*024b*/ 0x18010235U,
/*024c*/ 0x00010236U,
/*024d*/ 0xffffffffU,
/*024e*/ 0x08020236U,
/*024f*/ 0x10010236U,
/*0250*/ 0x18010236U,
/*0251*/ 0xffffffffU,
/*0252*/ 0x00020237U,
/*0253*/ 0x08010237U,
/*0254*/ 0x10010237U,
/*0255*/ 0xffffffffU,
/*0256*/ 0x18020237U,
/*0257*/ 0x00070238U,
/*0258*/ 0x08010238U,
/*0259*/ 0x10010238U,
/*025a*/ 0x18010238U,
/*025b*/ 0x00010239U,
/*025c*/ 0x08010239U,
/*025d*/ 0x10010239U,
/*025e*/ 0xffffffffU,
/*025f*/ 0x18010239U,
/*0260*/ 0x0004023aU,
/*0261*/ 0x0804023aU,
/*0262*/ 0x1004023aU,
/*0263*/ 0x1801023aU,
/*0264*/ 0x0002023bU,
/*0265*/ 0x0806023bU,
/*0266*/ 0x1006023bU,
/*0267*/ 0xffffffffU,
/*0268*/ 0xffffffffU,
/*0269*/ 0xffffffffU,
/*026a*/ 0x1802023bU,
/*026b*/ 0x0010023cU,
/*026c*/ 0x1001023cU,
/*026d*/ 0x1801023cU,
/*026e*/ 0xffffffffU,
/*026f*/ 0x0004023dU,
/*0270*/ 0x0801023dU,
/*0271*/ 0x1004023dU,
/*0272*/ 0x1802023dU,
/*0273*/ 0x0008023eU,
/*0274*/ 0xffffffffU,
/*0275*/ 0xffffffffU,
/*0276*/ 0xffffffffU,
/*0277*/ 0x080a023eU,
/*0278*/ 0x0020023fU,
/*0279*/ 0x00200240U,
/*027a*/ 0x00050241U,
/*027b*/ 0x08010241U,
/*027c*/ 0x10050241U,
/*027d*/ 0x18080241U,
/*027e*/ 0x00010242U,
/*027f*/ 0x08080242U,
/*0280*/ 0x10010242U,
/*0281*/ 0x18080242U,
/*0282*/ 0x00010243U,
/*0283*/ 0x08040243U,
/*0284*/ 0x10040243U,
/*0285*/ 0x18040243U,
/*0286*/ 0x00040244U,
/*0287*/ 0x08040244U,
/*0288*/ 0x10040244U,
/*0289*/ 0x18040244U,
/*028a*/ 0x00040245U,
/*028b*/ 0x08040245U,
/*028c*/ 0x10040245U,
/*028d*/ 0x18010245U,
/*028e*/ 0x00040246U,
/*028f*/ 0x08040246U,
/*0290*/ 0x10040246U,
/*0291*/ 0x18040246U,
/*0292*/ 0x00040247U,
/*0293*/ 0x08040247U,
/*0294*/ 0x10060247U,
/*0295*/ 0x18060247U,
/*0296*/ 0x00060248U,
/*0297*/ 0x08060248U,
/*0298*/ 0x10060248U,
/*0299*/ 0x18060248U,
/*029a*/ 0x00040249U,
/*029b*/ 0x08010249U,
/*029c*/ 0x10010249U,
/*029d*/ 0x18020249U,
/*029e*/ 0xffffffffU,
/*029f*/ 0xffffffffU,
/*02a0*/ 0xffffffffU,
/*02a1*/ 0xffffffffU,
/*02a2*/ 0xffffffffU,
/*02a3*/ 0xffffffffU,
/*02a4*/ 0xffffffffU,
/*02a5*/ 0xffffffffU,
/*02a6*/ 0x0004024aU,
/*02a7*/ 0x0804024aU,
/*02a8*/ 0x1001024aU,
/*02a9*/ 0x1801024aU,
/*02aa*/ 0xffffffffU,
/*02ab*/ 0x0001024bU,
/*02ac*/ 0x0801024bU,
/*02ad*/ 0xffffffffU,
/*02ae*/ 0x1001024bU,
/*02af*/ 0x1801024bU,
/*02b0*/ 0x0001024cU,
/*02b1*/ 0x0804024cU,
/*02b2*/ 0x1004024cU,
/*02b3*/ 0x000a024dU,
/*02b4*/ 0x0020024eU,
/*02b5*/ 0x0004024fU,
/*02b6*/ 0x0808024fU,
/*02b7*/ 0xffffffffU,
/*02b8*/ 0xffffffffU,
/*02b9*/ 0xffffffffU,
/*02ba*/ 0xffffffffU,
/*02bb*/ 0xffffffffU,
/*02bc*/ 0xffffffffU,
/*02bd*/ 0x1002024fU,
/*02be*/ 0x1802024fU,
/*02bf*/ 0x00200250U,
/*02c0*/ 0x00020251U,
/*02c1*/ 0x08100251U,
/*02c2*/ 0x00100252U,
/*02c3*/ 0x10040252U,
/*02c4*/ 0x18040252U,
/*02c5*/ 0x00050253U,
/*02c6*/ 0x08050253U,
/*02c7*/ 0xffffffffU,
/*02c8*/ 0xffffffffU,
/*02c9*/ 0xffffffffU,
/*02ca*/ 0xffffffffU,
/*02cb*/ 0x10010253U,
/*02cc*/ 0x18010253U,
/*02cd*/ 0x00080254U,
/*02ce*/ 0x08080254U,
/*02cf*/ 0x10080254U,
/*02d0*/ 0x18080254U,
/*02d1*/ 0x00080255U,
/*02d2*/ 0x08080255U,
/*02d3*/ 0x10080255U,
/*02d4*/ 0x18080255U,
/*02d5*/ 0x00080256U,
/*02d6*/ 0x08080256U,
/*02d7*/ 0x10080256U,
/*02d8*/ 0xffffffffU,
/*02d9*/ 0xffffffffU,
/*02da*/ 0xffffffffU,
/*02db*/ 0xffffffffU,
/*02dc*/ 0xffffffffU,
/*02dd*/ 0xffffffffU,
/*02de*/ 0x18030256U,
/*02df*/ 0x00010257U,
/*02e0*/ 0x08020257U,
/*02e1*/ 0x10010257U,
/*02e2*/ 0x18040257U,
/*02e3*/ 0x00020258U,
/*02e4*/ 0x08010258U,
/*02e5*/ 0x10010258U,
/*02e6*/ 0xffffffffU,
/*02e7*/ 0x18010258U,
/*02e8*/ 0x00040259U,
/*02e9*/ 0x08080259U,
/*02ea*/ 0x100a0259U,
/*02eb*/ 0x000a025aU,
/*02ec*/ 0x100a025aU,
/*02ed*/ 0x000a025bU,
/*02ee*/ 0x100a025bU,
/*02ef*/ 0x000a025cU,
/*02f0*/ 0x0020025dU,
/*02f1*/ 0x0020025eU,
/*02f2*/ 0x0001025fU,
/*02f3*/ 0xffffffffU,
/*02f4*/ 0xffffffffU,
/*02f5*/ 0xffffffffU,
/*02f6*/ 0x0802025fU,
/*02f7*/ 0x1002025fU,
/*02f8*/ 0x00100260U,
/*02f9*/ 0x10050260U,
/*02fa*/ 0x18060260U,
/*02fb*/ 0x00050261U,
/*02fc*/ 0x08050261U,
/*02fd*/ 0x100e0261U,
/*02fe*/ 0x00050262U,
/*02ff*/ 0x080e0262U,
/*0300*/ 0x18050262U,
/*0301*/ 0x000e0263U,
/*0302*/ 0x10050263U,
/*0303*/ 0x18010263U,
/*0304*/ 0x00050264U,
/*0305*/ 0x08050264U,
/*0306*/ 0x100a0264U,
/*0307*/ 0x000a0265U,
/*0308*/ 0x10050265U,
/*0309*/ 0x18050265U,
/*030a*/ 0x000a0266U,
/*030b*/ 0x100a0266U,
/*030c*/ 0x00050267U,
/*030d*/ 0x08050267U,
/*030e*/ 0x100a0267U,
/*030f*/ 0x000a0268U,
/*0310*/ 0xffffffffU,
/*0311*/ 0xffffffffU,
/*0312*/ 0xffffffffU,
/*0313*/ 0xffffffffU,
/*0314*/ 0xffffffffU,
/*0315*/ 0xffffffffU,
/*0316*/ 0x10070268U,
/*0317*/ 0x18070268U,
/*0318*/ 0x00040269U,
/*0319*/ 0x08040269U,
/*031a*/ 0xffffffffU,
/*031b*/ 0xffffffffU,
/*031c*/ 0xffffffffU,
/*031d*/ 0x10040269U,
/*031e*/ 0x18080269U,
/*031f*/ 0x0008026aU,
/*0320*/ 0x0804026aU,
/*0321*/ 0xffffffffU,
/*0322*/ 0xffffffffU,
/*0323*/ 0xffffffffU,
/*0324*/ 0x1004026aU,
/*0325*/ 0xffffffffU,
/*0326*/ 0xffffffffU,
/*0327*/ 0xffffffffU,
/*0328*/ 0x1804026aU,
/*0329*/ 0xffffffffU,
/*032a*/ 0xffffffffU,
/*032b*/ 0xffffffffU,
/*032c*/ 0x0004026bU,
/*032d*/ 0x0805026bU,
/*032e*/ 0x1007026bU,
/*032f*/ 0x1808026bU,
/*0330*/ 0x0010026cU,
/*0331*/ 0x1008026cU,
/*0332*/ 0x0010026dU,
/*0333*/ 0x1008026dU,
/*0334*/ 0x0010026eU,
/*0335*/ 0x1008026eU,
/*0336*/ 0x1808026eU,
/*0337*/ 0x0001026fU,
/*0338*/ 0x0801026fU,
/*0339*/ 0x1006026fU,
/*033a*/ 0x1806026fU,
/*033b*/ 0x00060270U,
/*033c*/ 0xffffffffU,
/*033d*/ 0x08010270U,
/*033e*/ 0x10030270U,
/*033f*/ 0xffffffffU,
/*0340*/ 0xffffffffU,
/*0341*/ 0xffffffffU,
/*0342*/ 0x000a0271U,
/*0343*/ 0x100a0271U,
/*0344*/ 0x00040272U,
/*0345*/ 0x08010272U,
/*0346*/ 0x10040272U,
/*0347*/ 0xffffffffU,
/*0348*/ 0xffffffffU,
/*0349*/ 0xffffffffU,
/*034a*/ 0xffffffffU,
/*034b*/ 0xffffffffU,
/*034c*/ 0xffffffffU,
/*034d*/ 0x18070272U,
/*034e*/ 0x00070273U,
/*034f*/ 0x08050273U,
/*0350*/ 0x10050273U,
/*0351*/ 0xffffffffU,
/*0352*/ 0xffffffffU,
/*0353*/ 0xffffffffU,
/*0354*/ 0x18040273U,
/*0355*/ 0x00010274U,
/*0356*/ 0x08010274U,
/*0357*/ 0x10020274U,
/*0358*/ 0x18080274U,
/*0359*/ 0x00200275U,
/*035a*/ 0x00200276U,
/*035b*/ 0x00100277U,
/*035c*/ 0xffffffffU,
/*035d*/ 0xffffffffU,
/*035e*/ 0xffffffffU,
/*035f*/ 0x10020277U,
/*0360*/ 0x18010277U,
/*0361*/ 0xffffffffU,
/*0362*/ 0x00020278U,
/*0363*/ 0x08100278U,
/*0364*/ 0x00100279U,
/*0365*/ 0x10100279U,
/*0366*/ 0x0008027aU,
/*0367*/ 0x0808027aU,
/*0368*/ 0x1008027aU,
/*0369*/ 0xffffffffU,
/*036a*/ 0x0010027bU,
/*036b*/ 0x1010027bU,
/*036c*/ 0x0010027cU,
/*036d*/ 0x1008027cU,
/*036e*/ 0x1808027cU,
/*036f*/ 0x0008027dU,
/*0370*/ 0xffffffffU,
/*0371*/ 0x0810027dU,
/*0372*/ 0x0010027eU,
/*0373*/ 0x1010027eU,
/*0374*/ 0x0008027fU,
/*0375*/ 0x0808027fU,
/*0376*/ 0x1008027fU,
/*0377*/ 0xffffffffU,
/*0378*/ 0x1808027fU,
/*0379*/ 0x00100280U,
/*037a*/ 0x10100280U,
/*037b*/ 0x00100281U,
/*037c*/ 0x10080281U,
/*037d*/ 0x18080281U,
/*037e*/ 0x00080282U,
/*037f*/ 0xffffffffU,
/*0380*/ 0x08100282U,
/*0381*/ 0x00100283U,
/*0382*/ 0x10100283U,
/*0383*/ 0x00080284U,
/*0384*/ 0x08080284U,
/*0385*/ 0x10080284U,
/*0386*/ 0xffffffffU,
/*0387*/ 0x00100285U,
/*0388*/ 0x10100285U,
/*0389*/ 0x00100286U,
/*038a*/ 0x10080286U,
/*038b*/ 0x18080286U,
/*038c*/ 0x00080287U,
/*038d*/ 0xffffffffU,
/*038e*/ 0x08080287U,
/*038f*/ 0x10100287U,
/*0390*/ 0x00100288U,
/*0391*/ 0x10100288U,
/*0392*/ 0x00080289U,
/*0393*/ 0x08080289U,
/*0394*/ 0x10080289U,
/*0395*/ 0xffffffffU,
/*0396*/ 0x0010028aU,
/*0397*/ 0x1010028aU,
/*0398*/ 0x0010028bU,
/*0399*/ 0x1008028bU,
/*039a*/ 0x1808028bU,
/*039b*/ 0x0008028cU,
/*039c*/ 0xffffffffU,
/*039d*/ 0x0810028cU,
/*039e*/ 0x0010028dU,
/*039f*/ 0x1010028dU,
/*03a0*/ 0x0008028eU,
/*03a1*/ 0x0808028eU,
/*03a2*/ 0x1008028eU,
/*03a3*/ 0xffffffffU,
/*03a4*/ 0x1808028eU,
/*03a5*/ 0x0010028fU,
/*03a6*/ 0x1010028fU,
/*03a7*/ 0x00100290U,
/*03a8*/ 0x10080290U,
/*03a9*/ 0x18080290U,
/*03aa*/ 0x00080291U,
/*03ab*/ 0xffffffffU,
/*03ac*/ 0x08100291U,
/*03ad*/ 0x00100292U,
/*03ae*/ 0x10100292U,
/*03af*/ 0x00080293U,
/*03b0*/ 0x08080293U,
/*03b1*/ 0x10080293U,
/*03b2*/ 0xffffffffU,
/*03b3*/ 0x00100294U,
/*03b4*/ 0x10100294U,
/*03b5*/ 0x00100295U,
/*03b6*/ 0x10080295U,
/*03b7*/ 0x18080295U,
/*03b8*/ 0x00080296U,
/*03b9*/ 0xffffffffU,
/*03ba*/ 0x08080296U,
/*03bb*/ 0x10020296U,
/*03bc*/ 0x18030296U,
/*03bd*/ 0x000a0297U,
/*03be*/ 0x100a0297U,
/*03bf*/ 0x000a0298U,
/*03c0*/ 0x10050298U,
/*03c1*/ 0x18040298U,
/*03c2*/ 0x00080299U,
/*03c3*/ 0x08080299U,
/*03c4*/ 0x10060299U,
/*03c5*/ 0x18060299U,
/*03c6*/ 0x0011029aU,
/*03c7*/ 0x1808029aU,
/*03c8*/ 0x0004029bU,
/*03c9*/ 0x0806029bU,
/*03ca*/ 0xffffffffU,
/*03cb*/ 0x1006029bU,
/*03cc*/ 0x1808029bU,
/*03cd*/ 0x0008029cU,
/*03ce*/ 0x0804029cU,
/*03cf*/ 0x1008029cU,
/*03d0*/ 0x1808029cU,
/*03d1*/ 0x0006029dU,
/*03d2*/ 0x0806029dU,
/*03d3*/ 0x0011029eU,
/*03d4*/ 0x1808029eU,
/*03d5*/ 0x0004029fU,
/*03d6*/ 0x0806029fU,
/*03d7*/ 0xffffffffU,
/*03d8*/ 0x1006029fU,
/*03d9*/ 0x1808029fU,
/*03da*/ 0x000802a0U,
/*03db*/ 0x080402a0U,
/*03dc*/ 0x100802a0U,
/*03dd*/ 0x180802a0U,
/*03de*/ 0x000602a1U,
/*03df*/ 0x080602a1U,
/*03e0*/ 0x001102a2U,
/*03e1*/ 0x180802a2U,
/*03e2*/ 0x000402a3U,
/*03e3*/ 0x080602a3U,
/*03e4*/ 0xffffffffU,
/*03e5*/ 0x100602a3U,
/*03e6*/ 0x180802a3U,
/*03e7*/ 0x000802a4U,
/*03e8*/ 0x080402a4U,
/*03e9*/ 0x100402a4U,
/*03ea*/ 0x180402a4U,
/*03eb*/ 0x000402a5U,
/*03ec*/ 0x080402a5U,
/*03ed*/ 0x100402a5U,
/*03ee*/ 0x180402a5U,
/*03ef*/ 0x000402a6U,
/*03f0*/ 0x080402a6U,
/*03f1*/ 0x100402a6U,
/*03f2*/ 0x180402a6U,
/*03f3*/ 0x000402a7U,
/*03f4*/ 0x080402a7U,
/*03f5*/ 0x100402a7U,
/*03f6*/ 0x180402a7U,
/*03f7*/ 0x000402a8U,
/*03f8*/ 0x080402a8U,
/*03f9*/ 0x100402a8U,
/*03fa*/ 0x180402a8U,
/*03fb*/ 0x000402a9U,
/*03fc*/ 0x081202a9U,
/*03fd*/ 0x001102aaU,
/*03fe*/ 0x001202abU,
/*03ff*/ 0x002002acU,
/*0400*/ 0x002002adU,
/*0401*/ 0x002002aeU,
/*0402*/ 0x002002afU,
/*0403*/ 0x002002b0U,
/*0404*/ 0x002002b1U,
/*0405*/ 0x002002b2U,
/*0406*/ 0x002002b3U,
/*0407*/ 0x002002b4U,
/*0408*/ 0x000302b5U,
/*0409*/ 0x080502b5U,
/*040a*/ 0x100502b5U,
/*040b*/ 0x180102b5U,
/*040c*/ 0x000502b6U,
/*040d*/ 0x080502b6U,
/*040e*/ 0x100502b6U,
/*040f*/ 0x180502b6U,
/*0410*/ 0x000502b7U,
/*0411*/ 0x080502b7U,
/*0412*/ 0x100502b7U,
/*0413*/ 0x180502b7U,
/*0414*/ 0x000502b8U,
/*0415*/ 0x080502b8U,
/*0416*/ 0x100502b8U,
/*0417*/ 0x180502b8U,
/*0418*/ 0x000502b9U,
/*0419*/ 0x080502b9U,
/*041a*/ 0x100502b9U,
/*041b*/ 0x180502b9U,
/*041c*/ 0x000502baU,
/*041d*/ 0x080502baU,
/*041e*/ 0x100502baU,
/*041f*/ 0x180502baU,
/*0420*/ 0x000502bbU,
/*0421*/ 0x080502bbU,
/*0422*/ 0x100102bbU,
/*0423*/ 0x180202bbU,
/*0424*/ 0x000202bcU,
/*0425*/ 0x080202bcU,
/*0426*/ 0x100202bcU,
/*0427*/ 0x180102bcU,
/*0428*/ 0x000402bdU,
/*0429*/ 0x081002bdU,
/*042a*/ 0x002002beU,
/*042b*/ 0x001002bfU,
/*042c*/ 0x002002c0U,
/*042d*/ 0x001002c1U,
/*042e*/ 0x002002c2U,
/*042f*/ 0x000702c3U,
/*0430*/ 0x080102c3U,
/*0431*/ 0x100202c3U,
/*0432*/ 0x180602c3U,
/*0433*/ 0x000102c4U,
/*0434*/ 0x080102c4U,
/*0435*/ 0x002002c5U,
/*0436*/ 0x000302c6U,
/*0437*/ 0x002002c7U,
/*0438*/ 0x002002c8U,
/*0439*/ 0xffffffffU,
/*043a*/ 0xffffffffU,
/*043b*/ 0xffffffffU,
/*043c*/ 0xffffffffU,
/*043d*/ 0xffffffffU,
/*043e*/ 0xffffffffU,
/*043f*/ 0xffffffffU,
/*0440*/ 0xffffffffU,
/*0441*/ 0xffffffffU,
/*0442*/ 0xffffffffU,
/*0443*/ 0xffffffffU,
/*0444*/ 0xffffffffU,
/*0445*/ 0xffffffffU,
/*0446*/ 0xffffffffU,
/*0447*/ 0xffffffffU,
/*0448*/ 0xffffffffU,
/*0449*/ 0xffffffffU,
/*044a*/ 0xffffffffU,
/*044b*/ 0xffffffffU,
/*044c*/ 0xffffffffU,
/*044d*/ 0xffffffffU,
/*044e*/ 0xffffffffU,
/*044f*/ 0xffffffffU,
/*0450*/ 0xffffffffU,
/*0451*/ 0xffffffffU,
/*0452*/ 0xffffffffU,
/*0453*/ 0xffffffffU,
/*0454*/ 0xffffffffU,
/*0455*/ 0xffffffffU,
/*0456*/ 0xffffffffU,
/*0457*/ 0xffffffffU,
/*0458*/ 0xffffffffU,
/*0459*/ 0xffffffffU,
/*045a*/ 0xffffffffU,
/*045b*/ 0xffffffffU,
/*045c*/ 0xffffffffU,
/*045d*/ 0xffffffffU,
/*045e*/ 0xffffffffU,
/*045f*/ 0x000402c9U,
/*0460*/ 0xffffffffU,
/*0461*/ 0xffffffffU,
/*0462*/ 0xffffffffU,
/*0463*/ 0xffffffffU,
/*0464*/ 0xffffffffU,
/*0465*/ 0xffffffffU,
/*0466*/ 0xffffffffU,
/*0467*/ 0xffffffffU,
/*0468*/ 0xffffffffU,
/*0469*/ 0xffffffffU,
/*046a*/ 0xffffffffU,
/*046b*/ 0xffffffffU,
/*046c*/ 0xffffffffU,
/*046d*/ 0xffffffffU,
/*046e*/ 0xffffffffU,
/*046f*/ 0xffffffffU,
/*0470*/ 0xffffffffU,
/*0471*/ 0xffffffffU,
/*0472*/ 0xffffffffU,
/*0473*/ 0xffffffffU,
/*0474*/ 0xffffffffU,
/*0475*/ 0xffffffffU,
/*0476*/ 0xffffffffU,
/*0477*/ 0xffffffffU,
/*0478*/ 0xffffffffU,
/*0479*/ 0xffffffffU,
/*047a*/ 0xffffffffU,
/*047b*/ 0xffffffffU,
/*047c*/ 0xffffffffU,
/*047d*/ 0xffffffffU,
/*047e*/ 0xffffffffU,
/*047f*/ 0xffffffffU,
/*0480*/ 0xffffffffU,
/*0481*/ 0xffffffffU,
/*0482*/ 0xffffffffU,
/*0483*/ 0xffffffffU,
/*0484*/ 0xffffffffU,
/*0485*/ 0xffffffffU,
/*0486*/ 0xffffffffU,
/*0487*/ 0xffffffffU,
/*0488*/ 0xffffffffU,
/*0489*/ 0xffffffffU,
/*048a*/ 0xffffffffU,
/*048b*/ 0xffffffffU,
/*048c*/ 0xffffffffU,
/*048d*/ 0xffffffffU,
/*048e*/ 0xffffffffU,
/*048f*/ 0xffffffffU,
/*0490*/ 0xffffffffU,
/*0491*/ 0xffffffffU,
/*0492*/ 0xffffffffU,
/*0493*/ 0xffffffffU,
/*0494*/ 0xffffffffU,
	 },
	{
/*0000*/ 0x00200400U,
/*0001*/ 0x00040401U,
/*0002*/ 0x080b0401U,
/*0003*/ 0x000a0402U,
/*0004*/ 0x10020402U,
/*0005*/ 0x18010402U,
/*0006*/ 0x00050403U,
/*0007*/ 0x08050403U,
/*0008*/ 0x10050403U,
/*0009*/ 0x18050403U,
/*000a*/ 0x00050404U,
/*000b*/ 0x08050404U,
/*000c*/ 0x10050404U,
/*000d*/ 0x18050404U,
/*000e*/ 0x00050405U,
/*000f*/ 0x08040405U,
/*0010*/ 0x10030405U,
/*0011*/ 0x00180406U,
/*0012*/ 0x18030406U,
/*0013*/ 0x00180407U,
/*0014*/ 0x18020407U,
/*0015*/ 0x00010408U,
/*0016*/ 0x08020408U,
/*0017*/ 0x10010408U,
/*0018*/ 0x18010408U,
/*0019*/ 0x00020409U,
/*001a*/ 0x08040409U,
/*001b*/ 0x10040409U,
/*001c*/ 0x18040409U,
/*001d*/ 0xffffffffU,
/*001e*/ 0x0004040aU,
/*001f*/ 0xffffffffU,
/*0020*/ 0xffffffffU,
/*0021*/ 0x0809040aU,
/*0022*/ 0x1801040aU,
/*0023*/ 0x0020040bU,
/*0024*/ 0x001c040cU,
/*0025*/ 0x0001040dU,
/*0026*/ 0x0807040dU,
/*0027*/ 0x1009040dU,
/*0028*/ 0x000a040eU,
/*0029*/ 0x1005040eU,
/*002a*/ 0x1801040eU,
/*002b*/ 0x1001040fU,
/*002c*/ 0x1802040fU,
/*002d*/ 0x0009040fU,
/*002e*/ 0x00090410U,
/*002f*/ 0x10020410U,
/*0030*/ 0x00200411U,
/*0031*/ 0x00010412U,
/*0032*/ 0x08020412U,
/*0033*/ 0xffffffffU,
/*0034*/ 0xffffffffU,
/*0035*/ 0xffffffffU,
/*0036*/ 0xffffffffU,
/*0037*/ 0x00200413U,
/*0038*/ 0x00200414U,
/*0039*/ 0x00200415U,
/*003a*/ 0x00200416U,
/*003b*/ 0x00030417U,
/*003c*/ 0x08010417U,
/*003d*/ 0x10040417U,
/*003e*/ 0x18030417U,
/*003f*/ 0x00040418U,
/*0040*/ 0x08040418U,
/*0041*/ 0x10040418U,
/*0042*/ 0x18040418U,
/*0043*/ 0x00010419U,
/*0044*/ 0x08010419U,
/*0045*/ 0x10060419U,
/*0046*/ 0x18040419U,
/*0047*/ 0xffffffffU,
/*0048*/ 0x0006041aU,
/*0049*/ 0x0804041aU,
/*004a*/ 0x1006041aU,
/*004b*/ 0x1804041aU,
/*004c*/ 0x0002041bU,
/*004d*/ 0x0805041bU,
/*004e*/ 0x1008041bU,
/*004f*/ 0xffffffffU,
/*0050*/ 0x1806041bU,
/*0051*/ 0x0003041cU,
/*0052*/ 0x080b041cU,
/*0053*/ 0x1804041cU,
/*0054*/ 0x0004041dU,
/*0055*/ 0x0804041dU,
/*0056*/ 0x1001041dU,
/*0057*/ 0xffffffffU,
/*0058*/ 0x0009041eU,
/*0059*/ 0x0020041fU,
/*005a*/ 0x00200420U,
/*005b*/ 0x00200421U,
/*005c*/ 0x00200422U,
/*005d*/ 0x00100423U,
/*005e*/ 0xffffffffU,
/*005f*/ 0x10010423U,
/*0060*/ 0x18060423U,
/*0061*/ 0x00080424U,
/*0062*/ 0x00200425U,
/*0063*/ 0x00100426U,
/*0064*/ 0x100a0426U,
/*0065*/ 0x00060427U,
/*0066*/ 0x08070427U,
/*0067*/ 0x10080427U,
/*0068*/ 0x18080427U,
/*0069*/ 0x000a0428U,
/*006a*/ 0x10070428U,
/*006b*/ 0x18080428U,
/*006c*/ 0x00080429U,
/*006d*/ 0x08030429U,
/*006e*/ 0x100a0429U,
/*006f*/ 0x000a042aU,
/*0070*/ 0x0011042bU,
/*0071*/ 0x0009042cU,
/*0072*/ 0x1009042cU,
/*0073*/ 0x0010042dU,
/*0074*/ 0x100e042dU,
/*0075*/ 0x000e042eU,
/*0076*/ 0x0012042fU,
/*0077*/ 0x000a0430U,
/*0078*/ 0x100a0430U,
/*0079*/ 0x00020431U,
/*007a*/ 0x00200432U,
/*007b*/ 0x000b0433U,
/*007c*/ 0x100b0433U,
/*007d*/ 0x00200434U,
/*007e*/ 0x00120435U,
/*007f*/ 0x00200436U,
/*0080*/ 0x00200437U,
/*0081*/ 0x00080438U,
/*0082*/ 0x08010438U,
/*0083*/ 0x10010438U,
/*0084*/ 0x18010438U,
/*0085*/ 0x00080439U,
/*0086*/ 0x080c0439U,
/*0087*/ 0x000c043aU,
/*0088*/ 0x100c043aU,
/*0089*/ 0x000c043bU,
/*008a*/ 0x100c043bU,
/*008b*/ 0x000c043cU,
/*008c*/ 0x100c043cU,
/*008d*/ 0x000c043dU,
/*008e*/ 0x100c043dU,
/*008f*/ 0x000c043eU,
/*0090*/ 0x100c043eU,
/*0091*/ 0x000b043fU,
/*0092*/ 0x1009043fU,
/*0093*/ 0x00010440U,
/*0094*/ 0x000b0441U,
/*0095*/ 0x100b0441U,
/*0096*/ 0x000b0442U,
/*0097*/ 0x100b0442U,
/*0098*/ 0x000b0443U,
/*0099*/ 0x100b0443U,
/*009a*/ 0x000b0444U,
/*009b*/ 0x100b0444U,
/*009c*/ 0x000b0445U,
/*009d*/ 0x100a0445U,
/*009e*/ 0x00020446U,
/*009f*/ 0x080a0446U,
/*00a0*/ 0x000a0447U,
/*00a1*/ 0x100a0447U,
/*00a2*/ 0x000a0448U,
/*00a3*/ 0x100a0448U,
/*00a4*/ 0x000a0449U,
/*00a5*/ 0x100a0449U,
/*00a6*/ 0x000a044aU,
/*00a7*/ 0x100a044aU,
/*00a8*/ 0x000a044bU,
/*00a9*/ 0x100a044bU,
/*00aa*/ 0x000a044cU,
/*00ab*/ 0x100a044cU,
/*00ac*/ 0x000a044dU,
/*00ad*/ 0x100a044dU,
/*00ae*/ 0x000a044eU,
/*00af*/ 0x100a044eU,
/*00b0*/ 0x000a044fU,
/*00b1*/ 0x100a044fU,
/*00b2*/ 0x000a0450U,
/*00b3*/ 0x100a0450U,
/*00b4*/ 0x000a0451U,
/*00b5*/ 0x100a0451U,
/*00b6*/ 0x000a0452U,
/*00b7*/ 0x100a0452U,
/*00b8*/ 0x000a0453U,
/*00b9*/ 0x100a0453U,
/*00ba*/ 0x000a0454U,
/*00bb*/ 0x10040454U,
/*00bc*/ 0x18030454U,
/*00bd*/ 0x000a0455U,
/*00be*/ 0x100a0455U,
/*00bf*/ 0x00010456U,
/*00c0*/ 0x080a0456U,
/*00c1*/ 0x18040456U,
/*00c2*/ 0x000b0457U,
/*00c3*/ 0x100a0457U,
/*00c4*/ 0x00030458U,
/*00c5*/ 0x00080459U,
/*00c6*/ 0x08080459U,
/*00c7*/ 0x10080459U,
/*00c8*/ 0x18080459U,
/*00c9*/ 0x0008045aU,
/*00ca*/ 0xffffffffU,
/*00cb*/ 0x0808045aU,
/*00cc*/ 0x1001045aU,
/*00cd*/ 0x1808045aU,
/*00ce*/ 0x0008045bU,
/*00cf*/ 0x0802045bU,
/*00d0*/ 0x1002045bU,
/*00d1*/ 0x1805045bU,
/*00d2*/ 0x0005045cU,
/*00d3*/ 0xffffffffU,
/*00d4*/ 0x0804045cU,
/*00d5*/ 0x100a045cU,
/*00d6*/ 0x0006045dU,
/*00d7*/ 0x0808045dU,
/*00d8*/ 0x1008045dU,
/*00d9*/ 0x1804045dU,
/*00da*/ 0x0004045eU,
/*00db*/ 0x0805045eU,
/*00dc*/ 0x1004045eU,
/*00dd*/ 0x1805045eU,
/*00de*/ 0x000a045fU,
/*00df*/ 0x100a045fU,
/*00e0*/ 0x00080460U,
/*00e1*/ 0xffffffffU,
/*00e2*/ 0x08040460U,
/*00e3*/ 0xffffffffU,
/*00e4*/ 0xffffffffU,
/*00e5*/ 0x00050600U,
/*00e6*/ 0x08050600U,
/*00e7*/ 0x10050600U,
/*00e8*/ 0x18050600U,
/*00e9*/ 0x00050601U,
/*00ea*/ 0x08050601U,
/*00eb*/ 0x100b0601U,
/*00ec*/ 0x00010602U,
/*00ed*/ 0x08030602U,
/*00ee*/ 0x00200603U,
/*00ef*/ 0x00100604U,
/*00f0*/ 0x10040604U,
/*00f1*/ 0x000a0605U,
/*00f2*/ 0x10090605U,
/*00f3*/ 0x00080606U,
/*00f4*/ 0x08030606U,
/*00f5*/ 0x10030606U,
/*00f6*/ 0x18010606U,
/*00f7*/ 0x00010607U,
/*00f8*/ 0x08070607U,
/*00f9*/ 0x10070607U,
/*00fa*/ 0x18050607U,
/*00fb*/ 0x00010608U,
/*00fc*/ 0x08020608U,
/*00fd*/ 0x10030608U,
/*00fe*/ 0x18010608U,
/*00ff*/ 0x000f0609U,
/*0100*/ 0x0020060aU,
/*0101*/ 0x0020060bU,
/*0102*/ 0x000b060cU,
/*0103*/ 0x100b060cU,
/*0104*/ 0x000b060dU,
/*0105*/ 0x0018060eU,
/*0106*/ 0x0018060fU,
/*0107*/ 0xffffffffU,
/*0108*/ 0xffffffffU,
/*0109*/ 0xffffffffU,
/*010a*/ 0xffffffffU,
/*010b*/ 0xffffffffU,
/*010c*/ 0x1802060fU,
/*010d*/ 0x00020610U,
/*010e*/ 0x08040610U,
/*010f*/ 0x10040610U,
/*0110*/ 0x18010610U,
/*0111*/ 0x00010611U,
/*0112*/ 0x08010611U,
/*0113*/ 0x10030611U,
/*0114*/ 0x00200612U,
/*0115*/ 0x00200613U,
/*0116*/ 0xffffffffU,
/*0117*/ 0x00140614U,
/*0118*/ 0x00140615U,
/*0119*/ 0x00140616U,
/*011a*/ 0x00140617U,
/*011b*/ 0x00140618U,
/*011c*/ 0x00140619U,
/*011d*/ 0x0014061aU,
/*011e*/ 0x0014061bU,
/*011f*/ 0x0018061cU,
/*0120*/ 0x000a061dU,
/*0121*/ 0x1006061dU,
/*0122*/ 0x1806061dU,
/*0123*/ 0x0006061eU,
/*0124*/ 0xffffffffU,
/*0125*/ 0x0806061eU,
/*0126*/ 0x0008061fU,
/*0127*/ 0x080b061fU,
/*0128*/ 0x000b0620U,
/*0129*/ 0x100b0620U,
/*012a*/ 0x000b0621U,
/*012b*/ 0x100b0621U,
/*012c*/ 0x000b0622U,
/*012d*/ 0x10040622U,
/*012e*/ 0x000a0623U,
/*012f*/ 0x10060623U,
/*0130*/ 0x18080623U,
/*0131*/ 0x00080624U,
/*0132*/ 0x08040624U,
/*0133*/ 0x00020680U,
/*0134*/ 0x00010681U,
/*0135*/ 0x08010681U,
/*0136*/ 0x10020681U,
/*0137*/ 0x18050681U,
/*0138*/ 0x00050682U,
/*0139*/ 0x08050682U,
/*013a*/ 0x10050682U,
/*013b*/ 0x000b0683U,
/*013c*/ 0x10050683U,
/*013d*/ 0x18010683U,
/*013e*/ 0x00010684U,
/*013f*/ 0xffffffffU,
/*0140*/ 0x08010684U,
/*0141*/ 0x10010684U,
/*0142*/ 0x18040684U,
/*0143*/ 0x000b0685U,
/*0144*/ 0x100b0685U,
/*0145*/ 0x000b0686U,
/*0146*/ 0x10040686U,
/*0147*/ 0x000b0687U,
/*0148*/ 0x10040687U,
/*0149*/ 0x18010687U,
/*014a*/ 0x00010688U,
/*014b*/ 0x08010688U,
/*014c*/ 0x00200689U,
/*014d*/ 0x0020068aU,
/*014e*/ 0x0008068bU,
/*014f*/ 0x080a068bU,
/*0150*/ 0x1805068bU,
/*0151*/ 0x000a068cU,
/*0152*/ 0x1003068cU,
/*0153*/ 0x1803068cU,
/*0154*/ 0x0001068dU,
/*0155*/ 0x0802068dU,
/*0156*/ 0x1001068dU,
/*0157*/ 0x1801068dU,
/*0158*/ 0x0001068eU,
/*0159*/ 0x0802068eU,
/*015a*/ 0x1001068eU,
/*015b*/ 0x0004068fU,
/*015c*/ 0x0804068fU,
/*015d*/ 0x1004068fU,
/*015e*/ 0x1804068fU,
/*015f*/ 0x00010690U,
/*0160*/ 0x08010690U,
/*0161*/ 0x10010690U,
/*0162*/ 0x00200691U,
/*0163*/ 0x00200692U,
/*0164*/ 0x00200693U,
/*0165*/ 0x00200694U,
/*0166*/ 0xffffffffU,
/*0167*/ 0x1801068eU,
/*0168*/ 0x000d0696U,
/*0169*/ 0x100d0696U,
/*016a*/ 0x000d0697U,
/*016b*/ 0x00050698U,
/*016c*/ 0x00010699U,
/*016d*/ 0x080e0699U,
/*016e*/ 0x000e069aU,
/*016f*/ 0x100e069aU,
/*0170*/ 0x000e069bU,
/*0171*/ 0x100e069bU,
/*0172*/ 0x0004069cU,
/*0173*/ 0x0804069cU,
/*0174*/ 0x1004069cU,
/*0175*/ 0x1804069cU,
/*0176*/ 0x0004069dU,
/*0177*/ 0x080b069dU,
/*0178*/ 0x000b069eU,
/*0179*/ 0x100b069eU,
/*017a*/ 0x000b069fU,
/*017b*/ 0xffffffffU,
/*017c*/ 0xffffffffU,
/*017d*/ 0xffffffffU,
/*017e*/ 0xffffffffU,
/*017f*/ 0x000d06a0U,
/*0180*/ 0x100d06a0U,
/*0181*/ 0x000d06a1U,
/*0182*/ 0x101006a1U,
/*0183*/ 0x00080695U,
/*0184*/ 0x08080695U,
/*0185*/ 0x001006a2U,
/*0186*/ 0x101006a2U,
/*0187*/ 0x001006a3U,
/*0188*/ 0x101006a3U,
/*0189*/ 0x001006a4U,
/*018a*/ 0x100306a4U,
/*018b*/ 0x180406a4U,
/*018c*/ 0x000106a5U,
/*018d*/ 0x080806a5U,
/*018e*/ 0x100106a5U,
/*018f*/ 0x180506a5U,
/*0190*/ 0x000106a6U,
/*0191*/ 0x081406a6U,
/*0192*/ 0x000a06a7U,
/*0193*/ 0x100c06a7U,
/*0194*/ 0x001206a8U,
/*0195*/ 0x001406a9U,
/*0196*/ 0x001206aaU,
/*0197*/ 0x001106abU,
/*0198*/ 0x001106acU,
/*0199*/ 0x001206adU,
/*019a*/ 0x001206aeU,
/*019b*/ 0x001206afU,
/*019c*/ 0x001206b0U,
/*019d*/ 0x001206b1U,
/*019e*/ 0x001206b2U,
/*019f*/ 0x001206b3U,
/*01a0*/ 0x001206b4U,
/*01a1*/ 0x001206b5U,
/*01a2*/ 0x001206b6U,
/*01a3*/ 0x000e06b7U,
/*01a4*/ 0x100d06b7U,
/*01a5*/ 0x002006b8U,
/*01a6*/ 0x001706b9U,
/*01a7*/ 0x000906baU,
/*01a8*/ 0x100106baU,
/*01a9*/ 0x180106baU,
/*01aa*/ 0x002006bbU,
/*01ab*/ 0x000806bcU,
/*01ac*/ 0x080306bcU,
/*01ad*/ 0x100306bcU,
/*01ae*/ 0x001806bdU,
/*01af*/ 0x001806beU,
/*01b0*/ 0x180706beU,
/*01b1*/ 0x000506bfU,
/*01b2*/ 0x080806bfU,
/*01b3*/ 0x100806bfU,
/*01b4*/ 0x180806bfU,
/*01b5*/ 0x000106c0U,
/*01b6*/ 0x080106c0U,
/*01b7*/ 0x002006c1U,
/*01b8*/ 0xffffffffU,
/*01b9*/ 0xffffffffU,
/*01ba*/ 0xffffffffU,
/*01bb*/ 0xffffffffU,
/*01bc*/ 0xffffffffU,
/*01bd*/ 0xffffffffU,
/*01be*/ 0xffffffffU,
/*01bf*/ 0x001006c2U,
/*01c0*/ 0x100106c2U,
/*01c1*/ 0x180106c2U,
/*01c2*/ 0x000206c3U,
/*01c3*/ 0x080406c3U,
/*01c4*/ 0x100906c3U,
/*01c5*/ 0x000706c4U,
/*01c6*/ 0x080406c4U,
/*01c7*/ 0x002006c5U,
/*01c8*/ 0x000106c6U,
/*01c9*/ 0x080206c6U,
/*01ca*/ 0x100606c6U,
/*01cb*/ 0x001006c7U,
/*01cc*/ 0x100106c7U,
/*01cd*/ 0x002006c8U,
/*01ce*/ 0x000806c9U,
/*01cf*/ 0x080106c9U,
/*01d0*/ 0x100506c9U,
/*01d1*/ 0xffffffffU,
/*01d2*/ 0x180206c9U,
/*01d3*/ 0x000106caU,
/*01d4*/ 0x002006cbU,
/*01d5*/ 0x000b06ccU,
/*01d6*/ 0x100106ccU,
/*01d7*/ 0x180306ccU,
/*01d8*/ 0x000806cdU,
/*01d9*/ 0x080206cdU,
/*01da*/ 0x100c06cdU,
/*01db*/ 0x000406ceU,
/*01dc*/ 0x080106ceU,
/*01dd*/ 0xffffffffU,
/*01de*/ 0x00010200U,
/*01df*/ 0x08040200U,
/*01e0*/ 0x10100200U,
/*01e1*/ 0x00010201U,
/*01e2*/ 0x08010201U,
/*01e3*/ 0x10010201U,
/*01e4*/ 0xffffffffU,
/*01e5*/ 0x00100202U,
/*01e6*/ 0x10080202U,
/*01e7*/ 0xffffffffU,
/*01e8*/ 0xffffffffU,
/*01e9*/ 0xffffffffU,
/*01ea*/ 0xffffffffU,
/*01eb*/ 0xffffffffU,
/*01ec*/ 0xffffffffU,
/*01ed*/ 0xffffffffU,
/*01ee*/ 0xffffffffU,
/*01ef*/ 0x00200203U,
/*01f0*/ 0x00100204U,
/*01f1*/ 0x00200205U,
/*01f2*/ 0x00100206U,
/*01f3*/ 0x00200207U,
/*01f4*/ 0x00100208U,
/*01f5*/ 0x00140209U,
/*01f6*/ 0x0020020aU,
/*01f7*/ 0x0020020bU,
/*01f8*/ 0x0020020cU,
/*01f9*/ 0x0020020dU,
/*01fa*/ 0x0014020eU,
/*01fb*/ 0x0020020fU,
/*01fc*/ 0x00200210U,
/*01fd*/ 0x00200211U,
/*01fe*/ 0x00200212U,
/*01ff*/ 0x00140213U,
/*0200*/ 0x00200214U,
/*0201*/ 0x00200215U,
/*0202*/ 0x00200216U,
/*0203*/ 0x00200217U,
/*0204*/ 0x00090218U,
/*0205*/ 0x10010218U,
/*0206*/ 0x00200219U,
/*0207*/ 0x0005021aU,
/*0208*/ 0x0801021aU,
/*0209*/ 0x1008021aU,
/*020a*/ 0x1808021aU,
/*020b*/ 0x001c021bU,
/*020c*/ 0x001c021cU,
/*020d*/ 0x001c021dU,
/*020e*/ 0x001c021eU,
/*020f*/ 0x001c021fU,
/*0210*/ 0x001c0220U,
/*0211*/ 0x001c0221U,
/*0212*/ 0x001c0222U,
/*0213*/ 0x001c0223U,
/*0214*/ 0x001c0224U,
/*0215*/ 0x001c0225U,
/*0216*/ 0x001c0226U,
/*0217*/ 0x001c0227U,
/*0218*/ 0x001c0228U,
/*0219*/ 0x001c0229U,
/*021a*/ 0x001c022aU,
/*021b*/ 0x0001022bU,
/*021c*/ 0x0801022bU,
/*021d*/ 0x1001022bU,
/*021e*/ 0x1804022bU,
/*021f*/ 0x0008022cU,
/*0220*/ 0x0808022cU,
/*0221*/ 0x1008022cU,
/*0222*/ 0x1804022cU,
/*0223*/ 0x0007022dU,
/*0224*/ 0xffffffffU,
/*0225*/ 0x0807022dU,
/*0226*/ 0x1007022dU,
/*0227*/ 0xffffffffU,
/*0228*/ 0x1807022dU,
/*0229*/ 0x0007022eU,
/*022a*/ 0xffffffffU,
/*022b*/ 0x0807022eU,
/*022c*/ 0x1002022eU,
/*022d*/ 0x1801022eU,
/*022e*/ 0x0001022fU,
/*022f*/ 0x080a022fU,
/*0230*/ 0x00140230U,
/*0231*/ 0x000a0231U,
/*0232*/ 0x00140232U,
/*0233*/ 0x000a0233U,
/*0234*/ 0x00140234U,
/*0235*/ 0x18010234U,
/*0236*/ 0x00100235U,
/*0237*/ 0x10050235U,
/*0238*/ 0x18010235U,
/*0239*/ 0x00010236U,
/*023a*/ 0x08010236U,
/*023b*/ 0x10010236U,
/*023c*/ 0x18010236U,
/*023d*/ 0x00010237U,
/*023e*/ 0x08010237U,
/*023f*/ 0x10020237U,
/*0240*/ 0x18020237U,
/*0241*/ 0x00020238U,
/*0242*/ 0x08020238U,
/*0243*/ 0x10020238U,
/*0244*/ 0x18030238U,
/*0245*/ 0x00010239U,
/*0246*/ 0x08010239U,
/*0247*/ 0x10010239U,
/*0248*/ 0x18010239U,
/*0249*/ 0xffffffffU,
/*024a*/ 0x0002023aU,
/*024b*/ 0x0801023aU,
/*024c*/ 0x1001023aU,
/*024d*/ 0xffffffffU,
/*024e*/ 0x1802023aU,
/*024f*/ 0x0001023bU,
/*0250*/ 0x0801023bU,
/*0251*/ 0xffffffffU,
/*0252*/ 0x1002023bU,
/*0253*/ 0x1801023bU,
/*0254*/ 0x0001023cU,
/*0255*/ 0xffffffffU,
/*0256*/ 0x0802023cU,
/*0257*/ 0x1007023cU,
/*0258*/ 0x1801023cU,
/*0259*/ 0x0001023dU,
/*025a*/ 0x0801023dU,
/*025b*/ 0x1001023dU,
/*025c*/ 0x1801023dU,
/*025d*/ 0x0001023eU,
/*025e*/ 0x0801023eU,
/*025f*/ 0x1001023eU,
/*0260*/ 0x1804023eU,
/*0261*/ 0x0004023fU,
/*0262*/ 0x0804023fU,
/*0263*/ 0x1001023fU,
/*0264*/ 0x1802023fU,
/*0265*/ 0x00060240U,
/*0266*/ 0x08060240U,
/*0267*/ 0x10020240U,
/*0268*/ 0x18020240U,
/*0269*/ 0x00020241U,
/*026a*/ 0xffffffffU,
/*026b*/ 0x08100241U,
/*026c*/ 0x18010241U,
/*026d*/ 0x00010242U,
/*026e*/ 0x08010242U,
/*026f*/ 0x10040242U,
/*0270*/ 0x18010242U,
/*0271*/ 0x00040243U,
/*0272*/ 0x08020243U,
/*0273*/ 0x10080243U,
/*0274*/ 0xffffffffU,
/*0275*/ 0xffffffffU,
/*0276*/ 0xffffffffU,
/*0277*/ 0x000a0244U,
/*0278*/ 0x00200245U,
/*0279*/ 0x00200246U,
/*027a*/ 0x00050247U,
/*027b*/ 0x08010247U,
/*027c*/ 0x10050247U,
/*027d*/ 0x18080247U,
/*027e*/ 0x00010248U,
/*027f*/ 0x08080248U,
/*0280*/ 0x10010248U,
/*0281*/ 0x18080248U,
/*0282*/ 0x00010249U,
/*0283*/ 0x08040249U,
/*0284*/ 0x10040249U,
/*0285*/ 0x18040249U,
/*0286*/ 0x0004024aU,
/*0287*/ 0x0804024aU,
/*0288*/ 0x1004024aU,
/*0289*/ 0x1804024aU,
/*028a*/ 0x0004024bU,
/*028b*/ 0x0804024bU,
/*028c*/ 0x1004024bU,
/*028d*/ 0x1801024bU,
/*028e*/ 0x0004024cU,
/*028f*/ 0x0804024cU,
/*0290*/ 0x1004024cU,
/*0291*/ 0x1804024cU,
/*0292*/ 0x0004024dU,
/*0293*/ 0x0804024dU,
/*0294*/ 0x1006024dU,
/*0295*/ 0x1806024dU,
/*0296*/ 0x0006024eU,
/*0297*/ 0x0806024eU,
/*0298*/ 0x1006024eU,
/*0299*/ 0x1806024eU,
/*029a*/ 0xffffffffU,
/*029b*/ 0x0001024fU,
/*029c*/ 0x0801024fU,
/*029d*/ 0x1002024fU,
/*029e*/ 0xffffffffU,
/*029f*/ 0xffffffffU,
/*02a0*/ 0xffffffffU,
/*02a1*/ 0xffffffffU,
/*02a2*/ 0xffffffffU,
/*02a3*/ 0xffffffffU,
/*02a4*/ 0xffffffffU,
/*02a5*/ 0xffffffffU,
/*02a6*/ 0x1804024fU,
/*02a7*/ 0x00040250U,
/*02a8*/ 0x08010250U,
/*02a9*/ 0x10010250U,
/*02aa*/ 0x18010250U,
/*02ab*/ 0x00010251U,
/*02ac*/ 0x08010251U,
/*02ad*/ 0x10010251U,
/*02ae*/ 0x18010251U,
/*02af*/ 0x00010252U,
/*02b0*/ 0x08010252U,
/*02b1*/ 0x10040252U,
/*02b2*/ 0x18040252U,
/*02b3*/ 0x000a0253U,
/*02b4*/ 0x00200254U,
/*02b5*/ 0x00040255U,
/*02b6*/ 0x08080255U,
/*02b7*/ 0x10020255U,
/*02b8*/ 0x18020255U,
/*02b9*/ 0x00020256U,
/*02ba*/ 0x08020256U,
/*02bb*/ 0x10020256U,
/*02bc*/ 0x18020256U,
/*02bd*/ 0xffffffffU,
/*02be*/ 0xffffffffU,
/*02bf*/ 0x00200257U,
/*02c0*/ 0x00020258U,
/*02c1*/ 0x08100258U,
/*02c2*/ 0x00100259U,
/*02c3*/ 0x10040259U,
/*02c4*/ 0x18040259U,
/*02c5*/ 0x0005025aU,
/*02c6*/ 0x0805025aU,
/*02c7*/ 0x0020025bU,
/*02c8*/ 0x0020025cU,
/*02c9*/ 0x0020025dU,
/*02ca*/ 0x0020025eU,
/*02cb*/ 0x0001025fU,
/*02cc*/ 0x0801025fU,
/*02cd*/ 0x1007025fU,
/*02ce*/ 0x1807025fU,
/*02cf*/ 0x00070260U,
/*02d0*/ 0x08070260U,
/*02d1*/ 0x10070260U,
/*02d2*/ 0x18070260U,
/*02d3*/ 0x00070261U,
/*02d4*/ 0x08070261U,
/*02d5*/ 0x10070261U,
/*02d6*/ 0x18070261U,
/*02d7*/ 0x00070262U,
/*02d8*/ 0x08070262U,
/*02d9*/ 0x10070262U,
/*02da*/ 0x18070262U,
/*02db*/ 0x00030263U,
/*02dc*/ 0x08030263U,
/*02dd*/ 0x10030263U,
/*02de*/ 0xffffffffU,
/*02df*/ 0x18010263U,
/*02e0*/ 0x00020264U,
/*02e1*/ 0x08010264U,
/*02e2*/ 0x10040264U,
/*02e3*/ 0x18020264U,
/*02e4*/ 0x00010265U,
/*02e5*/ 0x08010265U,
/*02e6*/ 0x10010265U,
/*02e7*/ 0x18010265U,
/*02e8*/ 0x00040266U,
/*02e9*/ 0x08080266U,
/*02ea*/ 0x100a0266U,
/*02eb*/ 0x000a0267U,
/*02ec*/ 0x100a0267U,
/*02ed*/ 0x000a0268U,
/*02ee*/ 0x100a0268U,
/*02ef*/ 0x000a0269U,
/*02f0*/ 0x0020026aU,
/*02f1*/ 0x0020026bU,
/*02f2*/ 0x0001026cU,
/*02f3*/ 0x0802026cU,
/*02f4*/ 0x1002026cU,
/*02f5*/ 0x1802026cU,
/*02f6*/ 0xffffffffU,
/*02f7*/ 0x0002026dU,
/*02f8*/ 0x0810026dU,
/*02f9*/ 0x1805026dU,
/*02fa*/ 0x0006026eU,
/*02fb*/ 0x0805026eU,
/*02fc*/ 0x1005026eU,
/*02fd*/ 0x000e026fU,
/*02fe*/ 0x1005026fU,
/*02ff*/ 0x000e0270U,
/*0300*/ 0x10050270U,
/*0301*/ 0x000e0271U,
/*0302*/ 0x10050271U,
/*0303*/ 0x18010271U,
/*0304*/ 0x00050272U,
/*0305*/ 0x08050272U,
/*0306*/ 0x100a0272U,
/*0307*/ 0x000a0273U,
/*0308*/ 0x10050273U,
/*0309*/ 0x18050273U,
/*030a*/ 0x000a0274U,
/*030b*/ 0x100a0274U,
/*030c*/ 0x00050275U,
/*030d*/ 0x08050275U,
/*030e*/ 0x100a0275U,
/*030f*/ 0x000a0276U,
/*0310*/ 0xffffffffU,
/*0311*/ 0xffffffffU,
/*0312*/ 0xffffffffU,
/*0313*/ 0xffffffffU,
/*0314*/ 0xffffffffU,
/*0315*/ 0xffffffffU,
/*0316*/ 0x10070276U,
/*0317*/ 0x18070276U,
/*0318*/ 0x00040277U,
/*0319*/ 0x08040277U,
/*031a*/ 0xffffffffU,
/*031b*/ 0xffffffffU,
/*031c*/ 0xffffffffU,
/*031d*/ 0x10040277U,
/*031e*/ 0x18080277U,
/*031f*/ 0x00080278U,
/*0320*/ 0x08040278U,
/*0321*/ 0xffffffffU,
/*0322*/ 0xffffffffU,
/*0323*/ 0xffffffffU,
/*0324*/ 0x10040278U,
/*0325*/ 0xffffffffU,
/*0326*/ 0xffffffffU,
/*0327*/ 0xffffffffU,
/*0328*/ 0x18040278U,
/*0329*/ 0xffffffffU,
/*032a*/ 0xffffffffU,
/*032b*/ 0xffffffffU,
/*032c*/ 0x00040279U,
/*032d*/ 0x08050279U,
/*032e*/ 0x10070279U,
/*032f*/ 0x18080279U,
/*0330*/ 0x0010027aU,
/*0331*/ 0x1008027aU,
/*0332*/ 0x0010027bU,
/*0333*/ 0x1008027bU,
/*0334*/ 0x0010027cU,
/*0335*/ 0x1008027cU,
/*0336*/ 0x1808027cU,
/*0337*/ 0x0001027dU,
/*0338*/ 0x0801027dU,
/*0339*/ 0x1006027dU,
/*033a*/ 0x1806027dU,
/*033b*/ 0x0006027eU,
/*033c*/ 0x0801027eU,
/*033d*/ 0x1001027eU,
/*033e*/ 0x1803027eU,
/*033f*/ 0x000a027fU,
/*0340*/ 0x100a027fU,
/*0341*/ 0x000a0280U,
/*0342*/ 0xffffffffU,
/*0343*/ 0x100a0280U,
/*0344*/ 0x00040281U,
/*0345*/ 0x08010281U,
/*0346*/ 0x10040281U,
/*0347*/ 0xffffffffU,
/*0348*/ 0xffffffffU,
/*0349*/ 0xffffffffU,
/*034a*/ 0xffffffffU,
/*034b*/ 0xffffffffU,
/*034c*/ 0xffffffffU,
/*034d*/ 0x18070281U,
/*034e*/ 0x00070282U,
/*034f*/ 0x08050282U,
/*0350*/ 0x10050282U,
/*0351*/ 0xffffffffU,
/*0352*/ 0xffffffffU,
/*0353*/ 0xffffffffU,
/*0354*/ 0x18040282U,
/*0355*/ 0x00010283U,
/*0356*/ 0x08010283U,
/*0357*/ 0x10020283U,
/*0358*/ 0x18080283U,
/*0359*/ 0x00200284U,
/*035a*/ 0x00200285U,
/*035b*/ 0x00100286U,
/*035c*/ 0x10020286U,
/*035d*/ 0x18020286U,
/*035e*/ 0x00020287U,
/*035f*/ 0xffffffffU,
/*0360*/ 0x08010287U,
/*0361*/ 0x10010287U,
/*0362*/ 0x18020287U,
/*0363*/ 0x00080288U,
/*0364*/ 0x08080288U,
/*0365*/ 0x10080288U,
/*0366*/ 0x18080288U,
/*0367*/ 0x00080289U,
/*0368*/ 0x08080289U,
/*0369*/ 0xffffffffU,
/*036a*/ 0x10080289U,
/*036b*/ 0x18080289U,
/*036c*/ 0x0008028aU,
/*036d*/ 0x0808028aU,
/*036e*/ 0x1008028aU,
/*036f*/ 0x1808028aU,
/*0370*/ 0xffffffffU,
/*0371*/ 0x0008028bU,
/*0372*/ 0x0808028bU,
/*0373*/ 0x1008028bU,
/*0374*/ 0x1808028bU,
/*0375*/ 0x0008028cU,
/*0376*/ 0x0808028cU,
/*0377*/ 0xffffffffU,
/*0378*/ 0x1008028cU,
/*0379*/ 0x1808028cU,
/*037a*/ 0x0008028dU,
/*037b*/ 0x0808028dU,
/*037c*/ 0x1008028dU,
/*037d*/ 0x1808028dU,
/*037e*/ 0x0008028eU,
/*037f*/ 0xffffffffU,
/*0380*/ 0x0808028eU,
/*0381*/ 0x1008028eU,
/*0382*/ 0x1808028eU,
/*0383*/ 0x0008028fU,
/*0384*/ 0x0808028fU,
/*0385*/ 0x1008028fU,
/*0386*/ 0xffffffffU,
/*0387*/ 0x1808028fU,
/*0388*/ 0x00080290U,
/*0389*/ 0x08080290U,
/*038a*/ 0x10080290U,
/*038b*/ 0x18080290U,
/*038c*/ 0x00080291U,
/*038d*/ 0xffffffffU,
/*038e*/ 0x08080291U,
/*038f*/ 0x10080291U,
/*0390*/ 0x18080291U,
/*0391*/ 0x00080292U,
/*0392*/ 0x08080292U,
/*0393*/ 0x10080292U,
/*0394*/ 0x18080292U,
/*0395*/ 0xffffffffU,
/*0396*/ 0x00080293U,
/*0397*/ 0x08080293U,
/*0398*/ 0x10080293U,
/*0399*/ 0x18080293U,
/*039a*/ 0x00080294U,
/*039b*/ 0x08080294U,
/*039c*/ 0xffffffffU,
/*039d*/ 0x10080294U,
/*039e*/ 0x18080294U,
/*039f*/ 0x00080295U,
/*03a0*/ 0x08080295U,
/*03a1*/ 0x10080295U,
/*03a2*/ 0x18080295U,
/*03a3*/ 0xffffffffU,
/*03a4*/ 0x00080296U,
/*03a5*/ 0x08080296U,
/*03a6*/ 0x10080296U,
/*03a7*/ 0x18080296U,
/*03a8*/ 0x00080297U,
/*03a9*/ 0x08080297U,
/*03aa*/ 0x10080297U,
/*03ab*/ 0xffffffffU,
/*03ac*/ 0x18080297U,
/*03ad*/ 0x00080298U,
/*03ae*/ 0x08080298U,
/*03af*/ 0x10080298U,
/*03b0*/ 0x18080298U,
/*03b1*/ 0x00080299U,
/*03b2*/ 0xffffffffU,
/*03b3*/ 0x08080299U,
/*03b4*/ 0x10080299U,
/*03b5*/ 0x18080299U,
/*03b6*/ 0x0008029aU,
/*03b7*/ 0x0808029aU,
/*03b8*/ 0x1008029aU,
/*03b9*/ 0xffffffffU,
/*03ba*/ 0x1808029aU,
/*03bb*/ 0x0002029bU,
/*03bc*/ 0x0803029bU,
/*03bd*/ 0x100a029bU,
/*03be*/ 0x000a029cU,
/*03bf*/ 0x100a029cU,
/*03c0*/ 0x0005029dU,
/*03c1*/ 0x0808029dU,
/*03c2*/ 0x1008029dU,
/*03c3*/ 0x1808029dU,
/*03c4*/ 0x0006029eU,
/*03c5*/ 0x0806029eU,
/*03c6*/ 0x0011029fU,
/*03c7*/ 0x1808029fU,
/*03c8*/ 0x000402a0U,
/*03c9*/ 0x080602a0U,
/*03ca*/ 0xffffffffU,
/*03cb*/ 0x100602a0U,
/*03cc*/ 0x180802a0U,
/*03cd*/ 0xffffffffU,
/*03ce*/ 0x000802a1U,
/*03cf*/ 0x080802a1U,
/*03d0*/ 0x100802a1U,
/*03d1*/ 0x180602a1U,
/*03d2*/ 0x000602a2U,
/*03d3*/ 0x081102a2U,
/*03d4*/ 0x000802a3U,
/*03d5*/ 0x080402a3U,
/*03d6*/ 0x100602a3U,
/*03d7*/ 0xffffffffU,
/*03d8*/ 0x180602a3U,
/*03d9*/ 0x000802a4U,
/*03da*/ 0xffffffffU,
/*03db*/ 0x080802a4U,
/*03dc*/ 0x100802a4U,
/*03dd*/ 0x180802a4U,
/*03de*/ 0x000602a5U,
/*03df*/ 0x080602a5U,
/*03e0*/ 0x001102a6U,
/*03e1*/ 0x180802a6U,
/*03e2*/ 0x000402a7U,
/*03e3*/ 0x080602a7U,
/*03e4*/ 0xffffffffU,
/*03e5*/ 0x100602a7U,
/*03e6*/ 0x180802a7U,
/*03e7*/ 0xffffffffU,
/*03e8*/ 0x000402a8U,
/*03e9*/ 0x080402a8U,
/*03ea*/ 0x100402a8U,
/*03eb*/ 0x180402a8U,
/*03ec*/ 0x000402a9U,
/*03ed*/ 0x080402a9U,
/*03ee*/ 0x100402a9U,
/*03ef*/ 0x180402a9U,
/*03f0*/ 0x000402aaU,
/*03f1*/ 0x080402aaU,
/*03f2*/ 0x100402aaU,
/*03f3*/ 0x180402aaU,
/*03f4*/ 0x000402abU,
/*03f5*/ 0x080402abU,
/*03f6*/ 0x100402abU,
/*03f7*/ 0x180402abU,
/*03f8*/ 0x000402acU,
/*03f9*/ 0x080402acU,
/*03fa*/ 0x100402acU,
/*03fb*/ 0x180402acU,
/*03fc*/ 0x001202adU,
/*03fd*/ 0x001102aeU,
/*03fe*/ 0x001202afU,
/*03ff*/ 0x002002b0U,
/*0400*/ 0x002002b1U,
/*0401*/ 0x002002b2U,
/*0402*/ 0x002002b3U,
/*0403*/ 0x002002b4U,
/*0404*/ 0x002002b5U,
/*0405*/ 0x002002b6U,
/*0406*/ 0x002002b7U,
/*0407*/ 0x002002b8U,
/*0408*/ 0x000202b9U,
/*0409*/ 0x080502b9U,
/*040a*/ 0x100502b9U,
/*040b*/ 0x180102b9U,
/*040c*/ 0x000402baU,
/*040d*/ 0x080402baU,
/*040e*/ 0x100402baU,
/*040f*/ 0x180402baU,
/*0410*/ 0x000402bbU,
/*0411*/ 0x080402bbU,
/*0412*/ 0x100402bbU,
/*0413*/ 0x180402bbU,
/*0414*/ 0xffffffffU,
/*0415*/ 0xffffffffU,
/*0416*/ 0xffffffffU,
/*0417*/ 0xffffffffU,
/*0418*/ 0xffffffffU,
/*0419*/ 0xffffffffU,
/*041a*/ 0x000402bcU,
/*041b*/ 0x080402bcU,
/*041c*/ 0x100402bcU,
/*041d*/ 0x180402bcU,
/*041e*/ 0x000402bdU,
/*041f*/ 0x080402bdU,
/*0420*/ 0x100402bdU,
/*0421*/ 0x180402bdU,
/*0422*/ 0x000102beU,
/*0423*/ 0x080202beU,
/*0424*/ 0x100202beU,
/*0425*/ 0x180202beU,
/*0426*/ 0x000202bfU,
/*0427*/ 0x080102bfU,
/*0428*/ 0x100402bfU,
/*0429*/ 0x001002c0U,
/*042a*/ 0x002002c1U,
/*042b*/ 0x001002c2U,
/*042c*/ 0x002002c3U,
/*042d*/ 0x001002c4U,
/*042e*/ 0x002002c5U,
/*042f*/ 0x000702c6U,
/*0430*/ 0x080102c6U,
/*0431*/ 0x100202c6U,
/*0432*/ 0x180602c6U,
/*0433*/ 0x000102c7U,
/*0434*/ 0x080102c7U,
/*0435*/ 0x002002c8U,
/*0436*/ 0x000202c9U,
/*0437*/ 0x002002caU,
/*0438*/ 0x002002cbU,
/*0439*/ 0x000c02ccU,
/*043a*/ 0x100c02ccU,
/*043b*/ 0x002002cdU,
/*043c*/ 0x000302ceU,
/*043d*/ 0x002002cfU,
/*043e*/ 0x000302d0U,
/*043f*/ 0x002002d1U,
/*0440*/ 0x000302d2U,
/*0441*/ 0x002002d3U,
/*0442*/ 0x000302d4U,
/*0443*/ 0x002002d5U,
/*0444*/ 0x000302d6U,
/*0445*/ 0x002002d7U,
/*0446*/ 0x000302d8U,
/*0447*/ 0x002002d9U,
/*0448*/ 0x000302daU,
/*0449*/ 0x002002dbU,
/*044a*/ 0x000302dcU,
/*044b*/ 0x002002ddU,
/*044c*/ 0x000302deU,
/*044d*/ 0x002002dfU,
/*044e*/ 0x000302e0U,
/*044f*/ 0x080302e0U,
/*0450*/ 0x100202e0U,
/*0451*/ 0x180202e0U,
/*0452*/ 0x002002e1U,
/*0453*/ 0x002002e2U,
/*0454*/ 0x002002e3U,
/*0455*/ 0x002002e4U,
/*0456*/ 0x000402e5U,
/*0457*/ 0x001e02e6U,
/*0458*/ 0x001e02e7U,
/*0459*/ 0x001e02e8U,
/*045a*/ 0x001e02e9U,
/*045b*/ 0x001e02eaU,
/*045c*/ 0x001e02ebU,
/*045d*/ 0x001e02ecU,
/*045e*/ 0x001e02edU,
/*045f*/ 0x000402eeU,
/*0460*/ 0xffffffffU,
/*0461*/ 0xffffffffU,
/*0462*/ 0xffffffffU,
/*0463*/ 0xffffffffU,
/*0464*/ 0x080402eeU,
/*0465*/ 0x100102eeU,
/*0466*/ 0x180802eeU,
/*0467*/ 0x000402efU,
/*0468*/ 0x080102efU,
/*0469*/ 0x100802efU,
/*046a*/ 0x180402efU,
/*046b*/ 0x000102f0U,
/*046c*/ 0x080802f0U,
/*046d*/ 0x100402f0U,
/*046e*/ 0x180102f0U,
/*046f*/ 0x000802f1U,
/*0470*/ 0x080402f1U,
/*0471*/ 0x100102f1U,
/*0472*/ 0x180802f1U,
/*0473*/ 0x000402f2U,
/*0474*/ 0x080102f2U,
/*0475*/ 0x100802f2U,
/*0476*/ 0x180402f2U,
/*0477*/ 0x000102f3U,
/*0478*/ 0x080802f3U,
/*0479*/ 0x100402f3U,
/*047a*/ 0x180102f3U,
/*047b*/ 0x000802f4U,
/*047c*/ 0x080802f4U,
/*047d*/ 0x100102f4U,
/*047e*/ 0x180502f4U,
/*047f*/ 0xffffffffU,
/*0480*/ 0xffffffffU,
/*0481*/ 0xffffffffU,
/*0482*/ 0xffffffffU,
/*0483*/ 0xffffffffU,
/*0484*/ 0xffffffffU,
/*0485*/ 0xffffffffU,
/*0486*/ 0xffffffffU,
/*0487*/ 0xffffffffU,
/*0488*/ 0xffffffffU,
/*0489*/ 0xffffffffU,
/*048a*/ 0xffffffffU,
/*048b*/ 0xffffffffU,
/*048c*/ 0xffffffffU,
/*048d*/ 0xffffffffU,
/*048e*/ 0xffffffffU,
/*048f*/ 0xffffffffU,
/*0490*/ 0xffffffffU,
/*0491*/ 0xffffffffU,
/*0492*/ 0xffffffffU,
/*0493*/ 0xffffffffU,
/*0494*/ 0xffffffffU,
	 },
	{
/*0000*/ 0x00200800U,
/*0001*/ 0x00040801U,
/*0002*/ 0x080b0801U,
/*0003*/ 0x000a0802U,
/*0004*/ 0x10020802U,
/*0005*/ 0x18010802U,
/*0006*/ 0x00060803U,
/*0007*/ 0x08060803U,
/*0008*/ 0x10060803U,
/*0009*/ 0x18060803U,
/*000a*/ 0x00060804U,
/*000b*/ 0x08060804U,
/*000c*/ 0x10050804U,
/*000d*/ 0x18060804U,
/*000e*/ 0x00060805U,
/*000f*/ 0x08040805U,
/*0010*/ 0x10030805U,
/*0011*/ 0x00180806U,
/*0012*/ 0x18030806U,
/*0013*/ 0x00180807U,
/*0014*/ 0x18020807U,
/*0015*/ 0x0801085eU,
/*0016*/ 0x00020808U,
/*0017*/ 0x08010808U,
/*0018*/ 0x10010808U,
/*0019*/ 0x18020808U,
/*001a*/ 0x00050809U,
/*001b*/ 0x08050809U,
/*001c*/ 0x10040809U,
/*001d*/ 0xffffffffU,
/*001e*/ 0x18040809U,
/*001f*/ 0x0002080aU,
/*0020*/ 0x0805080aU,
/*0021*/ 0x1009080aU,
/*0022*/ 0x0001080bU,
/*0023*/ 0x0020080cU,
/*0024*/ 0x001c080dU,
/*0025*/ 0x0001080eU,
/*0026*/ 0x0807080eU,
/*0027*/ 0x1009080eU,
/*0028*/ 0x000a080fU,
/*0029*/ 0x1005080fU,
/*002a*/ 0x1801080fU,
/*002b*/ 0x10010810U,
/*002c*/ 0x18020810U,
/*002d*/ 0x00090810U,
/*002e*/ 0x00090811U,
/*002f*/ 0x10020811U,
/*0030*/ 0x00200812U,
/*0031*/ 0x00010813U,
/*0032*/ 0x08020813U,
/*0033*/ 0x00200814U,
/*0034*/ 0x00200815U,
/*0035*/ 0x00200816U,
/*0036*/ 0x00200817U,
/*0037*/ 0xffffffffU,
/*0038*/ 0xffffffffU,
/*0039*/ 0xffffffffU,
/*003a*/ 0xffffffffU,
/*003b*/ 0x00030818U,
/*003c*/ 0x08010818U,
/*003d*/ 0x10040818U,
/*003e*/ 0x18030818U,
/*003f*/ 0x00040819U,
/*0040*/ 0x08040819U,
/*0041*/ 0x10040819U,
/*0042*/ 0x18040819U,
/*0043*/ 0x0001081aU,
/*0044*/ 0x0801081aU,
/*0045*/ 0x1006081aU,
/*0046*/ 0x1804081aU,
/*0047*/ 0x0008081bU,
/*0048*/ 0x0806081bU,
/*0049*/ 0x1004081bU,
/*004a*/ 0x1806081bU,
/*004b*/ 0x0004081cU,
/*004c*/ 0x0802081cU,
/*004d*/ 0x1005081cU,
/*004e*/ 0x1808081cU,
/*004f*/ 0xffffffffU,
/*0050*/ 0x0006081dU,
/*0051*/ 0x0803081dU,
/*0052*/ 0x100b081dU,
/*0053*/ 0x0004081eU,
/*0054*/ 0x0804081eU,
/*0055*/ 0x1004081eU,
/*0056*/ 0x1801081eU,
/*0057*/ 0xffffffffU,
/*0058*/ 0x0009081fU,
/*0059*/ 0x00200820U,
/*005a*/ 0x00200821U,
/*005b*/ 0x00200822U,
/*005c*/ 0x00200823U,
/*005d*/ 0x00100824U,
/*005e*/ 0xffffffffU,
/*005f*/ 0x10010824U,
/*0060*/ 0x18060824U,
/*0061*/ 0x00080825U,
/*0062*/ 0x00200826U,
/*0063*/ 0x00100827U,
/*0064*/ 0x100b0827U,
/*0065*/ 0x00070828U,
/*0066*/ 0x08070828U,
/*0067*/ 0x10090828U,
/*0068*/ 0x00090829U,
/*0069*/ 0x100b0829U,
/*006a*/ 0x0007082aU,
/*006b*/ 0x0808082aU,
/*006c*/ 0x1009082aU,
/*006d*/ 0x0003082bU,
/*006e*/ 0x080a082bU,
/*006f*/ 0x000a082cU,
/*0070*/ 0x0011082dU,
/*0071*/ 0x000a082eU,
/*0072*/ 0x100a082eU,
/*0073*/ 0x0010082fU,
/*0074*/ 0x100e082fU,
/*0075*/ 0x000e0830U,
/*0076*/ 0x00120831U,
/*0077*/ 0x000a0832U,
/*0078*/ 0x100a0832U,
/*0079*/ 0x00020833U,
/*007a*/ 0x00200834U,
/*007b*/ 0x000b0835U,
/*007c*/ 0x100b0835U,
/*007d*/ 0x00200836U,
/*007e*/ 0x00130837U,
/*007f*/ 0x00200838U,
/*0080*/ 0x00200839U,
/*0081*/ 0x0008083aU,
/*0082*/ 0x0801083aU,
/*0083*/ 0x1001083aU,
/*0084*/ 0x1801083aU,
/*0085*/ 0x0008083bU,
/*0086*/ 0x080c083bU,
/*0087*/ 0x000c083cU,
/*0088*/ 0x100c083cU,
/*0089*/ 0x000c083dU,
/*008a*/ 0x100c083dU,
/*008b*/ 0x000c083eU,
/*008c*/ 0x100c083eU,
/*008d*/ 0x000c083fU,
/*008e*/ 0x100c083fU,
/*008f*/ 0x000c0840U,
/*0090*/ 0x100c0840U,
/*0091*/ 0x000b0841U,
/*0092*/ 0x10090841U,
/*0093*/ 0x00010842U,
/*0094*/ 0x000b0843U,
/*0095*/ 0x100b0843U,
/*0096*/ 0x000b0844U,
/*0097*/ 0x100b0844U,
/*0098*/ 0x000b0845U,
/*0099*/ 0x100b0845U,
/*009a*/ 0x000b0846U,
/*009b*/ 0x100b0846U,
/*009c*/ 0x000b0847U,
/*009d*/ 0x100a0847U,
/*009e*/ 0x00020848U,
/*009f*/ 0x080a0848U,
/*00a0*/ 0x000a0849U,
/*00a1*/ 0x100a0849U,
/*00a2*/ 0x000a084aU,
/*00a3*/ 0x100a084aU,
/*00a4*/ 0x000a084bU,
/*00a5*/ 0x100a084bU,
/*00a6*/ 0x000a084cU,
/*00a7*/ 0x100a084cU,
/*00a8*/ 0x000a084dU,
/*00a9*/ 0x100a084dU,
/*00aa*/ 0x000a084eU,
/*00ab*/ 0x100a084eU,
/*00ac*/ 0x000a084fU,
/*00ad*/ 0x100a084fU,
/*00ae*/ 0x000a0850U,
/*00af*/ 0x100a0850U,
/*00b0*/ 0x000a0851U,
/*00b1*/ 0x100a0851U,
/*00b2*/ 0x000a0852U,
/*00b3*/ 0x100a0852U,
/*00b4*/ 0x000a0853U,
/*00b5*/ 0x100a0853U,
/*00b6*/ 0x000a0854U,
/*00b7*/ 0x100a0854U,
/*00b8*/ 0x000a0855U,
/*00b9*/ 0x100a0855U,
/*00ba*/ 0x000a0856U,
/*00bb*/ 0x10040856U,
/*00bc*/ 0x18030856U,
/*00bd*/ 0x000a0857U,
/*00be*/ 0x100a0857U,
/*00bf*/ 0x00010858U,
/*00c0*/ 0x080a0858U,
/*00c1*/ 0x18040858U,
/*00c2*/ 0x000b0859U,
/*00c3*/ 0x100a0859U,
/*00c4*/ 0x0003085aU,
/*00c5*/ 0x0008085bU,
/*00c6*/ 0x0808085bU,
/*00c7*/ 0x1008085bU,
/*00c8*/ 0x1808085bU,
/*00c9*/ 0x0008085cU,
/*00ca*/ 0x0808085cU,
/*00cb*/ 0x1008085cU,
/*00cc*/ 0x1801085cU,
/*00cd*/ 0x0008085dU,
/*00ce*/ 0x0808085dU,
/*00cf*/ 0x1002085dU,
/*00d0*/ 0x1802085dU,
/*00d1*/ 0x0005085eU,
/*00d2*/ 0x1005085eU,
/*00d3*/ 0x1805085eU,
/*00d4*/ 0x0004085fU,
/*00d5*/ 0x080b085fU,
/*00d6*/ 0x1806085fU,
/*00d7*/ 0x00080860U,
/*00d8*/ 0x08080860U,
/*00d9*/ 0x10040860U,
/*00da*/ 0x18040860U,
/*00db*/ 0x00060861U,
/*00dc*/ 0x08040861U,
/*00dd*/ 0x10050861U,
/*00de*/ 0x000a0862U,
/*00df*/ 0x100a0862U,
/*00e0*/ 0x00080863U,
/*00e1*/ 0x08010863U,
/*00e2*/ 0x10040863U,
/*00e3*/ 0x00020864U,
/*00e4*/ 0x08030864U,
/*00e5*/ 0x00050a00U,
/*00e6*/ 0x08050a00U,
/*00e7*/ 0x10050a00U,
/*00e8*/ 0x18050a00U,
/*00e9*/ 0x00050a01U,
/*00ea*/ 0x08050a01U,
/*00eb*/ 0x100b0a01U,
/*00ec*/ 0x00010a02U,
/*00ed*/ 0x08030a02U,
/*00ee*/ 0x00200a03U,
/*00ef*/ 0x00100a04U,
/*00f0*/ 0x10040a04U,
/*00f1*/ 0x000b0a05U,
/*00f2*/ 0x10070a05U,
/*00f3*/ 0x00090a06U,
/*00f4*/ 0x10030a06U,
/*00f5*/ 0x18030a06U,
/*00f6*/ 0x00010a07U,
/*00f7*/ 0x08010a07U,
/*00f8*/ 0x10070a07U,
/*00f9*/ 0x18070a07U,
/*00fa*/ 0x00050a08U,
/*00fb*/ 0x08010a08U,
/*00fc*/ 0x10020a08U,
/*00fd*/ 0x18030a08U,
/*00fe*/ 0x00010a09U,
/*00ff*/ 0x080f0a09U,
/*0100*/ 0x00200a0aU,
/*0101*/ 0x00200a0bU,
/*0102*/ 0x000b0a0cU,
/*0103*/ 0x100b0a0cU,
/*0104*/ 0x000b0a0dU,
/*0105*/ 0x00180a0eU,
/*0106*/ 0x00180a0fU,
/*0107*/ 0xffffffffU,
/*0108*/ 0xffffffffU,
/*0109*/ 0xffffffffU,
/*010a*/ 0xffffffffU,
/*010b*/ 0xffffffffU,
/*010c*/ 0x18020a0fU,
/*010d*/ 0x00020a10U,
/*010e*/ 0x08040a10U,
/*010f*/ 0x10040a10U,
/*0110*/ 0x18010a10U,
/*0111*/ 0x00010a11U,
/*0112*/ 0x08010a11U,
/*0113*/ 0x10030a11U,
/*0114*/ 0x00200a12U,
/*0115*/ 0x00200a13U,
/*0116*/ 0xffffffffU,
/*0117*/ 0x00140a14U,
/*0118*/ 0x00140a15U,
/*0119*/ 0x00140a16U,
/*011a*/ 0x00140a17U,
/*011b*/ 0x00140a18U,
/*011c*/ 0x00140a19U,
/*011d*/ 0x00140a1aU,
/*011e*/ 0x00140a1bU,
/*011f*/ 0x001e0a1cU,
/*0120*/ 0x000a0a1dU,
/*0121*/ 0x10060a1dU,
/*0122*/ 0x18060a1dU,
/*0123*/ 0x00060a1eU,
/*0124*/ 0x08060a1eU,
/*0125*/ 0x10060a1eU,
/*0126*/ 0x00080a1fU,
/*0127*/ 0x080b0a1fU,
/*0128*/ 0x000b0a20U,
/*0129*/ 0x100b0a20U,
/*012a*/ 0x000b0a21U,
/*012b*/ 0x100b0a21U,
/*012c*/ 0x000b0a22U,
/*012d*/ 0x10040a22U,
/*012e*/ 0x000b0a23U,
/*012f*/ 0x10060a23U,
/*0130*/ 0x18080a23U,
/*0131*/ 0x00080a24U,
/*0132*/ 0x08040a24U,
/*0133*/ 0x00020b80U,
/*0134*/ 0x00010b81U,
/*0135*/ 0x08010b81U,
/*0136*/ 0x10020b81U,
/*0137*/ 0x18050b81U,
/*0138*/ 0x00050b82U,
/*0139*/ 0x08050b82U,
/*013a*/ 0x10050b82U,
/*013b*/ 0x000b0b83U,
/*013c*/ 0x10050b83U,
/*013d*/ 0x18010b83U,
/*013e*/ 0x00010b84U,
/*013f*/ 0x08010b84U,
/*0140*/ 0x10010b84U,
/*0141*/ 0x18010b84U,
/*0142*/ 0x00040b85U,
/*0143*/ 0x080b0b85U,
/*0144*/ 0x000b0b86U,
/*0145*/ 0x100b0b86U,
/*0146*/ 0x00040b87U,
/*0147*/ 0x080b0b87U,
/*0148*/ 0x18040b87U,
/*0149*/ 0x00010b88U,
/*014a*/ 0x08010b88U,
/*014b*/ 0x10010b88U,
/*014c*/ 0x00200b89U,
/*014d*/ 0x00200b8aU,
/*014e*/ 0x00080b8bU,
/*014f*/ 0x080a0b8bU,
/*0150*/ 0x18050b8bU,
/*0151*/ 0x000b0b8cU,
/*0152*/ 0x10030b8cU,
/*0153*/ 0x18030b8cU,
/*0154*/ 0x00010b8dU,
/*0155*/ 0x08020b8dU,
/*0156*/ 0x10010b8dU,
/*0157*/ 0x18010b8dU,
/*0158*/ 0x00010b8eU,
/*0159*/ 0xffffffffU,
/*015a*/ 0x08010b8eU,
/*015b*/ 0x18040b8eU,
/*015c*/ 0x00040b8fU,
/*015d*/ 0x08040b8fU,
/*015e*/ 0x10040b8fU,
/*015f*/ 0x18010b8fU,
/*0160*/ 0x00010b90U,
/*0161*/ 0x08010b90U,
/*0162*/ 0x00200b91U,
/*0163*/ 0x00200b92U,
/*0164*/ 0x00200b93U,
/*0165*/ 0x00200b94U,
/*0166*/ 0xffffffffU,
/*0167*/ 0x10010b8eU,
/*0168*/ 0x000d0b96U,
/*0169*/ 0x100d0b96U,
/*016a*/ 0x000d0b97U,
/*016b*/ 0x00050b98U,
/*016c*/ 0x00010b99U,
/*016d*/ 0x080e0b99U,
/*016e*/ 0x000e0b9aU,
/*016f*/ 0x100e0b9aU,
/*0170*/ 0x000e0b9bU,
/*0171*/ 0x100e0b9bU,
/*0172*/ 0x00040b9cU,
/*0173*/ 0x08040b9cU,
/*0174*/ 0x10040b9cU,
/*0175*/ 0x18040b9cU,
/*0176*/ 0x00040b9dU,
/*0177*/ 0x080b0b9dU,
/*0178*/ 0x000b0b9eU,
/*0179*/ 0x100b0b9eU,
/*017a*/ 0x000b0b9fU,
/*017b*/ 0x00040ba0U,
/*017c*/ 0x08040ba0U,
/*017d*/ 0x10040ba0U,
/*017e*/ 0x18040ba0U,
/*017f*/ 0x000d0ba1U,
/*0180*/ 0x100d0ba1U,
/*0181*/ 0x000d0ba2U,
/*0182*/ 0x10100ba2U,
/*0183*/ 0x00080b95U,
/*0184*/ 0x08080b95U,
/*0185*/ 0x00100ba3U,
/*0186*/ 0x10100ba3U,
/*0187*/ 0x00100ba4U,
/*0188*/ 0x10100ba4U,
/*0189*/ 0x00100ba5U,
/*018a*/ 0x10030ba5U,
/*018b*/ 0x18040ba5U,
/*018c*/ 0x00010ba6U,
/*018d*/ 0x08080ba6U,
/*018e*/ 0x10010ba6U,
/*018f*/ 0x000a0ba7U,
/*0190*/ 0x10010ba7U,
/*0191*/ 0x00140ba8U,
/*0192*/ 0x000b0ba9U,
/*0193*/ 0x100c0ba9U,
/*0194*/ 0x00120baaU,
/*0195*/ 0x00140babU,
/*0196*/ 0x00120bacU,
/*0197*/ 0x00110badU,
/*0198*/ 0x00110baeU,
/*0199*/ 0x00120bafU,
/*019a*/ 0x00120bb0U,
/*019b*/ 0x00120bb1U,
/*019c*/ 0x00120bb2U,
/*019d*/ 0x00120bb3U,
/*019e*/ 0x00120bb4U,
/*019f*/ 0x00120bb5U,
/*01a0*/ 0x00120bb6U,
/*01a1*/ 0x00120bb7U,
/*01a2*/ 0x00120bb8U,
/*01a3*/ 0x000e0bb9U,
/*01a4*/ 0x100d0bb9U,
/*01a5*/ 0x00200bbaU,
/*01a6*/ 0x00170bbbU,
/*01a7*/ 0x000d0bbcU,
/*01a8*/ 0x10010bbcU,
/*01a9*/ 0x18010bbcU,
/*01aa*/ 0x00200bbdU,
/*01ab*/ 0x00080bbeU,
/*01ac*/ 0x08030bbeU,
/*01ad*/ 0x10030bbeU,
/*01ae*/ 0x00180bbfU,
/*01af*/ 0x00180bc0U,
/*01b0*/ 0x18070bc0U,
/*01b1*/ 0x00070bc1U,
/*01b2*/ 0x08080bc1U,
/*01b3*/ 0x10080bc1U,
/*01b4*/ 0x18080bc1U,
/*01b5*/ 0x00010bc2U,
/*01b6*/ 0x08010bc2U,
/*01b7*/ 0x00200bc3U,
/*01b8*/ 0x00070bc4U,
/*01b9*/ 0x08140bc4U,
/*01ba*/ 0x00140bc5U,
/*01bb*/ 0x00190bc6U,
/*01bc*/ 0x00170bc7U,
/*01bd*/ 0x00110bc8U,
/*01be*/ 0x00110bc9U,
/*01bf*/ 0x00100bcaU,
/*01c0*/ 0x10010bcaU,
/*01c1*/ 0x18010bcaU,
/*01c2*/ 0x00020bcbU,
/*01c3*/ 0x08040bcbU,
/*01c4*/ 0x10090bcbU,
/*01c5*/ 0x00070bccU,
/*01c6*/ 0x08040bccU,
/*01c7*/ 0x00200bcdU,
/*01c8*/ 0x00010bceU,
/*01c9*/ 0x08020bceU,
/*01ca*/ 0x10060bceU,
/*01cb*/ 0x00100bcfU,
/*01cc*/ 0x10010bcfU,
/*01cd*/ 0x00200bd0U,
/*01ce*/ 0x00080bd1U,
/*01cf*/ 0x08010bd1U,
/*01d0*/ 0x10050bd1U,
/*01d1*/ 0x18030bd1U,
/*01d2*/ 0x00020bd2U,
/*01d3*/ 0xffffffffU,
/*01d4*/ 0x00200bd3U,
/*01d5*/ 0x000b0bd4U,
/*01d6*/ 0xffffffffU,
/*01d7*/ 0x10030bd4U,
/*01d8*/ 0x18080bd4U,
/*01d9*/ 0x00020bd5U,
/*01da*/ 0x080c0bd5U,
/*01db*/ 0x18040bd5U,
/*01dc*/ 0x00010bd6U,
/*01dd*/ 0x08050bd6U,
/*01de*/ 0x00010200U,
/*01df*/ 0x08040200U,
/*01e0*/ 0x10100200U,
/*01e1*/ 0x00010201U,
/*01e2*/ 0x08010201U,
/*01e3*/ 0x10010201U,
/*01e4*/ 0x18010201U,
/*01e5*/ 0x00100202U,
/*01e6*/ 0x10080202U,
/*01e7*/ 0x18010202U,
/*01e8*/ 0x00200203U,
/*01e9*/ 0x00200204U,
/*01ea*/ 0x00200205U,
/*01eb*/ 0x00200206U,
/*01ec*/ 0x00020207U,
/*01ed*/ 0x08010207U,
/*01ee*/ 0x10010207U,
/*01ef*/ 0x00200208U,
/*01f0*/ 0x00140209U,
/*01f1*/ 0x0020020aU,
/*01f2*/ 0x0014020bU,
/*01f3*/ 0x0020020cU,
/*01f4*/ 0x0014020dU,
/*01f5*/ 0x0014020eU,
/*01f6*/ 0x0020020fU,
/*01f7*/ 0x00200210U,
/*01f8*/ 0x00200211U,
/*01f9*/ 0x00200212U,
/*01fa*/ 0x00140213U,
/*01fb*/ 0x00200214U,
/*01fc*/ 0x00200215U,
/*01fd*/ 0x00200216U,
/*01fe*/ 0x00200217U,
/*01ff*/ 0x00140218U,
/*0200*/ 0x00200219U,
/*0201*/ 0x0020021aU,
/*0202*/ 0x0020021bU,
/*0203*/ 0x0020021cU,
/*0204*/ 0x0009021dU,
/*0205*/ 0x1001021dU,
/*0206*/ 0x0020021eU,
/*0207*/ 0x0005021fU,
/*0208*/ 0x0801021fU,
/*0209*/ 0x1008021fU,
/*020a*/ 0x1808021fU,
/*020b*/ 0x001e0220U,
/*020c*/ 0x001e0221U,
/*020d*/ 0x001e0222U,
/*020e*/ 0x001e0223U,
/*020f*/ 0x001e0224U,
/*0210*/ 0x001e0225U,
/*0211*/ 0x001e0226U,
/*0212*/ 0x001e0227U,
/*0213*/ 0x001e0228U,
/*0214*/ 0x001e0229U,
/*0215*/ 0x001e022aU,
/*0216*/ 0x001e022bU,
/*0217*/ 0x001e022cU,
/*0218*/ 0x001e022dU,
/*0219*/ 0x001e022eU,
/*021a*/ 0x001e022fU,
/*021b*/ 0x00010230U,
/*021c*/ 0x08010230U,
/*021d*/ 0x10010230U,
/*021e*/ 0x18040230U,
/*021f*/ 0x00080231U,
/*0220*/ 0x08080231U,
/*0221*/ 0x10080231U,
/*0222*/ 0x18040231U,
/*0223*/ 0x00070232U,
/*0224*/ 0x08060232U,
/*0225*/ 0x10070232U,
/*0226*/ 0x18070232U,
/*0227*/ 0x00060233U,
/*0228*/ 0x08070233U,
/*0229*/ 0x10070233U,
/*022a*/ 0x18060233U,
/*022b*/ 0x00070234U,
/*022c*/ 0x08020234U,
/*022d*/ 0x10010234U,
/*022e*/ 0x18010234U,
/*022f*/ 0x000a0235U,
/*0230*/ 0x00140236U,
/*0231*/ 0x000a0237U,
/*0232*/ 0x00140238U,
/*0233*/ 0x000a0239U,
/*0234*/ 0x0014023aU,
/*0235*/ 0xffffffffU,
/*0236*/ 0xffffffffU,
/*0237*/ 0x0005023bU,
/*0238*/ 0x0001023cU,
/*0239*/ 0x1001023cU,
/*023a*/ 0x1801023cU,
/*023b*/ 0x0001023dU,
/*023c*/ 0x0801023dU,
/*023d*/ 0x1001023dU,
/*023e*/ 0x1801023dU,
/*023f*/ 0x0002023eU,
/*0240*/ 0x0802023eU,
/*0241*/ 0x1002023eU,
/*0242*/ 0x1802023eU,
/*0243*/ 0x0002023fU,
/*0244*/ 0x0803023fU,
/*0245*/ 0x1001023fU,
/*0246*/ 0x1801023fU,
/*0247*/ 0x00010240U,
/*0248*/ 0x08010240U,
/*0249*/ 0x10010240U,
/*024a*/ 0x18020240U,
/*024b*/ 0x00010241U,
/*024c*/ 0x08010241U,
/*024d*/ 0x10010241U,
/*024e*/ 0x18020241U,
/*024f*/ 0x00010242U,
/*0250*/ 0x08010242U,
/*0251*/ 0x10010242U,
/*0252*/ 0x18020242U,
/*0253*/ 0x00010243U,
/*0254*/ 0x08010243U,
/*0255*/ 0x10010243U,
/*0256*/ 0x18020243U,
/*0257*/ 0xffffffffU,
/*0258*/ 0x00010244U,
/*0259*/ 0x08010244U,
/*025a*/ 0x10010244U,
/*025b*/ 0x18010244U,
/*025c*/ 0x00010245U,
/*025d*/ 0x08010245U,
/*025e*/ 0x10010245U,
/*025f*/ 0x18010245U,
/*0260*/ 0x00040246U,
/*0261*/ 0x08040246U,
/*0262*/ 0x10040246U,
/*0263*/ 0x18010246U,
/*0264*/ 0x00020247U,
/*0265*/ 0x08060247U,
/*0266*/ 0x10060247U,
/*0267*/ 0x18020247U,
/*0268*/ 0x00020248U,
/*0269*/ 0x08020248U,
/*026a*/ 0xffffffffU,
/*026b*/ 0x10100248U,
/*026c*/ 0x00010249U,
/*026d*/ 0x08010249U,
/*026e*/ 0x10010249U,
/*026f*/ 0x18040249U,
/*0270*/ 0x0001024aU,
/*0271*/ 0x0804024aU,
/*0272*/ 0x1003024aU,
/*0273*/ 0x1808024aU,
/*0274*/ 0x000a024bU,
/*0275*/ 0x100a024bU,
/*0276*/ 0x000a024cU,
/*0277*/ 0xffffffffU,
/*0278*/ 0x0020024dU,
/*0279*/ 0x0020024eU,
/*027a*/ 0x0005024fU,
/*027b*/ 0x1801023aU,
/*027c*/ 0x0805023cU,
/*027d*/ 0x0808024fU,
/*027e*/ 0x1001024fU,
/*027f*/ 0x1808024fU,
/*0280*/ 0x00010250U,
/*0281*/ 0x08080250U,
/*0282*/ 0x10010250U,
/*0283*/ 0x18040250U,
/*0284*/ 0x00040251U,
/*0285*/ 0x08040251U,
/*0286*/ 0x10040251U,
/*0287*/ 0x18040251U,
/*0288*/ 0x00040252U,
/*0289*/ 0x08040252U,
/*028a*/ 0x10040252U,
/*028b*/ 0x18040252U,
/*028c*/ 0x00040253U,
/*028d*/ 0x08010253U,
/*028e*/ 0x10040253U,
/*028f*/ 0x18040253U,
/*0290*/ 0x00040254U,
/*0291*/ 0x08040254U,
/*0292*/ 0x10040254U,
/*0293*/ 0x18040254U,
/*0294*/ 0x00060255U,
/*0295*/ 0x08060255U,
/*0296*/ 0x10060255U,
/*0297*/ 0x18060255U,
/*0298*/ 0x00060256U,
/*0299*/ 0x08060256U,
/*029a*/ 0x10040256U,
/*029b*/ 0x18010256U,
/*029c*/ 0x00010257U,
/*029d*/ 0x08020257U,
/*029e*/ 0x00200258U,
/*029f*/ 0x00200259U,
/*02a0*/ 0x0020025aU,
/*02a1*/ 0x0020025bU,
/*02a2*/ 0x0020025cU,
/*02a3*/ 0x0020025dU,
/*02a4*/ 0x0020025eU,
/*02a5*/ 0x0020025fU,
/*02a6*/ 0x00040260U,
/*02a7*/ 0x08040260U,
/*02a8*/ 0x10010260U,
/*02a9*/ 0x18010260U,
/*02aa*/ 0x00010261U,
/*02ab*/ 0x08010261U,
/*02ac*/ 0x10010261U,
/*02ad*/ 0x18010261U,
/*02ae*/ 0x00010262U,
/*02af*/ 0x08010262U,
/*02b0*/ 0x10010262U,
/*02b1*/ 0x18040262U,
/*02b2*/ 0x00040263U,
/*02b3*/ 0x080a0263U,
/*02b4*/ 0x00200264U,
/*02b5*/ 0x00040265U,
/*02b6*/ 0x08080265U,
/*02b7*/ 0x10020265U,
/*02b8*/ 0x18020265U,
/*02b9*/ 0x00020266U,
/*02ba*/ 0x08020266U,
/*02bb*/ 0x10020266U,
/*02bc*/ 0x18020266U,
/*02bd*/ 0xffffffffU,
/*02be*/ 0xffffffffU,
/*02bf*/ 0x00200267U,
/*02c0*/ 0x00030268U,
/*02c1*/ 0x08100268U,
/*02c2*/ 0x00100269U,
/*02c3*/ 0x10040269U,
/*02c4*/ 0x18040269U,
/*02c5*/ 0x0005026aU,
/*02c6*/ 0x0805026aU,
/*02c7*/ 0xffffffffU,
/*02c8*/ 0xffffffffU,
/*02c9*/ 0xffffffffU,
/*02ca*/ 0xffffffffU,
/*02cb*/ 0x1001026aU,
/*02cc*/ 0x1801026aU,
/*02cd*/ 0x0008026bU,
/*02ce*/ 0x0808026bU,
/*02cf*/ 0x1008026bU,
/*02d0*/ 0x1808026bU,
/*02d1*/ 0x0008026cU,
/*02d2*/ 0x0808026cU,
/*02d3*/ 0x1008026cU,
/*02d4*/ 0x1808026cU,
/*02d5*/ 0x0008026dU,
/*02d6*/ 0x0808026dU,
/*02d7*/ 0x1008026dU,
/*02d8*/ 0x1808026dU,
/*02d9*/ 0x0008026eU,
/*02da*/ 0x0808026eU,
/*02db*/ 0x1003026eU,
/*02dc*/ 0x1803026eU,
/*02dd*/ 0x0003026fU,
/*02de*/ 0xffffffffU,
/*02df*/ 0x0801026fU,
/*02e0*/ 0x1002026fU,
/*02e1*/ 0x1801026fU,
/*02e2*/ 0x00040270U,
/*02e3*/ 0x08020270U,
/*02e4*/ 0x10010270U,
/*02e5*/ 0x18010270U,
/*02e6*/ 0x00010271U,
/*02e7*/ 0x08010271U,
/*02e8*/ 0x10040271U,
/*02e9*/ 0x18080271U,
/*02ea*/ 0x000a0272U,
/*02eb*/ 0x100a0272U,
/*02ec*/ 0x000a0273U,
/*02ed*/ 0x100a0273U,
/*02ee*/ 0x000a0274U,
/*02ef*/ 0x100a0274U,
/*02f0*/ 0x00200275U,
/*02f1*/ 0x00200276U,
/*02f2*/ 0x00010277U,
/*02f3*/ 0x08020277U,
/*02f4*/ 0x10020277U,
/*02f5*/ 0x18020277U,
/*02f6*/ 0xffffffffU,
/*02f7*/ 0x00020278U,
/*02f8*/ 0x08100278U,
/*02f9*/ 0x18050278U,
/*02fa*/ 0x00060279U,
/*02fb*/ 0x08050279U,
/*02fc*/ 0x10050279U,
/*02fd*/ 0x000e027aU,
/*02fe*/ 0x1005027aU,
/*02ff*/ 0x000e027bU,
/*0300*/ 0x1005027bU,
/*0301*/ 0x000e027cU,
/*0302*/ 0x1005027cU,
/*0303*/ 0x1801027cU,
/*0304*/ 0x0005027dU,
/*0305*/ 0x0805027dU,
/*0306*/ 0x100a027dU,
/*0307*/ 0x000a027eU,
/*0308*/ 0x1005027eU,
/*0309*/ 0x1805027eU,
/*030a*/ 0x000a027fU,
/*030b*/ 0x100a027fU,
/*030c*/ 0x00050280U,
/*030d*/ 0x08050280U,
/*030e*/ 0x100a0280U,
/*030f*/ 0x000a0281U,
/*0310*/ 0x10070281U,
/*0311*/ 0x18070281U,
/*0312*/ 0x00070282U,
/*0313*/ 0x08070282U,
/*0314*/ 0x10070282U,
/*0315*/ 0x18070282U,
/*0316*/ 0xffffffffU,
/*0317*/ 0xffffffffU,
/*0318*/ 0x00040283U,
/*0319*/ 0x08040283U,
/*031a*/ 0x10040283U,
/*031b*/ 0x18040283U,
/*031c*/ 0x00040284U,
/*031d*/ 0xffffffffU,
/*031e*/ 0x08080284U,
/*031f*/ 0x10080284U,
/*0320*/ 0x18040284U,
/*0321*/ 0x00050285U,
/*0322*/ 0x08080285U,
/*0323*/ 0x10050285U,
/*0324*/ 0x18040285U,
/*0325*/ 0x00050286U,
/*0326*/ 0x08080286U,
/*0327*/ 0x10050286U,
/*0328*/ 0x18040286U,
/*0329*/ 0x00050287U,
/*032a*/ 0x08080287U,
/*032b*/ 0x10050287U,
/*032c*/ 0x18040287U,
/*032d*/ 0x00050288U,
/*032e*/ 0x08070288U,
/*032f*/ 0x10080288U,
/*0330*/ 0x00100289U,
/*0331*/ 0x10080289U,
/*0332*/ 0x0010028aU,
/*0333*/ 0x1008028aU,
/*0334*/ 0x0010028bU,
/*0335*/ 0x1008028bU,
/*0336*/ 0x1808028bU,
/*0337*/ 0x0001028cU,
/*0338*/ 0x0801028cU,
/*0339*/ 0x1006028cU,
/*033a*/ 0x1806028cU,
/*033b*/ 0x0006028dU,
/*033c*/ 0x0801028dU,
/*033d*/ 0x1001028dU,
/*033e*/ 0x1803028dU,
/*033f*/ 0x000a028eU,
/*0340*/ 0x100a028eU,
/*0341*/ 0x000a028fU,
/*0342*/ 0xffffffffU,
/*0343*/ 0x100a028fU,
/*0344*/ 0x00040290U,
/*0345*/ 0x08010290U,
/*0346*/ 0x10040290U,
/*0347*/ 0x18070290U,
/*0348*/ 0x00070291U,
/*0349*/ 0x08070291U,
/*034a*/ 0x10070291U,
/*034b*/ 0x18070291U,
/*034c*/ 0x00070292U,
/*034d*/ 0xffffffffU,
/*034e*/ 0xffffffffU,
/*034f*/ 0x08050292U,
/*0350*/ 0x10050292U,
/*0351*/ 0x18040292U,
/*0352*/ 0x00040293U,
/*0353*/ 0x08040293U,
/*0354*/ 0xffffffffU,
/*0355*/ 0x10010293U,
/*0356*/ 0x18010293U,
/*0357*/ 0x00020294U,
/*0358*/ 0x08080294U,
/*0359*/ 0x00200295U,
/*035a*/ 0x00200296U,
/*035b*/ 0x00100297U,
/*035c*/ 0x10020297U,
/*035d*/ 0x18020297U,
/*035e*/ 0x00020298U,
/*035f*/ 0xffffffffU,
/*0360*/ 0x08010298U,
/*0361*/ 0x10010298U,
/*0362*/ 0x18020298U,
/*0363*/ 0x00100299U,
/*0364*/ 0x10100299U,
/*0365*/ 0x0010029aU,
/*0366*/ 0x1008029aU,
/*0367*/ 0x1808029aU,
/*0368*/ 0x0008029bU,
/*0369*/ 0x0808029bU,
/*036a*/ 0x1010029bU,
/*036b*/ 0x0010029cU,
/*036c*/ 0x1010029cU,
/*036d*/ 0x0008029dU,
/*036e*/ 0x0808029dU,
/*036f*/ 0x1008029dU,
/*0370*/ 0x1808029dU,
/*0371*/ 0x0010029eU,
/*0372*/ 0x1010029eU,
/*0373*/ 0x0010029fU,
/*0374*/ 0x1008029fU,
/*0375*/ 0x1808029fU,
/*0376*/ 0x000802a0U,
/*0377*/ 0x080802a0U,
/*0378*/ 0x100802a0U,
/*0379*/ 0x001002a1U,
/*037a*/ 0x101002a1U,
/*037b*/ 0x001002a2U,
/*037c*/ 0x100802a2U,
/*037d*/ 0x180802a2U,
/*037e*/ 0x000802a3U,
/*037f*/ 0x080802a3U,
/*0380*/ 0x101002a3U,
/*0381*/ 0x001002a4U,
/*0382*/ 0x101002a4U,
/*0383*/ 0x000802a5U,
/*0384*/ 0x080802a5U,
/*0385*/ 0x100802a5U,
/*0386*/ 0x180802a5U,
/*0387*/ 0x001002a6U,
/*0388*/ 0x101002a6U,
/*0389*/ 0x001002a7U,
/*038a*/ 0x100802a7U,
/*038b*/ 0x180802a7U,
/*038c*/ 0x000802a8U,
/*038d*/ 0x080802a8U,
/*038e*/ 0x100802a8U,
/*038f*/ 0x001002a9U,
/*0390*/ 0x101002a9U,
/*0391*/ 0x001002aaU,
/*0392*/ 0x100802aaU,
/*0393*/ 0x180802aaU,
/*0394*/ 0x000802abU,
/*0395*/ 0x080802abU,
/*0396*/ 0x101002abU,
/*0397*/ 0x001002acU,
/*0398*/ 0x101002acU,
/*0399*/ 0x000802adU,
/*039a*/ 0x080802adU,
/*039b*/ 0x100802adU,
/*039c*/ 0x180802adU,
/*039d*/ 0x001002aeU,
/*039e*/ 0x101002aeU,
/*039f*/ 0x001002afU,
/*03a0*/ 0x100802afU,
/*03a1*/ 0x180802afU,
/*03a2*/ 0x000802b0U,
/*03a3*/ 0x080802b0U,
/*03a4*/ 0x100802b0U,
/*03a5*/ 0x001002b1U,
/*03a6*/ 0x101002b1U,
/*03a7*/ 0x001002b2U,
/*03a8*/ 0x100802b2U,
/*03a9*/ 0x180802b2U,
/*03aa*/ 0x000802b3U,
/*03ab*/ 0x080802b3U,
/*03ac*/ 0x101002b3U,
/*03ad*/ 0x001002b4U,
/*03ae*/ 0x101002b4U,
/*03af*/ 0x000802b5U,
/*03b0*/ 0x080802b5U,
/*03b1*/ 0x100802b5U,
/*03b2*/ 0x180802b5U,
/*03b3*/ 0x001002b6U,
/*03b4*/ 0x101002b6U,
/*03b5*/ 0x001002b7U,
/*03b6*/ 0x100802b7U,
/*03b7*/ 0x180802b7U,
/*03b8*/ 0x000802b8U,
/*03b9*/ 0x080802b8U,
/*03ba*/ 0x100802b8U,
/*03bb*/ 0x180202b8U,
/*03bc*/ 0x000302b9U,
/*03bd*/ 0x080a02b9U,
/*03be*/ 0x000a02baU,
/*03bf*/ 0x100a02baU,
/*03c0*/ 0x000502bbU,
/*03c1*/ 0x080802bbU,
/*03c2*/ 0x100802bbU,
/*03c3*/ 0x180802bbU,
/*03c4*/ 0x000602bcU,
/*03c5*/ 0x080602bcU,
/*03c6*/ 0x001102bdU,
/*03c7*/ 0x180802bdU,
/*03c8*/ 0x000402beU,
/*03c9*/ 0x080602beU,
/*03ca*/ 0x100802beU,
/*03cb*/ 0x180802beU,
/*03cc*/ 0x000802bfU,
/*03cd*/ 0x080802bfU,
/*03ce*/ 0x100802bfU,
/*03cf*/ 0x180802bfU,
/*03d0*/ 0x000802c0U,
/*03d1*/ 0x080602c0U,
/*03d2*/ 0x100602c0U,
/*03d3*/ 0x001102c1U,
/*03d4*/ 0x180802c1U,
/*03d5*/ 0x000402c2U,
/*03d6*/ 0x080602c2U,
/*03d7*/ 0x100802c2U,
/*03d8*/ 0x180802c2U,
/*03d9*/ 0x000802c3U,
/*03da*/ 0x080802c3U,
/*03db*/ 0x100802c3U,
/*03dc*/ 0x180802c3U,
/*03dd*/ 0x000802c4U,
/*03de*/ 0x080602c4U,
/*03df*/ 0x100602c4U,
/*03e0*/ 0x001102c5U,
/*03e1*/ 0x180802c5U,
/*03e2*/ 0x000402c6U,
/*03e3*/ 0x080602c6U,
/*03e4*/ 0x100802c6U,
/*03e5*/ 0x180802c6U,
/*03e6*/ 0x000802c7U,
/*03e7*/ 0x080802c7U,
/*03e8*/ 0x100402c7U,
/*03e9*/ 0x180402c7U,
/*03ea*/ 0x000402c8U,
/*03eb*/ 0x080402c8U,
/*03ec*/ 0x100402c8U,
/*03ed*/ 0x180402c8U,
/*03ee*/ 0x000402c9U,
/*03ef*/ 0x080402c9U,
/*03f0*/ 0x100402c9U,
/*03f1*/ 0x180402c9U,
/*03f2*/ 0x000402caU,
/*03f3*/ 0x080402caU,
/*03f4*/ 0x100402caU,
/*03f5*/ 0x180402caU,
/*03f6*/ 0x000402cbU,
/*03f7*/ 0x080402cbU,
/*03f8*/ 0x100402cbU,
/*03f9*/ 0x180402cbU,
/*03fa*/ 0x000402ccU,
/*03fb*/ 0x080402ccU,
/*03fc*/ 0x001702cdU,
/*03fd*/ 0x001602ceU,
/*03fe*/ 0x001702cfU,
/*03ff*/ 0x002002d0U,
/*0400*/ 0x002002d1U,
/*0401*/ 0x002002d2U,
/*0402*/ 0x002002d3U,
/*0403*/ 0x002002d4U,
/*0404*/ 0x002002d5U,
/*0405*/ 0x002002d6U,
/*0406*/ 0x002002d7U,
/*0407*/ 0x002002d8U,
/*0408*/ 0x000202d9U,
/*0409*/ 0x080502d9U,
/*040a*/ 0x100502d9U,
/*040b*/ 0x180102d9U,
/*040c*/ 0x000502daU,
/*040d*/ 0x080502daU,
/*040e*/ 0x100502daU,
/*040f*/ 0x180502daU,
/*0410*/ 0x000502dbU,
/*0411*/ 0x080502dbU,
/*0412*/ 0x100502dbU,
/*0413*/ 0x180502dbU,
/*0414*/ 0x000502dcU,
/*0415*/ 0x080502dcU,
/*0416*/ 0x100502dcU,
/*0417*/ 0x180502dcU,
/*0418*/ 0x000502ddU,
/*0419*/ 0x080502ddU,
/*041a*/ 0x100502ddU,
/*041b*/ 0x180502ddU,
/*041c*/ 0x000502deU,
/*041d*/ 0x080502deU,
/*041e*/ 0x100502deU,
/*041f*/ 0x180502deU,
/*0420*/ 0x000502dfU,
/*0421*/ 0x080502dfU,
/*0422*/ 0x100102dfU,
/*0423*/ 0x180202dfU,
/*0424*/ 0x000202e0U,
/*0425*/ 0x080202e0U,
/*0426*/ 0x100202e0U,
/*0427*/ 0x180102e0U,
/*0428*/ 0x000802e1U,
/*0429*/ 0x081502e1U,
/*042a*/ 0x002002e2U,
/*042b*/ 0x001502e3U,
/*042c*/ 0x002002e4U,
/*042d*/ 0x001502e5U,
/*042e*/ 0x002002e6U,
/*042f*/ 0x000702e7U,
/*0430*/ 0x080102e7U,
/*0431*/ 0x100202e7U,
/*0432*/ 0x180602e7U,
/*0433*/ 0x000102e8U,
/*0434*/ 0x080102e8U,
/*0435*/ 0x002002e9U,
/*0436*/ 0x000202eaU,
/*0437*/ 0x002002ebU,
/*0438*/ 0x002002ecU,
/*0439*/ 0x000c02edU,
/*043a*/ 0x100c02edU,
/*043b*/ 0x002002eeU,
/*043c*/ 0x000302efU,
/*043d*/ 0x002002f0U,
/*043e*/ 0x000302f1U,
/*043f*/ 0x002002f2U,
/*0440*/ 0x000302f3U,
/*0441*/ 0x002002f4U,
/*0442*/ 0x000302f5U,
/*0443*/ 0x002002f6U,
/*0444*/ 0x000302f7U,
/*0445*/ 0x002002f8U,
/*0446*/ 0x000302f9U,
/*0447*/ 0x002002faU,
/*0448*/ 0x000302fbU,
/*0449*/ 0x002002fcU,
/*044a*/ 0x000302fdU,
/*044b*/ 0x002002feU,
/*044c*/ 0x000302ffU,
/*044d*/ 0x00200300U,
/*044e*/ 0x00030301U,
/*044f*/ 0x08030301U,
/*0450*/ 0x10020301U,
/*0451*/ 0x18020301U,
/*0452*/ 0x00200302U,
/*0453*/ 0x00200303U,
/*0454*/ 0x00200304U,
/*0455*/ 0x00200305U,
/*0456*/ 0x00040306U,
/*0457*/ 0x001e0307U,
/*0458*/ 0x001e0308U,
/*0459*/ 0x001e0309U,
/*045a*/ 0x001e030aU,
/*045b*/ 0x001e030bU,
/*045c*/ 0x001e030cU,
/*045d*/ 0x001e030dU,
/*045e*/ 0x001e030eU,
/*045f*/ 0x0004030fU,
/*0460*/ 0x0801030fU,
/*0461*/ 0x1010030fU,
/*0462*/ 0x00100310U,
/*0463*/ 0x10100310U,
/*0464*/ 0x00040311U,
/*0465*/ 0x08010311U,
/*0466*/ 0x10080311U,
/*0467*/ 0x18040311U,
/*0468*/ 0x00010312U,
/*0469*/ 0x08080312U,
/*046a*/ 0x10040312U,
/*046b*/ 0x18010312U,
/*046c*/ 0x00080313U,
/*046d*/ 0x08040313U,
/*046e*/ 0x10010313U,
/*046f*/ 0x18080313U,
/*0470*/ 0x00040314U,
/*0471*/ 0x08010314U,
/*0472*/ 0x10080314U,
/*0473*/ 0x18040314U,
/*0474*/ 0x00010315U,
/*0475*/ 0x08080315U,
/*0476*/ 0x10040315U,
/*0477*/ 0x18010315U,
/*0478*/ 0x00080316U,
/*0479*/ 0x08040316U,
/*047a*/ 0x10010316U,
/*047b*/ 0x18080316U,
/*047c*/ 0x00080317U,
/*047d*/ 0x00010318U,
/*047e*/ 0x08050318U,
/*047f*/ 0x10010318U,
/*0480*/ 0x18020318U,
/*0481*/ 0x00010319U,
/*0482*/ 0x08010319U,
/*0483*/ 0x10010319U,
/*0484*/ 0x18010319U,
/*0485*/ 0x0001031aU,
/*0486*/ 0x0801031aU,
/*0487*/ 0x1001031aU,
/*0488*/ 0x1801031aU,
/*0489*/ 0x0001031bU,
/*048a*/ 0x0801031bU,
/*048b*/ 0x1001031bU,
/*048c*/ 0x1801031bU,
/*048d*/ 0x0001031cU,
/*048e*/ 0x0801031cU,
/*048f*/ 0x1001031cU,
/*0490*/ 0x1801031cU,
/*0491*/ 0x0008031dU,
/*0492*/ 0x0808031dU,
/*0493*/ 0x1008031dU,
/*0494*/ 0x1808031dU,
	 }
};
