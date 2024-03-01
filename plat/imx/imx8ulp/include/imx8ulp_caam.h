/*
 * Copyright 2021-2024 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX8ULP_CAAM_H
#define IMX8ULP_CAAM_H

#include <lib/utils_def.h>

#include <platform_def.h>

#define CAAM_JR0MID		(IMX_CAAM_BASE + 0x10)
#define CAAM_JR1MID		(IMX_CAAM_BASE + 0x18)
#define CAAM_JR2MID		(IMX_CAAM_BASE + 0x20)
#define CAAM_JR3MID		(IMX_CAAM_BASE + 0x28)
#define CAAM_NS_MID		(0x7)

#define JR0_BASE		(IMX_CAAM_BASE + 0x1000)

void imx8ulp_caam_init(void);

#endif /* IMX8ULP_CAAM_H */
