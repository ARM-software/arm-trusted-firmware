/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lpm_v2/mt_lpm_smc.h>
#include <mt_spm_common.h>
#include <mt_spm_hwreq.h>
#include <mt_spm_rc_api_common.h>

#define SPM_RC_VALID_SET(dest, src) ({ (dest) |= (src); })
#define SPM_RC_VALID_CLR(dest, src) ({ (dest) &= ~(src); })

int spm_rc_constraint_status_get(uint32_t id, uint32_t type,
				 uint32_t act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) &&
	    (id != dest_rc_id)) || !dest || !src)
		return -1;
	spm_lock_get();

	switch (type) {
	case CONSTRAINT_GET_ENTER_CNT:
		if (id == MT_RM_CONSTRAINT_ID_ALL)
			dest->enter_cnt += src->enter_cnt;
		else
			dest->enter_cnt = src->enter_cnt;
		break;
	case CONSTRAINT_GET_VALID:
		dest->is_valid = src->is_valid;
		break;
	case CONSTRAINT_COND_BLOCK:
		dest->is_cond_block = src->is_cond_block;
		dest->all_pll_dump = src->all_pll_dump;
		break;
	case CONSTRAINT_GET_COND_BLOCK_DETAIL:
		dest->cond_res = src->cond_res;
		break;
	case CONSTRAINT_GET_RESIDNECY:
		dest->residency = src->residency;
		if (act & MT_LPM_SMC_ACT_CLR)
			src->residency = 0;
		break;
	default:
		break;
	}

	spm_lock_release();
	return 0;
}

int spm_rc_constraint_status_set(uint32_t id, uint32_t type,
				 uint32_t act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) &&
	    (id != dest_rc_id)) || !dest)
		return -1;

	spm_lock_get();

	switch (type) {
	case CONSTRAINT_UPDATE_VALID:
		if (src) {
			if (act & MT_LPM_SMC_ACT_SET)
				SPM_RC_VALID_SET(dest->is_valid, src->is_valid);
			else if (act & MT_LPM_SMC_ACT_CLR)
				SPM_RC_VALID_CLR(dest->is_valid, src->is_valid);
		}
		break;
	case CONSTRAINT_RESIDNECY:
		if (act & MT_LPM_SMC_ACT_CLR)
			dest->residency = 0;
		break;
	default:
		break;
	}

	spm_lock_release();

	return 0;
}

int spm_rc_constraint_valid_set(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				uint32_t valid,
				struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) &&
	    (id != dest_rc_id)) || !dest)
		return -1;

	spm_lock_get();
	SPM_RC_VALID_SET(dest->is_valid, valid);
	spm_lock_release();

	return 0;
}

int spm_rc_constraint_valid_clr(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				uint32_t valid,
				struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) &&
	    (id != dest_rc_id)) || !dest)
		return -1;

	spm_lock_get();
	SPM_RC_VALID_CLR(dest->is_valid, valid);
	spm_lock_release();

	return 0;
}

#define PMIC_WRAP_REG_1		0x3E8
#define PMIC_WRAP_REG_STEP	0x4
#define PMIC_WRAP_REG_2		0xC28
#define PMIC_WRAP_REG_3		0xF54

#ifndef MTK_PLAT_SPM_PMIC_WRAP_DUMP_UNSUPPORT
void mt_spm_dump_pmic_warp_reg(void)
{
	uint32_t temp;
	uint32_t i;

	for (i = 0; i <= PMIC_WRAP_REG_1; i += PMIC_WRAP_REG_STEP) {
		temp = mmio_read_32(PMIC_WRAP_BASE + i);
	}

	for (i = 0xC00; i <= PMIC_WRAP_REG_2; i += PMIC_WRAP_REG_STEP) {
		temp = mmio_read_32(PMIC_WRAP_BASE + i);
	}

	for (i = 0xF00; i <= PMIC_WRAP_REG_3; i += PMIC_WRAP_REG_STEP) {
		temp = mmio_read_32(PMIC_WRAP_BASE + i);
	}
}
#endif

uint32_t spm_allow_rc_vcore(int state_id)
{
	return CONSTRAINT_VCORE_ALLOW;
}

int spm_hwcg_name(uint32_t idex, char *name, size_t sz)
{
	int ret = 0;

	if (!name)
		return -1;

	switch (idex) {
	case HWCG_DDREN:
		ret = snprintf(name, sz - 1, "dram");
		break;
	case HWCG_VRF18:
		ret = snprintf(name, sz - 1, "vrf18");
		break;
	case HWCG_INFRA:
		ret = snprintf(name, sz - 1, "infra");
		break;
	case HWCG_PMIC:
		ret = snprintf(name, sz - 1, "pmic");
		break;
	case HWCG_F26M:
		ret = snprintf(name, sz - 1, "26m");
		break;
	case HWCG_VCORE:
		ret = snprintf(name, sz - 1, "vcore");
		break;
	default:
		ret = -1;
		break;
	}

	if (ret < 0)
		ret = -1;

	name[sz - 1] = '\0';

	return ret;
}
