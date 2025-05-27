/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mtk_bl31_interface.h>

uint64_t emi_mpu_read_addr(unsigned int region, unsigned int offset)
{
	return 0;
}

uint64_t emi_mpu_read_enable(unsigned int region)
{
	return 0;
}

uint64_t emi_mpu_read_aid(unsigned int region, unsigned int aid_shift)
{
	return 0;
}

uint64_t emi_mpu_check_ns_cpu(void)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

enum mtk_bl31_status emi_kp_set_protection(size_t start, size_t end, unsigned int region)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

enum mtk_bl31_status emi_kp_clear_violation(unsigned int emiid)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

enum mtk_bl31_status emi_clear_protection(unsigned int region)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

enum mtk_bl31_status emi_clear_md_violation(void)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

uint64_t emi_mpu_check_region_info(unsigned int region, uint64_t *sa, uint64_t *ea)
{
	return 0;
}

uint64_t emi_mpu_page_base_region(void)
{
	return 0;
}

uint64_t emi_mpu_smc_hp_mod_check(void)
{
	return 0;
}

enum mtk_bl31_status slb_clear_violation(unsigned int id)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

enum mtk_bl31_status emi_clear_violation(unsigned int id, unsigned int type)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

enum mtk_bl31_status slc_parity_select(unsigned int id, unsigned int port)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

enum mtk_bl31_status slc_parity_clear(unsigned int id)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

enum mtk_bl31_status emi_mpu_set_aid(unsigned int region, unsigned int num)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}

void emi_protection_init(void)
{
}

enum mtk_bl31_status emi_mpu_set_protection(uint32_t start, uint32_t end,
					    unsigned int region)
{
	return MTK_BL31_STATUS_NOT_SUPPORTED;
}
