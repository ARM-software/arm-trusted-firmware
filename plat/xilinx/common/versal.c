/*
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <services/arm_arch_svc.h>

#include <plat_private.h>
#include <plat_startup.h>
#include <pm_api_sys.h>

/**
 * plat_is_smccc_feature_available() - This function checks whether SMCCC
 *                                     feature is availabile for platform.
 * @fid: SMCCC function id.
 *
 * Return: SMC_ARCH_CALL_SUCCESS - if SMCCC feature is available.
 *         SMC_ARCH_CALL_NOT_SUPPORTED - Otherwise.
 *
 */
int32_t plat_is_smccc_feature_available(u_register_t fid)
{
	switch (fid) {
	case SMCCC_ARCH_SOC_ID:
		return SMC_ARCH_CALL_SUCCESS;
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}

/**
 * plat_get_soc_version() - Get the SOC version of the platform.
 *
 * Return: SiP defined SoC version in JEP-106.
 *
 * This function is called when the SoC_ID_type == 0.
 * For further details please refer to section 7.4 of SMC Calling Convention.
 */
int32_t plat_get_soc_version(void)
{
	uint32_t manfid;

	manfid = SOC_ID_SET_JEP_106(JEDEC_XILINX_BKID, JEDEC_XILINX_MFID);

	return (int32_t)(manfid | (platform_version & SOC_ID_IMPL_DEF_MASK));
}

/**
 * plat_get_soc_revision() - Get the SOC revision for the platform.
 *
 * Return: SiP defined SoC revision.
 *
 * This function is called when the  SoC_ID_type == 1
 * For further details please refer to section 7.4 of SMC Calling Convention
 */
int32_t plat_get_soc_revision(void)
{
	return (platform_id & SOC_ID_REV_MASK);
}
