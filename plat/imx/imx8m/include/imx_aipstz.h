/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_AIPSTZ_H
#define IMX_AIPSTZ_H

#include <lib/utils_def.h>

#define AIPSTZ_MPR0		U(0x0)
#define AIPSTZ_MPR1		U(0x4)

#define AIPSTZ_OPACR_NUM	U(0x5)
#define OPACR_OFFSET(i)		U((i) * 4 + 0x40)

struct aipstz_cfg {
	uintptr_t base;
	uint32_t mpr0;
	uint32_t mpr1;
	uint32_t opacr[AIPSTZ_OPACR_NUM];
};

void imx_aipstz_init(const struct aipstz_cfg *aipstz_cfg);

#endif /* IMX_AIPSTZ_H */
