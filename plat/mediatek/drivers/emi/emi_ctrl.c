/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <assert.h>
#include <lib/mtk_init/mtk_init.h>
#include <lib/utils_def.h>
#include <mtk_bl31_interface.h>
#include <mtk_sip_svc.h>
#include <platform_def.h>

#define NO_PROTECTION	0
#define SEC_RW		1
#define SEC_RW_NSEC_R	2
#define SEC_RW_NSEC_W	3
#define SEC_R_NSEC_R	4
#define FORBIDDEN	5
#define SEC_R_NSEC_RW	6

#define EMIMPU_SET	0
#define EMIMPU_CLEAR	1
#define EMIMPU_READ	2
#define EMIMPU_SLVERR	3
#define EMIDBG_DUMP	4
#define EMIDBG_MSG	5
#define AID_TABLE_SET	6
#define EMIMPU_CLEAR_MD	7
#define KP_SET		8
#define KP_CLEAR	9

#define EMIMPU_READ_SA			0
#define EMIMPU_READ_EA			1
#define EMIMPU_READ_APC			2
#define EMIMPU_READ_ENABLE		3
#define EMIMPU_READ_AID			4
#define EMIMPU_CHECK_NS_CPU		5
#define EMIMPU_CHECK_REGION_INFO	6
#define EMIMPU_PAGE_BASE_REGION		7
#define SLBMPU_CLEAR			8
#define EMIMPU_CHECK_HP_MOD		9
#define EMI_CLE				10
#define SLC_PARITY_SELECT		11
#define SLC_PARITY_CLEAR		12

struct emi_region {
	uint64_t start;
	size_t size;
	unsigned int region_id;
};

static const struct emi_region init_regions[] = {
	{TZRAM_BASE, TZRAM_SIZE, BL31_EMI_REGION_ID},
#ifndef SPD_NONE
	{BL32_REGION_BASE, BL32_REGION_SIZE, BL32_EMI_REGION_ID},
#endif
	{GPUMPU_BASE, GPUMPU_SIZE, GPUMPU_EMI_REGION_ID},
#if CONFIG_MTK_GPUEB
	{GPUEB_SHARED_BASE, GPUEB_SHARED_SIZE, GPUEB_SHARED_EMI_REGION_ID},
#endif
};

static uint64_t emi_mpu_read_by_type(unsigned int reg_type, unsigned int region,
				     unsigned int aid_shift, struct smccc_res *smccc_ret)
{
	switch (reg_type) {
	case EMIMPU_READ_SA:
		return emi_mpu_read_addr(region, 0x0);
	case EMIMPU_READ_EA:
		return emi_mpu_read_addr(region, 0x8);
	case EMIMPU_READ_ENABLE:
		return emi_mpu_read_enable(region);
	case EMIMPU_READ_AID:
		return emi_mpu_read_aid(region, aid_shift);
	case EMIMPU_CHECK_REGION_INFO:
		return emi_mpu_check_region_info(region, &smccc_ret->a1, &smccc_ret->a2);
	case EMIMPU_CHECK_NS_CPU:
		return emi_mpu_check_ns_cpu();
	case EMIMPU_PAGE_BASE_REGION:
		return emi_mpu_page_base_region();
	case EMIMPU_CHECK_HP_MOD:
		return emi_mpu_smc_hp_mod_check();
	default:
		return 0;
	}
}

static u_register_t sip_emidbg_control(u_register_t op_id,
				       u_register_t x2,
				       u_register_t x3,
				       u_register_t x4,
				       void *handle,
				       struct smccc_res *smccc_ret)
{
	enum mtk_bl31_status ret;

	switch (op_id) {
	case EMIDBG_DUMP:
		return MTK_SIP_E_SUCCESS;
	case EMIDBG_MSG:
		return MTK_SIP_E_SUCCESS;
#ifdef MTK_EMI_MPU_DEBUG
	case EMIMPU_READ:
		ret = emi_mpu_read_by_type((unsigned int)x2, (unsigned int)x3,
					   (unsigned int)x4, smccc_ret);
		break;
#endif
	case EMIMPU_CLEAR_MD:
		ret = emi_clear_md_violation();
		break;
	case KP_CLEAR:
		ret = emi_kp_clear_violation((unsigned int)x2);
		break;
#ifdef CONFIG_MTK_SLB_MPU_CLEAR
	case SLBMPU_CLEAR:
		ret = slb_clear_violation((unsigned int)x2);
		break;
#endif
#ifdef CONFIG_MTK_EMI_CLEAR
	case EMI_CLEAR:
		ret = emi_clear_violation((unsigned int)x2, (unsigned int)x3);
		break;
#endif
#ifdef CONFIG_MTK_SLC_PARITY
	case SLC_PARITY_SELECT:
		ret = slc_parity_select((unsigned int)x2, (unsigned int)x3);
		break;
	case SLC_PARITY_CLEAR:
		ret = slc_parity_clear((unsigned int)x2);
		break;
#endif
	default:
		return MTK_SIP_E_NOT_SUPPORTED;
	}

