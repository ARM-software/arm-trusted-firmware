/*
 * Copyright (C) 2025 Texas Instruments Incorporated - http://www.ti.com/
 * K3 SOC specific bl31_setup
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <ti_sci.h>
#include <ti_sci_transport.h>

#include <plat_private.h>

/* Table of regions to map using the MMU */
const mmap_region_t plat_k3_mmap[] = {
	{ /* sentinel */ }
};

int ti_soc_init(void)
{
	/* nothing to do right now */
	return 0;
}
