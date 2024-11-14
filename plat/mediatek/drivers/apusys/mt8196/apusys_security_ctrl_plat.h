/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_SECURITY_CTRL_PLAT_H
#define APUSYS_SECURITY_CTRL_PLAT_H

#include <lib/utils_def.h>
#include <platform_def.h>

#define SEC_CTRL_SOC2APU_SET1_0		(APU_SEC_CON + 0xC)
#define SEC_CTRL_SOC2APU_SET1_1		(APU_SEC_CON + 0x10)
#define SEC_CTRL_SIDE_BAND		(APU_SEC_CON + 0x24)

#define SEC_CTRL_REG_DOMAIN_NUM		(8)
#define SEC_CTRL_DOMAIN_REMAP_SEL	BIT(6)
#define SEC_CTRL_DOMAIN_MASK		(0xF)
#define SEC_CTRL_NS_MASK		(0x1)

#define SEC_CTRL_NARE_DOMAIN		(5)
#define SEC_CTRL_NARE_NS		(0)
#define SEC_CTRL_NARE_DOMAIN_SHF	(0)
#define SEC_CTRL_NARE_NS_SHF		(4)

#define SEC_CTRL_SARE0_DOMAIN		(5)
#define SEC_CTRL_SARE0_NS		(0)
#define SEC_CTRL_SARE0_DOMAIN_SHF	(5)
#define SEC_CTRL_SARE0_NS_SHF		(9)

#define SEC_CTRL_SARE1_DOMAIN		(5)
#define SEC_CTRL_SARE1_NS		(0)
#define SEC_CTRL_SARE1_DOMAIN_SHF	(10)
#define SEC_CTRL_SARE1_NS_SHF		(14)

#define REG_DOMAIN_BITS		(4)

#define D0_REMAP_DOMAIN		(0)
#define D1_REMAP_DOMAIN		(1)
#define D2_REMAP_DOMAIN		(2)
#define D3_REMAP_DOMAIN		(3)
#define D4_REMAP_DOMAIN		(4)
#define D5_REMAP_DOMAIN		(6)
#define D6_REMAP_DOMAIN		(6)
#define D7_REMAP_DOMAIN		(6)
#define D8_REMAP_DOMAIN		(8)
#define D9_REMAP_DOMAIN		(9)
#define D10_REMAP_DOMAIN	(10)
#define D11_REMAP_DOMAIN	(11)
#define D12_REMAP_DOMAIN	(12)
#define D13_REMAP_DOMAIN	(13)
#define D14_REMAP_DOMAIN	(6)
#define D15_REMAP_DOMAIN	(15)

void apusys_security_ctrl_init(void);
int apusys_plat_setup_sec_mem(void);

#endif /* APUSYS_SECURITY_CTRL_PLAT_H */
