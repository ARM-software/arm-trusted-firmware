/*
 * Copyright (c) 2016-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables.h>
#include <plat/common/platform.h>
#include <services/arm_arch_svc.h>

#include <mtk_plat_common.h>
#include <mtk_sip_svc.h>
#include <plat_private.h>

void clean_top_32b_of_param(uint32_t smc_fid,
				u_register_t *px1,
				u_register_t *px2,
				u_register_t *px3,
				u_register_t *px4)
{
	/* if parameters from SMC32. Clean top 32 bits */
	if (GET_SMC_CC(smc_fid) == SMC_64) {
		*px1 = *px1 & SMC32_PARAM_MASK;
		*px2 = *px2 & SMC32_PARAM_MASK;
		*px3 = *px3 & SMC32_PARAM_MASK;
		*px4 = *px4 & SMC32_PARAM_MASK;
	}
}

/*****************************************************************************
 * plat_is_smccc_feature_available() - This function checks whether SMCCC
 *                                     feature is availabile for platform.
 * @fid: SMCCC function id
 *
 * Return SMC_OK if SMCCC feature is available and SMC_ARCH_CALL_NOT_SUPPORTED
 * otherwise.
 *****************************************************************************/
int32_t plat_is_smccc_feature_available(u_register_t fid)
{
	switch (fid) {
	case SMCCC_ARCH_SOC_ID:
		return SMC_ARCH_CALL_SUCCESS;
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}

int32_t plat_get_soc_version(void)
{
	uint32_t manfid = SOC_ID_SET_JEP_106(JEDEC_MTK_BKID, JEDEC_MTK_MFID);

	return (int32_t)(manfid | (SOC_CHIP_ID & SOC_ID_IMPL_DEF_MASK));
}

int32_t plat_get_soc_revision(void)
{
	return 0;
}
