/*
 * Copyright 2021-2024 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <imx8ulp_caam.h>

void imx8ulp_caam_init(void)
{
	/* config CAAM JRaMID set MID to Cortex A */
	mmio_write_32(CAAM_JR0MID, CAAM_NS_MID);
	mmio_write_32(CAAM_JR1MID, CAAM_NS_MID);
	mmio_write_32(CAAM_JR2MID, CAAM_NS_MID);
	mmio_write_32(CAAM_JR3MID, CAAM_NS_MID);
}
