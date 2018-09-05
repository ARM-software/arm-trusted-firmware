/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX_AIPS_H__
#define __IMX_AIPS_H__

#include <stdint.h>

#define AIPSTZ_OAPCR_COUNT	0x05

struct aipstz_regs {
	uint32_t aipstz_mpr;
	uint32_t res[15];
	uint32_t aipstz_opacr[AIPSTZ_OAPCR_COUNT];
};

void imx_aips_init(void);

#endif /* __IMX_AIPS_H__ */
