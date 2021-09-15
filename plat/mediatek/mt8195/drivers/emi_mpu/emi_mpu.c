/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <emi_mpu.h>

#if ENABLE_EMI_MPU_SW_LOCK
static unsigned char region_lock_state[EMI_MPU_REGION_NUM];
#endif

#define EMI_MPU_START_MASK		(0x00FFFFFF)
#define EMI_MPU_END_MASK		(0x00FFFFFF)
#define EMI_MPU_APC_SW_LOCK_MASK	(0x00FFFFFF)
#define EMI_MPU_APC_HW_LOCK_MASK	(0x80FFFFFF)

static int _emi_mpu_set_protection(unsigned int start, unsigned int end,
					unsigned int apc)
{
	unsigned int dgroup;
	unsigned int region;

	region = (start >> 24) & 0xFF;
	start &= EMI_MPU_START_MASK;
	dgroup = (end >> 24) & 0xFF;
	end &= EMI_MPU_END_MASK;

	if  ((region >= EMI_MPU_REGION_NUM) || (dgroup > EMI_MPU_DGROUP_NUM)) {
		WARN("invalid region, domain\n");
		return -1;
	}

#if ENABLE_EMI_MPU_SW_LOCK
	if (region_lock_state[region] == 1) {
		WARN("invalid region\n");
		return -1;
	}

	if ((dgroup == 0) && ((apc >> 31) & 0x1)) {
		region_lock_state[region] = 1;
	}

	apc &= EMI_MPU_APC_SW_LOCK_MASK;
#else
	apc &= EMI_MPU_APC_HW_LOCK_MASK;
#endif

	if ((start >= DRAM_OFFSET) && (end >= start)) {
		start -= DRAM_OFFSET;
		end -= DRAM_OFFSET;
	} else {
		WARN("invalid range\n");
		return -1;
	}

	mmio_write_32(EMI_MPU_SA(region), start);
	mmio_write_32(EMI_MPU_EA(region), end);
	mmio_write_32(EMI_MPU_APC(region, dgroup), apc);

#if defined(SUB_EMI_MPU_BASE)
	mmio_write_32(SUB_EMI_MPU_SA(region), start);
	mmio_write_32(SUB_EMI_MPU_EA(region), end);
	mmio_write_32(SUB_EMI_MPU_APC(region, dgroup), apc);
#endif
	return 1;
}

int emi_mpu_set_protection(struct emi_region_info_t *region_info)
{
	unsigned int start, end;
	int i;

	if (region_info->region >= EMI_MPU_REGION_NUM) {
		WARN("invalid region\n");
		return -1;
	}

	start = (unsigned int)(region_info->start >> EMI_MPU_ALIGN_BITS) |
		(region_info->region << 24);

	for (i = EMI_MPU_DGROUP_NUM - 1; i >= 0; i--) {
		end = (unsigned int)(region_info->end >> EMI_MPU_ALIGN_BITS) |
			(i << 24);
		_emi_mpu_set_protection(start, end, region_info->apc[i]);
	}

	return 0;
}

void emi_mpu_init(void)
{
	/* TODO: more setting for EMI MPU. */
}