	return mtk_bl31_map_to_sip_error(ret);
}
DECLARE_SMC_HANDLER(MTK_SIP_EMIDBG_CONTROL, sip_emidbg_control);

static u_register_t sip_emimpu_control(u_register_t op_id,
				       u_register_t x2,
				       u_register_t x3,
				       u_register_t x4,
				       void *handle,
				       struct smccc_res *smccc_ret)
{
	enum mtk_bl31_status ret;

	switch (op_id) {
	case EMIMPU_SET:
		ret = emi_mpu_set_protection((uint32_t)x2, (uint32_t)x3, (unsigned int)x4);
		break;
	case AID_TABLE_SET:
		ret = emi_mpu_set_aid((unsigned int)x2, (unsigned int)x3);
		break;
	case EMIMPU_READ:
		ret = emi_mpu_read_by_type((unsigned int)x2, (unsigned int)x3,
					   (unsigned int)x4, smccc_ret);
		break;
	case KP_SET:
		ret = emi_kp_set_protection((size_t)x2, (size_t)x3, (unsigned int)x4);
		break;
	case KP_CLEAR:
		ret = emi_kp_clear_violation((unsigned int)x2);
		break;
	default:
		return MTK_SIP_E_NOT_SUPPORTED;
	}

	return mtk_bl31_map_to_sip_error(ret);
}
DECLARE_SMC_HANDLER(MTK_SIP_BL_EMIMPU_CONTROL, sip_emimpu_control);

static u_register_t sip_tee_emimpu_control(u_register_t op_id,
					   u_register_t x2,
					   u_register_t x3,
					   u_register_t x4,
					   void *handle,
					   struct smccc_res *smccc_ret)
{
	enum mtk_bl31_status ret;

	switch (op_id) {
	case EMIMPU_SET:
		ret = emi_mpu_set_protection((uint32_t)x2, (uint32_t)x3, (unsigned int)x4);
		break;
	case EMIMPU_CLEAR:
		ret = emi_clear_protection((unsigned int)x2);
		break;
	default:
		return MTK_SIP_E_NOT_SUPPORTED;
	}

	return mtk_bl31_map_to_sip_error(ret);
}
DECLARE_SMC_HANDLER(MTK_SIP_TEE_EMI_MPU_CONTROL, sip_tee_emimpu_control);

int emi_mpu_init(void)
{
	enum mtk_bl31_status ret;

	INFO("[%s] emi mpu initialization\n", __func__);

	for (size_t i = 0; i < ARRAY_SIZE(init_regions); i++) {
		uint64_t emi_start = init_regions[i].start >> EMI_MPU_ALIGN_BITS;
		uint64_t emi_end =
			(init_regions[i].start + init_regions[i].size) >> EMI_MPU_ALIGN_BITS;

		/* Avoid emi_end overflow */
		assert(emi_end >= emi_start);
		/*
		 * The emi address should be a 32-bit unsigned integer
		 * after being shifted with EMI_MPU_ALIGN_BITS.
		 */
		assert(!(emi_start >> 32) && !(emi_end >> 32));

		ret = emi_mpu_set_protection((uint32_t)emi_start,
					     (uint32_t)emi_end,
					     init_regions[i].region_id);
		if (ret != MTK_BL31_STATUS_SUCCESS) {
			ERROR("[%s] failed to set emi mpu region %u! ret: %d\n",
			      __func__, init_regions[i].region_id, ret);
			return -1;
		}
	}

	/* NSR enable */
	ret = emi_set_nsr_permission();
	if (ret != MTK_BL31_STATUS_SUCCESS) {
		ERROR("[%s] failed to set NSR permission! ret: %d\n",
		      __func__, ret);
		return -1;
	}

	return 0;
}
MTK_PLAT_SETUP_0_INIT(emi_mpu_init);
