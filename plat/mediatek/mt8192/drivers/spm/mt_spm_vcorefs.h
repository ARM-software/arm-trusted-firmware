/*
 * Copyright(C)2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_VCOREFS_H
#define MT_SPM_VCOREFS_H

uint64_t spm_vcorefs_args(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t *x4);

enum vcorefs_smc_cmd {
	VCOREFS_SMC_CMD_0,
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
#define DVFSRC_SW_REQ5                   (DVFSRC_BASE + 0x14)
#define DVFSRC_INT_EN                    (DVFSRC_BASE + 0xC8)
#define DVFSRC_MD_TURBO                  (DVFSRC_BASE + 0xDC)
#define DVFSRC_PCIE_VCORE_REQ            (DVFSRC_BASE + 0xE0)
#define DVFSRC_VCORE_USER_REQ            (DVFSRC_BASE + 0xE4)
#define DVFSRC_TIMEOUT_NEXTREQ           (DVFSRC_BASE + 0xF8)
#define DVFSRC_LEVEL_LABEL_0_1           (DVFSRC_BASE + 0x100)
#define DVFSRC_LEVEL_LABEL_2_3           (DVFSRC_BASE + 0x104)
#define DVFSRC_LEVEL_LABEL_4_5           (DVFSRC_BASE + 0x108)
#define DVFSRC_LEVEL_LABEL_6_7           (DVFSRC_BASE + 0x10C)
#define DVFSRC_LEVEL_LABEL_8_9           (DVFSRC_BASE + 0x110)
#define DVFSRC_LEVEL_LABEL_10_11         (DVFSRC_BASE + 0x114)
#define DVFSRC_LEVEL_LABEL_12_13         (DVFSRC_BASE + 0x118)
#define DVFSRC_LEVEL_LABEL_14_15         (DVFSRC_BASE + 0x11C)
#define DVFSRC_QOS_EN                    (DVFSRC_BASE + 0x280)
#define DVFSRC_HRT_BW_BASE               (DVFSRC_BASE + 0x294)
#define DVFSRC_RSRV_4                    (DVFSRC_BASE + 0x610)
#define DVFSRC_RSRV_5                    (DVFSRC_BASE + 0x614)
#define DVFSRC_DDR_REQUEST               (DVFSRC_BASE + 0xA00)
#define DVFSRC_DDR_REQUEST2              (DVFSRC_BASE + 0xA04)
#define DVFSRC_DDR_REQUEST3              (DVFSRC_BASE + 0xA08)
#define DVFSRC_DDR_REQUEST4              (DVFSRC_BASE + 0xA0C)
#define DVFSRC_DDR_REQUEST5              (DVFSRC_BASE + 0xA10)
#define DVFSRC_DDR_REQUEST6              (DVFSRC_BASE + 0xA14)
#define DVFSRC_DDR_REQUEST7              (DVFSRC_BASE + 0xA18)
#define DVFSRC_DDR_QOS0                  (DVFSRC_BASE + 0xA34)
#define DVFSRC_DDR_QOS1                  (DVFSRC_BASE + 0xA38)
#define DVFSRC_DDR_QOS2                  (DVFSRC_BASE + 0xA3C)
#define DVFSRC_DDR_QOS3                  (DVFSRC_BASE + 0xA40)
#define DVFSRC_DDR_QOS4                  (DVFSRC_BASE + 0xA44)
#define DVFSRC_HRT_REQ_UNIT              (DVFSRC_BASE + 0xA60)
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
#define DVFSRC_HRT_HIGH_3                (DVFSRC_BASE + 0xD38)
#define DVFSRC_HRT_LOW_3                 (DVFSRC_BASE + 0xD3C)
#define DVFSRC_LEVEL_LABEL_16_17         (DVFSRC_BASE + 0xD4C)
#define DVFSRC_LEVEL_LABEL_18_19         (DVFSRC_BASE + 0xD50)
#define DVFSRC_LEVEL_LABEL_20_21         (DVFSRC_BASE + 0xD54)
#define DVFSRC_LEVEL_LABEL_22_23         (DVFSRC_BASE + 0xD58)
#define DVFSRC_LEVEL_LABEL_24_25         (DVFSRC_BASE + 0xD5C)
#define DVFSRC_LEVEL_LABEL_26_27         (DVFSRC_BASE + 0xD60)
#define DVFSRC_LEVEL_LABEL_28_29         (DVFSRC_BASE + 0xD64)
#define DVFSRC_LEVEL_LABEL_30_31         (DVFSRC_BASE + 0xD68)
#define DVFSRC_CURRENT_FORCE             (DVFSRC_BASE + 0xD6C)

#define VCORE_VB_EFUSE	(0x11C105E8)

#endif /* MT_SPM_VCOREFS_H */
