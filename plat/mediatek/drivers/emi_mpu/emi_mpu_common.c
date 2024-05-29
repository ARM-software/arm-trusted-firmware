/*
 * Copyright (c) 2022-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <smccc_helpers.h>

#include <emi_mpu.h>
#include <lib/mtk_init/mtk_init.h>
#include <mtk_sip_svc.h>

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
	if (region_lock_state[region] == LOCK) {
		WARN("invalid region\n");
		return -1;
	}

	if ((dgroup == 0) && ((apc >> 31) & 0x1)) {
		region_lock_state[region] = LOCK;
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
	return 0;
}

int emi_mpu_clear_protection(unsigned int region)
{
	unsigned int dgroup;

	if (region >= EMI_MPU_REGION_NUM) {
		WARN("invalid region number\n");
		return -1;
	}

#if ENABLE_EMI_MPU_SW_LOCK
	if (region_lock_state[region] == LOCK) {
		WARN("SW:region is locked\n");
		return -1;
	}
#endif
	if (mmio_read_32(EMI_MPU_APC(region, 0)) & (LOCK << 31UL)) {
		WARN("HW:EMI-MPU region is locked\n");
		return -1;
	}

#if defined(SUB_EMI_MPU_BASE)
	if (mmio_read_32(SUB_EMI_MPU_APC(region, 0)) & (LOCK << 31UL)) {
		WARN("HW:SUB EMI-MPU region is locked\n");
		return -1;
	}
#endif

	for (dgroup = 0; dgroup < EMI_MPU_DGROUP_NUM; dgroup++)
		mmio_write_32(EMI_MPU_APC(region, dgroup), 0x0);

	mmio_write_32(EMI_MPU_SA(region), 0x0);
	mmio_write_32(EMI_MPU_EA(region), 0x0);

#if defined(SUB_EMI_MPU_BASE)
	for (dgroup = 0; dgroup < EMI_MPU_DGROUP_NUM; dgroup++)
		mmio_write_32(SUB_EMI_MPU_APC(region, dgroup), 0x0);

	mmio_write_32(SUB_EMI_MPU_SA(region), 0);
	mmio_write_32(SUB_EMI_MPU_EA(region), 0);
#endif
	return 0;
}


static void dump_emi_mpu_regions(void)
{
	int region, i;

	/* Only dump 8 regions(max: EMI_MPU_REGION_NUM --> 32) */
	for (region = 0; region < 8; ++region) {
		INFO("region %d:\n", region);
		INFO("\tsa: 0x%x, ea: 0x%x\n",
		     mmio_read_32(EMI_MPU_SA(region)), mmio_read_32(EMI_MPU_EA(region)));

		for (i = 0; i < EMI_MPU_DGROUP_NUM; ++i) {
			INFO("\tapc%d: 0x%x\n", i, mmio_read_32(EMI_MPU_APC(region, i)));
		}
	}
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
		end = (unsigned int)(region_info->end >> EMI_MPU_ALIGN_BITS) | (i << 24);

		if (_emi_mpu_set_protection(start, end, region_info->apc[i]) < 0) {
			WARN("Failed to set emi mpu protection(%d, %d, %d)\n",
			     start, end, region_info->apc[i]);
		}
	}

	return 0;
}

u_register_t mtk_emi_mpu_sip_handler(u_register_t x1, u_register_t x2,
				     u_register_t x3, u_register_t x4,
				     void *handle, struct smccc_res *smccc_ret)
{
	return (u_register_t) emi_mpu_optee_handler(x1, x2, x3);
}
DECLARE_SMC_HANDLER(MTK_SIP_TEE_MPU_PERM_SET, mtk_emi_mpu_sip_handler);

int emi_mpu_init(void)
{
	INFO("[%s] emi mpu initialization\n", __func__);

	set_emi_mpu_regions();
	dump_emi_mpu_regions();

	return 0;
}
MTK_PLAT_SETUP_0_INIT(emi_mpu_init);
