/*
 * Copyright (c) 2022-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <emi_mpu.h>

void set_emi_mpu_regions(void)
{
	/* TODO: set emi mpu region */
	INFO("%s, emi mpu is not setting currently\n", __func__);
}

int set_apu_emi_mpu_region(void)
{
	struct emi_region_info_t region_info;

	region_info.start = (unsigned long long)APUSYS_SEC_BUF_PA;
	region_info.end = (unsigned long long)(APUSYS_SEC_BUF_PA + APUSYS_SEC_BUF_SZ) - 1;
	region_info.region = APUSYS_SEC_BUF_EMI_REGION;

	SET_ACCESS_PERMISSION(region_info.apc, UNLOCK,
			      FORBIDDEN,     FORBIDDEN, FORBIDDEN,     FORBIDDEN,
			      FORBIDDEN,     FORBIDDEN, FORBIDDEN,     FORBIDDEN,
			      NO_PROTECTION, FORBIDDEN, NO_PROTECTION, FORBIDDEN,
			      FORBIDDEN,     FORBIDDEN, FORBIDDEN,     SEC_RW);

	return emi_mpu_set_protection(&region_info);
}
