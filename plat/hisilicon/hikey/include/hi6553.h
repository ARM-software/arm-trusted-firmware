/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HI6553_H__
#define __HI6553_H__

#include <hi6220.h>
#include <mmio.h>

#define HI6553_DISABLE6_XO_CLK			(PMUSSI_BASE + (0x036 << 2))

#define DISABLE6_XO_CLK_BB			(1 << 0)
#define DISABLE6_XO_CLK_CONN			(1 << 1)
#define DISABLE6_XO_CLK_NFC			(1 << 2)
#define DISABLE6_XO_CLK_RF1			(1 << 3)
#define DISABLE6_XO_CLK_RF2			(1 << 4)

#define HI6553_VERSION_REG			(PMUSSI_BASE + (0x000 << 2))
#define HI6553_IRQ2_MASK			(PMUSSI_BASE + (0x008 << 2))
#define HI6553_ENABLE2_LDO1_8			(PMUSSI_BASE + (0x029 << 2))
#define HI6553_DISABLE2_LDO1_8			(PMUSSI_BASE + (0x02a << 2))
#define HI6553_ONOFF_STATUS2_LDO1_8		(PMUSSI_BASE + (0x02b << 2))
#define HI6553_ENABLE3_LDO9_16			(PMUSSI_BASE + (0x02c << 2))
#define HI6553_DISABLE3_LDO9_16			(PMUSSI_BASE + (0x02d << 2))
#define HI6553_ONOFF_STATUS3_LDO9_16		(PMUSSI_BASE + (0x02e << 2))
#define HI6553_ENABLE4_LDO17_22			(PMUSSI_BASE + (0x02f << 2))
#define HI6553_DISABLE4_LDO17_22		(PMUSSI_BASE + (0x030 << 2))
#define HI6553_ONOFF_STATUS4_LDO17_22		(PMUSSI_BASE + (0x031 << 2))
#define HI6553_PERI_EN_MARK			(PMUSSI_BASE + (0x040 << 2))
#define HI6553_BUCK2_REG1			(PMUSSI_BASE + (0x04a << 2))
#define HI6553_BUCK2_REG5			(PMUSSI_BASE + (0x04e << 2))
#define HI6553_BUCK2_REG6			(PMUSSI_BASE + (0x04f << 2))
#define HI6553_BUCK3_REG3			(PMUSSI_BASE + (0x054 << 2))
#define HI6553_BUCK3_REG5			(PMUSSI_BASE + (0x056 << 2))
#define HI6553_BUCK3_REG6			(PMUSSI_BASE + (0x057 << 2))
#define HI6553_BUCK4_REG2			(PMUSSI_BASE + (0x05b << 2))
#define HI6553_BUCK4_REG5			(PMUSSI_BASE + (0x05e << 2))
#define HI6553_BUCK4_REG6			(PMUSSI_BASE + (0x05f << 2))
#define HI6553_CLK_TOP0				(PMUSSI_BASE + (0x063 << 2))
#define HI6553_CLK_TOP3				(PMUSSI_BASE + (0x066 << 2))
#define HI6553_CLK_TOP4				(PMUSSI_BASE + (0x067 << 2))
#define HI6553_VSET_BUCK2_ADJ			(PMUSSI_BASE + (0x06d << 2))
#define HI6553_VSET_BUCK3_ADJ			(PMUSSI_BASE + (0x06e << 2))
#define HI6553_LDO7_REG_ADJ			(PMUSSI_BASE + (0x078 << 2))
#define HI6553_LDO10_REG_ADJ			(PMUSSI_BASE + (0x07b << 2))
#define HI6553_LDO15_REG_ADJ			(PMUSSI_BASE + (0x080 << 2))
#define HI6553_LDO19_REG_ADJ			(PMUSSI_BASE + (0x084 << 2))
#define HI6553_LDO20_REG_ADJ			(PMUSSI_BASE + (0x085 << 2))
#define HI6553_LDO21_REG_ADJ			(PMUSSI_BASE + (0x086 << 2))
#define HI6553_LDO22_REG_ADJ			(PMUSSI_BASE + (0x087 << 2))
#define HI6553_DR_LED_CTRL			(PMUSSI_BASE + (0x098 << 2))
#define HI6553_DR_OUT_CTRL			(PMUSSI_BASE + (0x099 << 2))
#define HI6553_DR3_ISET				(PMUSSI_BASE + (0x09a << 2))
#define HI6553_DR3_START_DEL			(PMUSSI_BASE + (0x09b << 2))
#define HI6553_DR4_ISET				(PMUSSI_BASE + (0x09c << 2))
#define HI6553_DR4_START_DEL			(PMUSSI_BASE + (0x09d << 2))
#define HI6553_DR345_TIM_CONF0			(PMUSSI_BASE + (0x0a0 << 2))
#define HI6553_NP_REG_ADJ1			(PMUSSI_BASE + (0x0be << 2))
#define HI6553_NP_REG_CHG			(PMUSSI_BASE + (0x0c0 << 2))
#define HI6553_BUCK01_CTRL2			(PMUSSI_BASE + (0x0d9 << 2))
#define HI6553_BUCK0_CTRL1			(PMUSSI_BASE + (0x0dd << 2))
#define HI6553_BUCK0_CTRL5			(PMUSSI_BASE + (0x0e1 << 2))
#define HI6553_BUCK0_CTRL7			(PMUSSI_BASE + (0x0e3 << 2))
#define HI6553_BUCK1_CTRL1			(PMUSSI_BASE + (0x0e8 << 2))
#define HI6553_BUCK1_CTRL5			(PMUSSI_BASE + (0x0ec << 2))
#define HI6553_BUCK1_CTRL7			(PMUSSI_BASE + (0x0ef << 2))
#define HI6553_CLK19M2_600_586_EN		(PMUSSI_BASE + (0x0fe << 2))

#define LED_START_DELAY_TIME			0x00
#define LED_ELEC_VALUE				0x07
#define LED_LIGHT_TIME				0xf0
#define LED_GREEN_ENABLE			(1 << 1)
#define LED_OUT_CTRL				0x00

#define PMU_HI6552_V300				0x30
#define PMU_HI6552_V310				0x31

#endif	/* __HI6553_H__ */
