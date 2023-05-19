/*
 * Copyright (c) 2020-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/arm/css/sds.h>
#include <lib/smccc.h>
#include <plat/arm/common/plat_arm.h>
#include <services/arm_arch_svc.h>

#include "morello_def.h"
#include "morello_private.h"
#include <platform_def.h>

#ifdef TARGET_PLATFORM_SOC
struct morello_plat_info plat_info;
#endif

static scmi_channel_plat_info_t morello_scmi_plat_info = {
	.scmi_mbx_mem = MORELLO_SCMI_PAYLOAD_BASE,
	.db_reg_addr = PLAT_CSS_MHU_BASE + CSS_SCMI_MHU_DB_REG_OFF,
	.db_preserve_mask = 0xfffffffe,
	.db_modify_mask = 0x1,
	.ring_doorbell = &mhu_ring_doorbell
};

scmi_channel_plat_info_t *plat_css_get_scmi_info(unsigned int channel_id)
{
	return &morello_scmi_plat_info;
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	ops->pwr_domain_off = morello_pwr_domain_off;
	return css_scmi_override_pm_ops(ops);
}

void bl31_platform_setup(void)
{
#ifdef TARGET_PLATFORM_SOC
	int ret;

	ret = sds_init();
	if (ret != SDS_OK) {
		ERROR("SDS initialization failed. ret:%d\n", ret);
		panic();
	}

	ret = sds_struct_read(MORELLO_SDS_PLATFORM_INFO_STRUCT_ID,
				MORELLO_SDS_PLATFORM_INFO_OFFSET,
				&plat_info,
				MORELLO_SDS_PLATFORM_INFO_SIZE,
				SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK) {
		ERROR("Error getting platform info from SDS. ret:%d\n", ret);
		panic();
	}
#endif
	arm_bl31_platform_setup();
}

#ifdef TARGET_PLATFORM_SOC
/*****************************************************************************
 * plat_is_smccc_feature_available() - This function checks whether SMCCC
 *                                     feature is availabile for platform.
 * @fid: SMCCC function id
 *
 * Return SMC_ARCH_CALL_SUCCESS if SMCCC feature is available and
 * SMC_ARCH_CALL_NOT_SUPPORTED otherwise.
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

/* Get SOC version */
int32_t plat_get_soc_version(void)
{
	int ssc_version;

	ssc_version = mmio_read_32(SSC_VERSION);

	return (int32_t)
		(SOC_ID_SET_JEP_106(ARM_SOC_CONTINUATION_CODE,
					ARM_SOC_IDENTIFICATION_CODE) |
		(GET_SSC_VERSION_PART_NUM(ssc_version) & SOC_ID_IMPL_DEF_MASK));
}

/* Get SOC revision */
int32_t plat_get_soc_revision(void)
{
	return (int32_t)plat_info.silicon_revision;
}
#endif
