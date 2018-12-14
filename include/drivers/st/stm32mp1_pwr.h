/*
 * Copyright (c) 2017-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_PWR_H
#define STM32MP1_PWR_H

#include <lib/utils_def.h>

#define PWR_CR1			U(0x00)
#define PWR_CR2			U(0x08)
#define PWR_CR3			U(0x0C)
#define PWR_MPUCR		U(0x10)
#define PWR_WKUPCR		U(0x20)
#define PWR_MPUWKUPENR		U(0x28)

#define PWR_CR1_LPDS		BIT(0)
#define PWR_CR1_LPCFG		BIT(1)
#define PWR_CR1_LVDS		BIT(2)
#define PWR_CR1_DBP		BIT(8)

#define PWR_CR3_DDRSREN		BIT(10)
#define PWR_CR3_DDRSRDIS	BIT(11)
#define PWR_CR3_DDRRETEN	BIT(12)

#define PWR_MPUCR_PDDS		BIT(0)
#define PWR_MPUCR_CSTDBYDIS	BIT(3)
#define PWR_MPUCR_CSSF		BIT(9)

#endif /* STM32MP1_PWR_H */
