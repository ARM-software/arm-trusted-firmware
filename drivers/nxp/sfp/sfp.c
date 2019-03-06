/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */


#include <common/debug.h>

#include <platform_def.h>
#include <io.h>
#include <sfp.h>
#include <sfp_error_codes.h>

int sfp_check_its(void)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)SFP_FUSE_REGS_ADDR;

	if (sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_ITS_MASK)
		return 1;
	else
		return 0;
}

int sfp_check_oem_wp(void)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)SFP_FUSE_REGS_ADDR;

	if (sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_WP_MASK)
		return 1;
	else
		return 0;
}
