/*
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <imx_aipstz.h>

void imx_aipstz_init(const struct aipstz_cfg *aipstz_cfg)
{
	const struct aipstz_cfg *aipstz = aipstz_cfg;

	while (aipstz->base != 0U) {
		mmio_write_32(aipstz->base + AIPSTZ_MPR0, aipstz->mpr0);
		mmio_write_32(aipstz->base + AIPSTZ_MPR1, aipstz->mpr1);

		for (int i = 0; i < AIPSTZ_OPACR_NUM; i++)
			mmio_write_32(aipstz->base + OPACR_OFFSET(i), aipstz->opacr[i]);

		aipstz++;
	}
}
