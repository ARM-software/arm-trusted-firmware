/*
 * Copyright (c) 2015 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>

#include <platform_def.h>
#include <sbl_util.h>
#include <sotp.h>

#pragma weak plat_sbl_status

int plat_sbl_status(uint64_t sbl_status)
{
	return sbl_status ? 1:0;
}

int sbl_status(void)
{
	uint64_t sbl_sotp = 0;
	int ret = SBL_DISABLED;

	sbl_sotp = sotp_mem_read(SOTP_ATF_CFG_ROW_ID, SOTP_ROW_NO_ECC);

	if (sbl_sotp != SOTP_ECC_ERR_DETECT) {

		sbl_sotp &= SOTP_SBL_MASK;

		if (plat_sbl_status(sbl_sotp))
			ret = SBL_ENABLED;
	}

	VERBOSE("SBL status: %d\n", ret);

	return ret;
}
