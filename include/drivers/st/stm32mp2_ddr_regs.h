/*
 * Copyright (c) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#ifndef STM32MP2_DDR_REGS_H
#define STM32MP2_DDR_REGS_H

#include <drivers/st/stm32mp_ddrctrl_regs.h>
#include <lib/utils_def.h>

/* DDR Physical Interface Control (DDRPHYC) registers*/
struct stm32mp_ddrphy {
	uint32_t dummy;
} __packed;

/* DDRPHY registers offsets */
#define DDRPHY_INITENG0_P0_SEQ0BDISABLEFLAG6	U(0x240004)
#define DDRPHY_INITENG0_P0_PHYINLPX		U(0x2400A0)
#define DDRPHY_DRTUB0_UCCLKHCLKENABLES		U(0x300200)
#define DDRPHY_APBONLY0_MICROCONTMUXSEL		U(0x340000)

/* DDRPHY registers fields */
#define DDRPHY_INITENG0_P0_PHYINLPX_PHYINLP3			BIT(0)
#define DDRPHY_DRTUB0_UCCLKHCLKENABLES_UCCLKEN			BIT(0)
#define DDRPHY_DRTUB0_UCCLKHCLKENABLES_HCLKEN			BIT(1)
#define DDRPHY_APBONLY0_MICROCONTMUXSEL_MICROCONTMUXSEL		BIT(0)

/* DDRDBG registers offsets */
#define DDRDBG_LP_DISABLE			U(0x0)
#define DDRDBG_BYPASS_PCLKEN			U(0x4)

/* DDRDBG registers fields */
#define DDRDBG_LP_DISABLE_LPI_XPI_DISABLE	BIT(0)
#define DDRDBG_LP_DISABLE_LPI_DDRC_DISABLE	BIT(8)

#endif /* STM32MP2_DDR_REGS_H */
