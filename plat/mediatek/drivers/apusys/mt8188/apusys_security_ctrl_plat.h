/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_SECURITY_CTRL_PLAT_H
#define APUSYS_SECURITY_CTRL_PLAT_H

#include <platform_def.h>

#define SOC2APU_SET1_0	(APU_SEC_CON + 0x0c)
#define SOC2APU_SET1_1	(APU_SEC_CON + 0x10)

#define REG_DOMAIN_NUM		(8)
#define REG_DOMAIN_BITS		(4)
#define DOMAIN_REMAP_SEL	BIT(6)

#define D0_REMAP_DOMAIN		(0)
#define D1_REMAP_DOMAIN		(1)
#define D2_REMAP_DOMAIN		(2)
#define D3_REMAP_DOMAIN		(3)
#define D4_REMAP_DOMAIN		(4)
#define D5_REMAP_DOMAIN		(14)
#define D6_REMAP_DOMAIN		(6)
#define D7_REMAP_DOMAIN		(14)
#define D8_REMAP_DOMAIN		(8)
#define D9_REMAP_DOMAIN		(9)
#define D10_REMAP_DOMAIN	(10)
#define D11_REMAP_DOMAIN	(11)
#define D12_REMAP_DOMAIN	(12)
#define D13_REMAP_DOMAIN	(13)
#define D14_REMAP_DOMAIN	(14)
#define D15_REMAP_DOMAIN	(15)

void apusys_security_ctrl_init(void);

#endif /* APUSYS_SECURITY_CTRL_PLAT_H */
