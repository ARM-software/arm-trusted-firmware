/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_WDOG_H
#define IMX_WDOG_H

#include <stdint.h>

#include <arch.h>

struct wdog_regs {
	uint16_t wcr;
	uint16_t wsr;
	uint16_t wrsr;
	uint16_t wicr;
	uint16_t wmcr;
};

/* WCR bits */
#define WCR_WDZST		BIT(0)
#define WCR_WDBG		BIT(1)
#define WCR_WDE			BIT(2)
#define WCR_WDT			BIT(3)
#define WCR_SRS			BIT(4)
#define WCR_WDA			BIT(5)
#define WCR_SRE			BIT(6)
#define WCR_WDW			BIT(7)
#define WCR_WT(x)		((x) << 8)

/* WSR bits */
#define WSR_FIRST		0x5555
#define WSR_SECOND		0xAAAA

/* WRSR bits */
#define WRSR_SFTW		BIT(0)
#define WRSR_TOUT		BIT(1)
#define WRSR_POR		BIT(4)

/* WICR bits */
static inline int wicr_calc_wict(int sec, int half_sec)
{
	int wict_bits;

	/* Represents WICR bits 7 - 0 */
	wict_bits = ((sec << 1) | (half_sec ? 1 : 0));

	return wict_bits;
}

#define WICR_WTIS		BIT(14)
#define WICR_WIE		BIT(15)

/* WMCR bits */
#define WMCR_PDE		BIT(0)

/* External facing API */
void imx_wdog_init(void);

#endif /* IMX_WDOG_H */
