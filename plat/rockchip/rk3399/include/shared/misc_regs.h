/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MISC_REGS_H
#define MISC_REGS_H

/* CRU */
#define CRU_DPLL_CON0		0x40
#define CRU_DPLL_CON1		0x44
#define CRU_DPLL_CON2		0x48
#define CRU_DPLL_CON3		0x4c
#define CRU_DPLL_CON4		0x50
#define CRU_DPLL_CON5		0x54

/* CRU_PLL_CON3 */
#define PLL_SLOW_MODE		0
#define PLL_NORMAL_MODE		1
#define PLL_MODE(n)		((0x3 << (8 + 16)) | ((n) << 8))
#define PLL_POWER_DOWN(n)	((0x1 << (0 + 16)) | ((n) << 0))

/* PMU CRU */
#define PMU_CRU_GATEDIS_CON0	0x130

#endif /* MISC_REGS_H */
