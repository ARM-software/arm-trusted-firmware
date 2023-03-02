/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	/*
	 * As of now, there are no sources of Group0 secure interrupt enabled
	 * for FVP.
	 */
	(void)intid;
	return -1;
}
