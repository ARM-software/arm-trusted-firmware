/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __MT_SPM_VCOREFS__H__
#define __MT_SPM_VCOREFS__H__

int spm_vcorefs_get_vcore(unsigned int gear);
uint64_t spm_vcorefs_v2_args(u_register_t x1, u_register_t x2, u_register_t x3,
			     u_register_t *x4);

enum vcorefs_smc_cmd {
	VCOREFS_SMC_CMD_0 = 0,
	VCOREFS_SMC_CMD_1,
	VCOREFS_SMC_CMD_2,
	VCOREFS_SMC_CMD_3,
	VCOREFS_SMC_CMD_4,
	/* check spmfw status */
	VCOREFS_SMC_CMD_5,

	/* get spmfw type */
	VCOREFS_SMC_CMD_6,

	/* get spm reg status */
	VCOREFS_SMC_CMD_7,

	NUM_VCOREFS_SMC_CMD,
};

enum vcorefs_smc_cmd_new {
	VCOREFS_SMC_CMD_INIT = 0,
	VCOREFS_SMC_CMD_KICK = 1,
	VCOREFS_SMC_CMD_OPP_TYPE = 2,
	VCOREFS_SMC_CMD_FW_TYPE = 3,
	VCOREFS_SMC_CMD_GET_UV = 4,
	VCOREFS_SMC_CMD_GET_FREQ = 5,
	VCOREFS_SMC_CMD_GET_NUM_V = 6,
	VCOREFS_SMC_CMD_GET_NUM_F = 7,
	VCOREFS_SMC_CMD_FB_ACTION = 8,
	/*chip specific setting */
	VCOREFS_SMC_CMD_SET_FREQ = 16,
	VCOREFS_SMC_CMD_SET_EFUSE = 17,
	VCOREFS_SMC_CMD_GET_EFUSE = 18,
	VCOREFS_SMC_CMD_DVFS_HOPPING = 19,
	VCOREFS_SMC_CMD_DVFS_HOPPING_STATE = 20,
};

enum dvfsrc_channel {
	DVFSRC_CHANNEL_1 = 1,
	DVFSRC_CHANNEL_2,
	DVFSRC_CHANNEL_3,
	DVFSRC_CHANNEL_4,
	NUM_DVFSRC_CHANNEL,
};

#define _VCORE_BASE_UV		400000
#define _VCORE_STEP_UV		6250

/* PMIC */
#define __vcore_pmic_to_uv(pmic)	\
	(((pmic) * _VCORE_STEP_UV) + _VCORE_BASE_UV)

#define __vcore_uv_to_pmic(uv)	/* pmic >= uv */	\
	((((uv) - _VCORE_BASE_UV) + (_VCORE_STEP_UV - 1)) / _VCORE_STEP_UV)

struct reg_config {
	uint32_t offset;
	uint32_t val;
};

