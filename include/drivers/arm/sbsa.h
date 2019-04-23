/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBSA_H
#define SBSA_H

#include <stdint.h>

/* Register Offsets */
#define SBSA_WDOG_WCS_OFFSET		UL(0x000)
#define SBSA_WDOG_WOR_LOW_OFFSET	UL(0x008)
#define SBSA_WDOG_WOR_HIGH_OFFSET	UL(0x00C)

#define SBSA_WDOG_WCS_EN		U(0x1)

#define SBSA_WDOG_WOR_WIDTH		UL(48)

void sbsa_wdog_start(uintptr_t base, uint64_t ms);
void sbsa_wdog_stop(uintptr_t base);

#endif /* SBSA_H */
