/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <mtk_bl31_interface.h>
#include <mtk_sip_svc.h>

int mtk_bl31_map_to_sip_error(enum mtk_bl31_status status)
{
	switch (status) {
	case MTK_BL31_STATUS_SUCCESS:
		return MTK_SIP_E_SUCCESS;
	case MTK_BL31_STATUS_INVALID_PARAM:
		return MTK_SIP_E_INVALID_PARAM;
	case MTK_BL31_STATUS_NOT_SUPPORTED:
		return MTK_SIP_E_NOT_SUPPORTED;
	case MTK_BL31_STATUS_INVALID_RANGE:
		return MTK_SIP_E_INVALID_RANGE;
	case MTK_BL31_STATUS_PERMISSION_DENY:
		return MTK_SIP_E_PERMISSION_DENY;
	case MTK_BL31_STATUS_LOCK_FAIL:
		return MTK_SIP_E_LOCK_FAIL;
	default:
		ERROR("%s: unknown status: %d\n", __func__, status);
	}

	return MTK_SIP_E_NOT_SUPPORTED;
}

int mtk_bl31_mmap_add_dynamic_region(unsigned long long base_pa, size_t size,
				     enum mtk_bl31_memory_type type)
{
	unsigned int attr;

	switch (type) {
	case MTK_BL31_DEV_RW_SEC:
		attr = MT_DEVICE | MT_RW | MT_SECURE;
		break;
	default:
		attr = 0;
		ERROR("%s: unknown memory type %d\n", __func__, type);
		break;
	}

	return mmap_add_dynamic_region(base_pa, base_pa, size, attr);
}

int mtk_bl31_mmap_remove_dynamic_region(uintptr_t base_va, size_t size)
{
	return mmap_remove_dynamic_region(base_va, size);
}
