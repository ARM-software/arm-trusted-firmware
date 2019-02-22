/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <plat_common.h>
#include <io.h>
#include <sfp.h>

/*******************************************************************************
 * Returns true if secur eboot is enabled on board
 * mode = 0  (development mode - sb_en = 1)
 * mode = 1 (production mode - ITS = 1)
 ******************************************************************************/
bool check_boot_mode_secure(uint32_t *mode)
{
	uint32_t val = 0;
	uint32_t *rcwsr = (void *)(NXP_DCFG_ADDR + RCWSR_SB_EN_OFFSET);
	*mode = 0;

	if (sfp_check_its() == 1) {
		/* ITS =1 , Production mode */
		*mode = 1;
		return true;
	}

	val = (gur_in32(rcwsr) >> RCWSR_SBEN_SHIFT) &
				RCWSR_SBEN_MASK;

	if (val == RCWSR_SBEN_MASK) {
		*mode = 0;
		return true;
	}

	return false;
}