#define DVFSRC_BASIC_CONTROL             (DVFSRC_BASE + 0x0)
#define DVFSRC_SW_REQ1                   (DVFSRC_BASE + 0x4)
#define DVFSRC_SW_REQ2                   (DVFSRC_BASE + 0x8)
#define DVFSRC_SW_REQ3                   (DVFSRC_BASE + 0xC)
#define DVFSRC_SW_REQ4                   (DVFSRC_BASE + 0x10)
#define DVFSRC_SW_REQ5                   (DVFSRC_BASE + 0x14)
#define DVFSRC_SW_REQ6                   (DVFSRC_BASE + 0x18)
#define DVFSRC_SW_REQ7                   (DVFSRC_BASE + 0x1C)
#define DVFSRC_SW_REQ8                   (DVFSRC_BASE + 0x20)
#define DVFSRC_EMI_REQUEST               (DVFSRC_BASE + 0x24)
#define DVFSRC_EMI_REQUEST2              (DVFSRC_BASE + 0x28)
#define DVFSRC_EMI_REQUEST3              (DVFSRC_BASE + 0x2C)
#define DVFSRC_EMI_REQUEST4              (DVFSRC_BASE + 0x30)
#define DVFSRC_EMI_REQUEST5              (DVFSRC_BASE + 0x34)
#define DVFSRC_EMI_REQUEST6              (DVFSRC_BASE + 0x38)
#define DVFSRC_EMI_HRT                   (DVFSRC_BASE + 0x3C)
#define DVFSRC_EMI_HRT2                  (DVFSRC_BASE + 0x40)
#define DVFSRC_EMI_HRT3                  (DVFSRC_BASE + 0x44)
#define DVFSRC_EMI_QOS0                  (DVFSRC_BASE + 0x48)
#define DVFSRC_EMI_QOS1                  (DVFSRC_BASE + 0x4C)
#define DVFSRC_EMI_QOS2                  (DVFSRC_BASE + 0x50)
#define DVFSRC_EMI_MD2SPM0               (DVFSRC_BASE + 0x54)
#define DVFSRC_EMI_MD2SPM1               (DVFSRC_BASE + 0x58)
#define DVFSRC_EMI_MD2SPM2               (DVFSRC_BASE + 0x5C)
#define DVFSRC_EMI_MD2SPM0_T             (DVFSRC_BASE + 0x60)
#define DVFSRC_EMI_MD2SPM1_T             (DVFSRC_BASE + 0x64)
#define DVFSRC_EMI_MD2SPM2_T             (DVFSRC_BASE + 0x68)
#define DVFSRC_VCORE_REQUEST             (DVFSRC_BASE + 0x6C)
#define DVFSRC_VCORE_REQUEST2            (DVFSRC_BASE + 0x70)
#define DVFSRC_VCORE_REQUEST3            (DVFSRC_BASE + 0x74)
#define DVFSRC_VCORE_REQUEST4            (DVFSRC_BASE + 0x78)
#define DVFSRC_VCORE_HRT                 (DVFSRC_BASE + 0x7C)
#define DVFSRC_VCORE_HRT2                (DVFSRC_BASE + 0x80)
#define DVFSRC_VCORE_HRT3                (DVFSRC_BASE + 0x84)
#define DVFSRC_VCORE_QOS0                (DVFSRC_BASE + 0x88)
#define DVFSRC_VCORE_QOS1                (DVFSRC_BASE + 0x8C)
#define DVFSRC_VCORE_QOS2                (DVFSRC_BASE + 0x90)
#define DVFSRC_VCORE_MD2SPM0             (DVFSRC_BASE + 0x94)
#define DVFSRC_VCORE_MD2SPM1             (DVFSRC_BASE + 0x98)
#define DVFSRC_VCORE_MD2SPM2             (DVFSRC_BASE + 0x9C)
#define DVFSRC_VCORE_MD2SPM0_T           (DVFSRC_BASE + 0xA0)
#define DVFSRC_VCORE_MD2SPM1_T           (DVFSRC_BASE + 0xA4)
#define DVFSRC_VCORE_MD2SPM2_T           (DVFSRC_BASE + 0xA8)
#define DVFSRC_MD_VSRAM_REMAP            (DVFSRC_BASE + 0xBC)
#define DVFSRC_HALT_SW_CONTROL           (DVFSRC_BASE + 0xC0)
#define DVFSRC_INT                       (DVFSRC_BASE + 0xC4)
#define DVFSRC_INT_EN                    (DVFSRC_BASE + 0xC8)
#define DVFSRC_INT_CLR                   (DVFSRC_BASE + 0xCC)
#define DVFSRC_BW_MON_WINDOW             (DVFSRC_BASE + 0xD0)
#define DVFSRC_BW_MON_THRES_1            (DVFSRC_BASE + 0xD4)
#define DVFSRC_BW_MON_THRES_2            (DVFSRC_BASE + 0xD8)
#define DVFSRC_MD_TURBO                  (DVFSRC_BASE + 0xDC)
#define DVFSRC_PCIE_VCORE_REQ            (DVFSRC_BASE + 0xE0)
#define DVFSRC_VCORE_USER_REQ            (DVFSRC_BASE + 0xE4)
#define DVFSRC_DEBOUNCE_FOUR             (DVFSRC_BASE + 0xF0)
#define DVFSRC_DEBOUNCE_RISE_FALL        (DVFSRC_BASE + 0xF4)
#define DVFSRC_TIMEOUT_NEXTREQ           (DVFSRC_BASE + 0xF8)
#define DVFSRC_LEVEL_LABEL_0_1           (DVFSRC_BASE + 0x100)
#define DVFSRC_LEVEL_LABEL_2_3           (DVFSRC_BASE + 0x104)
#define DVFSRC_LEVEL_LABEL_4_5           (DVFSRC_BASE + 0x108)
#define DVFSRC_LEVEL_LABEL_6_7           (DVFSRC_BASE + 0x10C)
#define DVFSRC_LEVEL_LABEL_8_9           (DVFSRC_BASE + 0x110)
#define DVFSRC_LEVEL_LABEL_10_11         (DVFSRC_BASE + 0x114)
#define DVFSRC_LEVEL_LABEL_12_13         (DVFSRC_BASE + 0x118)
#define DVFSRC_LEVEL_LABEL_14_15         (DVFSRC_BASE + 0x11C)
#define DVFSRC_MM_BW_0                   (DVFSRC_BASE + 0x200)
#define DVFSRC_MM_BW_1                   (DVFSRC_BASE + 0x204)
#define DVFSRC_MM_BW_2                   (DVFSRC_BASE + 0x208)
#define DVFSRC_MM_BW_3                   (DVFSRC_BASE + 0x20C)
#define DVFSRC_MM_BW_4                   (DVFSRC_BASE + 0x210)
#define DVFSRC_MM_BW_5                   (DVFSRC_BASE + 0x214)
#define DVFSRC_MM_BW_6                   (DVFSRC_BASE + 0x218)
#define DVFSRC_MM_BW_7                   (DVFSRC_BASE + 0x21C)
#define DVFSRC_MM_BW_8                   (DVFSRC_BASE + 0x220)
#define DVFSRC_MM_BW_9                   (DVFSRC_BASE + 0x224)
#define DVFSRC_MM_BW_10                  (DVFSRC_BASE + 0x228)
#define DVFSRC_MM_BW_11                  (DVFSRC_BASE + 0x22C)
#define DVFSRC_MM_BW_12                  (DVFSRC_BASE + 0x230)
#define DVFSRC_MM_BW_13                  (DVFSRC_BASE + 0x234)
#define DVFSRC_MM_BW_14                  (DVFSRC_BASE + 0x238)
#define DVFSRC_MM_BW_15                  (DVFSRC_BASE + 0x23C)
#define DVFSRC_MD_BW_0                   (DVFSRC_BASE + 0x240)
#define DVFSRC_MD_BW_1                   (DVFSRC_BASE + 0x244)
#define DVFSRC_MD_BW_2                   (DVFSRC_BASE + 0x248)
#define DVFSRC_MD_BW_3                   (DVFSRC_BASE + 0x24C)
#define DVFSRC_MD_BW_4                   (DVFSRC_BASE + 0x250)
#define DVFSRC_MD_BW_5                   (DVFSRC_BASE + 0x254)
#define DVFSRC_MD_BW_6                   (DVFSRC_BASE + 0x258)
#define DVFSRC_MD_BW_7                   (DVFSRC_BASE + 0x25C)
#define DVFSRC_SW_BW_0                   (DVFSRC_BASE + 0x260)
#define DVFSRC_SW_BW_1                   (DVFSRC_BASE + 0x264)
#define DVFSRC_SW_BW_2                   (DVFSRC_BASE + 0x268)
#define DVFSRC_SW_BW_3                   (DVFSRC_BASE + 0x26C)
#define DVFSRC_SW_BW_4                   (DVFSRC_BASE + 0x270)
#define DVFSRC_SW_BW_5                   (DVFSRC_BASE + 0x274)
#define DVFSRC_SW_BW_6                   (DVFSRC_BASE + 0x278)
#define DVFSRC_QOS_EN                    (DVFSRC_BASE + 0x280)
#define DVFSRC_MD_BW_URG                 (DVFSRC_BASE + 0x284)
#define DVFSRC_ISP_HRT                   (DVFSRC_BASE + 0x290)
#define DVFSRC_HRT_BW_BASE               (DVFSRC_BASE + 0x294)
#define DVFSRC_SEC_SW_REQ                (DVFSRC_BASE + 0x304)
#define DVFSRC_EMI_MON_DEBOUNCE_TIME     (DVFSRC_BASE + 0x308)
#define DVFSRC_MD_LATENCY_IMPROVE        (DVFSRC_BASE + 0x30C)
#define DVFSRC_BASIC_CONTROL_3           (DVFSRC_BASE + 0x310)
#define DVFSRC_DEBOUNCE_TIME             (DVFSRC_BASE + 0x314)
#define DVFSRC_LEVEL_MASK                (DVFSRC_BASE + 0x318)
#define DVFSRC_DEFAULT_OPP               (DVFSRC_BASE + 0x31C)
#define DVFSRC_95MD_SCEN_EMI0            (DVFSRC_BASE + 0x500)
#define DVFSRC_95MD_SCEN_EMI1            (DVFSRC_BASE + 0x504)
#define DVFSRC_95MD_SCEN_EMI2            (DVFSRC_BASE + 0x508)
#define DVFSRC_95MD_SCEN_EMI3            (DVFSRC_BASE + 0x50C)
#define DVFSRC_95MD_SCEN_EMI0_T          (DVFSRC_BASE + 0x510)
#define DVFSRC_95MD_SCEN_EMI1_T          (DVFSRC_BASE + 0x514)
#define DVFSRC_95MD_SCEN_EMI2_T          (DVFSRC_BASE + 0x518)
#define DVFSRC_95MD_SCEN_EMI3_T          (DVFSRC_BASE + 0x51C)
#define DVFSRC_95MD_SCEN_EMI4            (DVFSRC_BASE + 0x520)
#define DVFSRC_95MD_SCEN_BW0             (DVFSRC_BASE + 0x524)
#define DVFSRC_95MD_SCEN_BW1             (DVFSRC_BASE + 0x528)
#define DVFSRC_95MD_SCEN_BW2             (DVFSRC_BASE + 0x52C)
#define DVFSRC_95MD_SCEN_BW3             (DVFSRC_BASE + 0x530)
#define DVFSRC_95MD_SCEN_BW0_T           (DVFSRC_BASE + 0x534)
#define DVFSRC_95MD_SCEN_BW1_T           (DVFSRC_BASE + 0x538)
#define DVFSRC_95MD_SCEN_BW2_T           (DVFSRC_BASE + 0x53C)
#define DVFSRC_95MD_SCEN_BW3_T           (DVFSRC_BASE + 0x540)
#define DVFSRC_95MD_SCEN_BW4             (DVFSRC_BASE + 0x544)
#define DVFSRC_MD_LEVEL_SW_REG           (DVFSRC_BASE + 0x548)
#define DVFSRC_RSRV_0                    (DVFSRC_BASE + 0x600)
#define DVFSRC_RSRV_1                    (DVFSRC_BASE + 0x604)
#define DVFSRC_RSRV_2                    (DVFSRC_BASE + 0x608)
#define DVFSRC_RSRV_3                    (DVFSRC_BASE + 0x60C)
#define DVFSRC_RSRV_4                    (DVFSRC_BASE + 0x610)
#define DVFSRC_RSRV_5                    (DVFSRC_BASE + 0x614)
#define DVFSRC_SPM_RESEND                (DVFSRC_BASE + 0x630)
#define DVFSRC_DEBUG_STA_0               (DVFSRC_BASE + 0x700)
#define DVFSRC_DEBUG_STA_1               (DVFSRC_BASE + 0x704)
#define DVFSRC_DEBUG_STA_2               (DVFSRC_BASE + 0x708)
#define DVFSRC_DEBUG_STA_3               (DVFSRC_BASE + 0x70C)
#define DVFSRC_DEBUG_STA_4               (DVFSRC_BASE + 0x710)
#define DVFSRC_DEBUG_STA_5               (DVFSRC_BASE + 0x714)
#define DVFSRC_EMI_REQUEST7              (DVFSRC_BASE + 0x800)
#define DVFSRC_EMI_HRT_1                 (DVFSRC_BASE + 0x804)
#define DVFSRC_EMI_HRT2_1                (DVFSRC_BASE + 0x808)
#define DVFSRC_EMI_HRT3_1                (DVFSRC_BASE + 0x80C)
#define DVFSRC_EMI_QOS3                  (DVFSRC_BASE + 0x810)
#define DVFSRC_EMI_QOS4                  (DVFSRC_BASE + 0x814)
#define DVFSRC_DDR_REQUEST               (DVFSRC_BASE + 0xA00)
#define DVFSRC_DDR_REQUEST2              (DVFSRC_BASE + 0xA04)
#define DVFSRC_DDR_REQUEST3              (DVFSRC_BASE + 0xA08)
#define DVFSRC_DDR_REQUEST4              (DVFSRC_BASE + 0xA0C)
#define DVFSRC_DDR_REQUEST5              (DVFSRC_BASE + 0xA10)
#define DVFSRC_DDR_REQUEST6              (DVFSRC_BASE + 0xA14)
#define DVFSRC_DDR_REQUEST7              (DVFSRC_BASE + 0xA18)
#define DVFSRC_DDR_HRT                   (DVFSRC_BASE + 0xA1C)
#define DVFSRC_DDR_HRT2                  (DVFSRC_BASE + 0xA20)
#define DVFSRC_DDR_HRT3                  (DVFSRC_BASE + 0xA24)
#define DVFSRC_DDR_HRT_1                 (DVFSRC_BASE + 0xA28)
#define DVFSRC_DDR_HRT2_1                (DVFSRC_BASE + 0xA2C)
#define DVFSRC_DDR_HRT3_1                (DVFSRC_BASE + 0xA30)
#define DVFSRC_DDR_QOS0                  (DVFSRC_BASE + 0xA34)
#define DVFSRC_DDR_QOS1                  (DVFSRC_BASE + 0xA38)
#define DVFSRC_DDR_QOS2                  (DVFSRC_BASE + 0xA3C)
#define DVFSRC_DDR_QOS3                  (DVFSRC_BASE + 0xA40)
#define DVFSRC_DDR_QOS4                  (DVFSRC_BASE + 0xA44)
#define DVFSRC_DDR_MD2SPM0               (DVFSRC_BASE + 0xA48)
#define DVFSRC_DDR_MD2SPM1               (DVFSRC_BASE + 0xA4C)
#define DVFSRC_DDR_MD2SPM2               (DVFSRC_BASE + 0xA50)
#define DVFSRC_DDR_MD2SPM0_T             (DVFSRC_BASE + 0xA54)
#define DVFSRC_DDR_MD2SPM1_T             (DVFSRC_BASE + 0xA58)
#define DVFSRC_DDR_MD2SPM2_T             (DVFSRC_BASE + 0xA5C)
#define DVFSRC_HRT_REQ_UNIT              (DVFSRC_BASE + 0xA60)
#define DVSFRC_HRT_REQ_MD_URG            (DVFSRC_BASE + 0xA64)
#define DVFSRC_HRT_REQ_MD_BW_0           (DVFSRC_BASE + 0xA68)
#define DVFSRC_HRT_REQ_MD_BW_1           (DVFSRC_BASE + 0xA6C)
#define DVFSRC_HRT_REQ_MD_BW_2           (DVFSRC_BASE + 0xA70)
#define DVFSRC_HRT_REQ_MD_BW_3           (DVFSRC_BASE + 0xA74)
#define DVFSRC_HRT_REQ_MD_BW_4           (DVFSRC_BASE + 0xA78)
#define DVFSRC_HRT_REQ_MD_BW_5           (DVFSRC_BASE + 0xA7C)
#define DVFSRC_HRT_REQ_MD_BW_6           (DVFSRC_BASE + 0xA80)
#define DVFSRC_HRT_REQ_MD_BW_7           (DVFSRC_BASE + 0xA84)
#define DVFSRC_HRT1_REQ_MD_BW_0          (DVFSRC_BASE + 0xA88)
#define DVFSRC_HRT1_REQ_MD_BW_1          (DVFSRC_BASE + 0xA8C)
#define DVFSRC_HRT1_REQ_MD_BW_2          (DVFSRC_BASE + 0xA90)
#define DVFSRC_HRT1_REQ_MD_BW_3          (DVFSRC_BASE + 0xA94)
#define DVFSRC_HRT1_REQ_MD_BW_4          (DVFSRC_BASE + 0xA98)
#define DVFSRC_HRT1_REQ_MD_BW_5          (DVFSRC_BASE + 0xA9C)
#define DVFSRC_HRT1_REQ_MD_BW_6          (DVFSRC_BASE + 0xAA0)
#define DVFSRC_HRT1_REQ_MD_BW_7          (DVFSRC_BASE + 0xAA4)
#define DVFSRC_HRT_REQ_MD_BW_8           (DVFSRC_BASE + 0xAA8)
#define DVFSRC_HRT_REQ_MD_BW_9           (DVFSRC_BASE + 0xAAC)
#define DVFSRC_HRT_REQ_MD_BW_10          (DVFSRC_BASE + 0xAB0)
#define DVFSRC_HRT1_REQ_MD_BW_8          (DVFSRC_BASE + 0xAB4)
#define DVFSRC_HRT1_REQ_MD_BW_9          (DVFSRC_BASE + 0xAB8)
#define DVFSRC_HRT1_REQ_MD_BW_10         (DVFSRC_BASE + 0xABC)
#define DVFSRC_HRT_REQ_BW_SW_REG         (DVFSRC_BASE + 0xAC0)
#define DVFSRC_HRT_REQUEST               (DVFSRC_BASE + 0xAC4)
#define DVFSRC_HRT_HIGH_2                (DVFSRC_BASE + 0xAC8)
#define DVFSRC_HRT_HIGH_1                (DVFSRC_BASE + 0xACC)
#define DVFSRC_HRT_HIGH                  (DVFSRC_BASE + 0xAD0)
#define DVFSRC_HRT_LOW_2                 (DVFSRC_BASE + 0xAD4)
#define DVFSRC_HRT_LOW_1                 (DVFSRC_BASE + 0xAD8)
#define DVFSRC_HRT_LOW                   (DVFSRC_BASE + 0xADC)
#define DVFSRC_DDR_ADD_REQUEST           (DVFSRC_BASE + 0xAE0)
#define DVFSRC_LAST                      (DVFSRC_BASE + 0xAE4)
#define DVFSRC_LAST_L                    (DVFSRC_BASE + 0xAE8)
#define DVFSRC_MD_SCENARIO               (DVFSRC_BASE + 0xAEC)
#define DVFSRC_RECORD_0_0                (DVFSRC_BASE + 0xAF0)
#define DVFSRC_RECORD_0_1                (DVFSRC_BASE + 0xAF4)
#define DVFSRC_RECORD_0_2                (DVFSRC_BASE + 0xAF8)
#define DVFSRC_RECORD_0_3                (DVFSRC_BASE + 0xAFC)
#define DVFSRC_RECORD_0_4                (DVFSRC_BASE + 0xB00)
#define DVFSRC_RECORD_0_5                (DVFSRC_BASE + 0xB04)
#define DVFSRC_RECORD_0_6                (DVFSRC_BASE + 0xB08)
#define DVFSRC_RECORD_0_7                (DVFSRC_BASE + 0xB0C)
#define DVFSRC_RECORD_0_L_0              (DVFSRC_BASE + 0xBF0)
#define DVFSRC_RECORD_0_L_1              (DVFSRC_BASE + 0xBF4)
#define DVFSRC_RECORD_0_L_2              (DVFSRC_BASE + 0xBF8)
#define DVFSRC_RECORD_0_L_3              (DVFSRC_BASE + 0xBFC)
#define DVFSRC_RECORD_0_L_4              (DVFSRC_BASE + 0xC00)
#define DVFSRC_RECORD_0_L_5              (DVFSRC_BASE + 0xC04)
#define DVFSRC_RECORD_0_L_6              (DVFSRC_BASE + 0xC08)
#define DVFSRC_RECORD_0_L_7              (DVFSRC_BASE + 0xC0C)
#define DVFSRC_EMI_REQUEST8              (DVFSRC_BASE + 0xCF0)
#define DVFSRC_DDR_REQUEST8              (DVFSRC_BASE + 0xCF4)
#define DVFSRC_EMI_HRT_2                 (DVFSRC_BASE + 0xCF8)
#define DVFSRC_EMI_HRT2_2                (DVFSRC_BASE + 0xCFC)
#define DVFSRC_EMI_HRT3_2                (DVFSRC_BASE + 0xD00)
#define DVFSRC_EMI_QOS5                  (DVFSRC_BASE + 0xD04)
#define DVFSRC_EMI_QOS6                  (DVFSRC_BASE + 0xD08)
#define DVFSRC_DDR_HRT_2                 (DVFSRC_BASE + 0xD0C)
#define DVFSRC_DDR_HRT2_2                (DVFSRC_BASE + 0xD10)
#define DVFSRC_DDR_HRT3_2                (DVFSRC_BASE + 0xD14)
#define DVFSRC_DDR_QOS5                  (DVFSRC_BASE + 0xD18)
#define DVFSRC_DDR_QOS6                  (DVFSRC_BASE + 0xD1C)
#define DVFSRC_VCORE_REQUEST5            (DVFSRC_BASE + 0xD20)
#define DVFSRC_VCORE_HRT_1               (DVFSRC_BASE + 0xD24)
#define DVFSRC_VCORE_HRT2_1              (DVFSRC_BASE + 0xD28)
#define DVFSRC_VCORE_HRT3_1              (DVFSRC_BASE + 0xD2C)
#define DVFSRC_VCORE_QOS3                (DVFSRC_BASE + 0xD30)
#define DVFSRC_VCORE_QOS4                (DVFSRC_BASE + 0xD34)
#define DVFSRC_HRT_HIGH_3                (DVFSRC_BASE + 0xD38)
#define DVFSRC_HRT_LOW_3                 (DVFSRC_BASE + 0xD3C)
#define DVFSRC_BASIC_CONTROL_2           (DVFSRC_BASE + 0xD40)
#define DVFSRC_CURRENT_LEVEL             (DVFSRC_BASE + 0xD44)
#define DVFSRC_TARGET_LEVEL              (DVFSRC_BASE + 0xD48)
#define DVFSRC_LEVEL_LABEL_16_17         (DVFSRC_BASE + 0xD4C)
#define DVFSRC_LEVEL_LABEL_18_19         (DVFSRC_BASE + 0xD50)
#define DVFSRC_LEVEL_LABEL_20_21         (DVFSRC_BASE + 0xD54)
#define DVFSRC_LEVEL_LABEL_22_23         (DVFSRC_BASE + 0xD58)
#define DVFSRC_LEVEL_LABEL_24_25         (DVFSRC_BASE + 0xD5C)
#define DVFSRC_LEVEL_LABEL_26_27         (DVFSRC_BASE + 0xD60)
#define DVFSRC_LEVEL_LABEL_28_29         (DVFSRC_BASE + 0xD64)
#define DVFSRC_LEVEL_LABEL_30_31         (DVFSRC_BASE + 0xD68)
#define DVFSRC_CURRENT_FORCE             (DVFSRC_BASE + 0xD6C)
#define DVFSRC_TARGET_FORCE              (DVFSRC_BASE + 0xD70)
#define DVFSRC_EMI_ADD_REQUEST           (DVFSRC_BASE + 0xD74)

#endif /* __MT_SPM_VCOREFS__H__ */
