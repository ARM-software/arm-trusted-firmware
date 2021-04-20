/*
 * Copyright (C) 2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef EFUSE_DEF_H
#define EFUSE_DEF_H

#include <platform_def.h>

#define MVEBU_AP_EFUSE_SRV_CTRL_REG	(MVEBU_AP_GEN_MGMT_BASE + 0x8)
#define EFUSE_SRV_CTRL_LD_SELECT_OFFS	6
#define EFUSE_SRV_CTRL_LD_SELECT_MASK	(1 << EFUSE_SRV_CTRL_LD_SELECT_OFFS)

#define MVEBU_AP_LD_EFUSE_BASE		(MVEBU_AP_GEN_MGMT_BASE + 0xF00)
/* Bits [31:0] - 32 data bits total */
#define MVEBU_AP_LDX_31_0_EFUSE_OFFS	(MVEBU_AP_LD_EFUSE_BASE)
/* Bits [62:32] - 31 data bits total 32nd bit is parity for bits [62:0]*/
#define MVEBU_AP_LDX_62_32_EFUSE_OFFS	(MVEBU_AP_LD_EFUSE_BASE + 0x4)
/* Bits [94:63] - 32 data bits total */
#define MVEBU_AP_LDX_94_63_EFUSE_OFFS	(MVEBU_AP_LD_EFUSE_BASE + 0x8)
/* Bits [125:95] - 31 data bits total, 32nd bit is parity for bits [125:63] */
#define MVEBU_AP_LDX_125_95_EFUSE_OFFS	(MVEBU_AP_LD_EFUSE_BASE + 0xC)
/* Bits [157:126] - 32 data bits total */
#define MVEBU_AP_LDX_126_157_EFUSE_OFFS	(MVEBU_AP_LD_EFUSE_BASE + 0x10)
/* Bits [188:158] - 31 data bits total, 32nd bit is parity for bits [188:126] */
#define MVEBU_AP_LDX_188_158_EFUSE_OFFS	(MVEBU_AP_LD_EFUSE_BASE + 0x14)
/* Bits [220:189] - 32 data bits total */
#define MVEBU_AP_LDX_220_189_EFUSE_OFFS	(MVEBU_AP_LD_EFUSE_BASE + 0x18)

#endif /* EFUSE_DEF_H */
