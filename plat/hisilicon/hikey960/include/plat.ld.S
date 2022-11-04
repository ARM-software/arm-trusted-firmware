/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_LD_S
#define PLAT_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>

MEMORY {
    RAM2 (rw): ORIGIN = DDR2_SEC_BASE, LENGTH = DDR2_SEC_SIZE
}

SECTIONS
{
	ram2_region (NOLOAD) : {
	*(ram2_region)
	}>RAM2
}

#endif /* PLAT_LD_S */
