// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Macros IDs for AMD Versal Gen 2
 *
 * Copyright (c) 2023-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#ifndef _VERSAL2_SCMI_H
#define _VERSAL2_SCMI_H

#define CLK_GEM0_0	0U
#define CLK_GEM0_1	1U
#define CLK_GEM0_2	2U
#define CLK_GEM0_3	3U
#define CLK_GEM0_4	4U
#define CLK_GEM1_0	5U
#define CLK_GEM1_1	6U
#define CLK_GEM1_2	7U
#define CLK_GEM1_3	8U
#define CLK_GEM1_4	9U
#define CLK_SERIAL0_0	10U
#define CLK_SERIAL0_1	11U
#define CLK_SERIAL1_0	12U
#define CLK_SERIAL1_1	13U
#define CLK_UFS0_0	14U
#define CLK_UFS0_1	15U
#define CLK_UFS0_2	16U
#define CLK_USB0_0	17U
#define CLK_USB0_1	18U
#define CLK_USB0_2	19U
#define CLK_USB1_0	20U
#define CLK_USB1_1	21U
#define CLK_USB1_2	22U
#define CLK_MMC0_0	23U
#define CLK_MMC0_1	24U
#define CLK_MMC0_2	25U
#define CLK_MMC1_0	26U
#define CLK_MMC1_1	27U
#define CLK_MMC1_2	28U
#define CLK_TTC0_0	29U
#define CLK_TTC1_0	30U
#define CLK_TTC2_0	31U
#define CLK_TTC3_0	32U
#define CLK_TTC4_0	33U
#define CLK_TTC5_0	34U
#define CLK_TTC6_0	35U
#define CLK_TTC7_0	36U
#define CLK_I2C0_0	37U
#define CLK_I2C1_0	38U
#define CLK_I2C2_0	39U
#define CLK_I2C3_0	40U
#define CLK_I2C4_0	41U
#define CLK_I2C5_0	42U
#define CLK_I2C6_0	43U
#define CLK_I2C7_0	44U
#define CLK_OSPI0_0	45U
#define CLK_QSPI0_0	46U
#define CLK_QSPI0_1	47U
#define CLK_WWDT0_0	48U
#define CLK_WWDT1_0	49U
#define CLK_WWDT2_0	50U
#define CLK_WWDT3_0	51U
#define CLK_ADMA0_0	52U
#define CLK_ADMA0_1	53U
#define CLK_ADMA1_0	54U
#define CLK_ADMA1_1	55U
#define CLK_ADMA2_0	56U
#define CLK_ADMA2_1	57U
#define CLK_ADMA3_0	58U
#define CLK_ADMA3_1	59U
#define CLK_ADMA4_0	60U
#define CLK_ADMA4_1	61U
#define CLK_ADMA5_0	62U
#define CLK_ADMA5_1	63U
#define CLK_ADMA6_0	64U
#define CLK_ADMA6_1	65U
#define CLK_ADMA7_0	66U
#define CLK_ADMA7_1	67U
#define CLK_CAN0_0	68U
#define CLK_CAN0_1	69U
#define CLK_CAN1_0	70U
#define CLK_CAN1_1	71U
#define CLK_CAN2_0	72U
#define CLK_CAN2_1	73U
#define CLK_CAN3_0	74U
#define CLK_CAN3_1	75U
#define CLK_PS_GPIO_0	76U
#define CLK_PMC_GPIO_0	77U
#define CLK_SPI0_0	78U
#define CLK_SPI0_1	79U
#define CLK_SPI1_0	80U
#define CLK_SPI1_1	81U
#define CLK_I3C0_0	82U
#define CLK_I3C1_0	83U
#define CLK_I3C2_0	84U
#define CLK_I3C3_0	85U
#define CLK_I3C4_0	86U
#define CLK_I3C5_0	87U
#define CLK_I3C6_0	88U
#define CLK_I3C7_0	89U

#define RESET_GEM0_0	0
#define RESET_GEM1_0	1
#define RESET_SERIAL0_0	2
#define RESET_SERIAL1_0	3
#define RESET_UFS0_0	4
#define RESET_I2C0_0	5
#define RESET_I2C1_0	6
#define RESET_I2C2_0	7
#define RESET_I2C3_0	8
#define RESET_I2C4_0	9
#define RESET_I2C5_0	10
#define RESET_I2C6_0	11
#define RESET_I2C7_0	12
#define RESET_I2C8_0	13
#define RESET_OSPI0_0	14
#define RESET_USB0_0	15
#define RESET_USB0_1	16
#define RESET_USB0_2	17
#define RESET_USB1_0	18
#define RESET_USB1_1	19
#define RESET_USB1_2	20
#define RESET_MMC0_0	21
#define RESET_MMC1_0	22
#define RESET_SPI0_0	23
#define RESET_SPI1_0	24
#define RESET_QSPI0_0	25
#define RESET_I3C0_0	26
#define RESET_I3C1_0	27
#define RESET_I3C2_0	28
#define RESET_I3C3_0	29
#define RESET_I3C4_0	30
#define RESET_I3C5_0	31
#define RESET_I3C6_0	32
#define RESET_I3C7_0	33
#define RESET_I3C8_0	34
#define RESET_UFSPHY_0  35

#define PD_USB0		0
#define PD_USB1		1

#endif /* _VERSAL2_SCMI_H */
