/*
 * Copyright (c) 2021-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <emi_mpu.h>
#include <mtk_sip_svc.h>

#if ENABLE_EMI_MPU_SW_LOCK
static unsigned char region_lock_state[EMI_MPU_REGION_NUM];
#endif

#define EMI_MPU_START_MASK		(0x00FFFFFF)
#define EMI_MPU_END_MASK		(0x00FFFFFF)
#define EMI_MPU_APC_SW_LOCK_MASK	(0x00FFFFFF)
#define EMI_MPU_APC_HW_LOCK_MASK	(0x80FFFFFF)
#define MPU_PHYSICAL_ADDR_SHIFT_BITS	(16)

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

void dump_emi_mpu_regions(void)
{
	unsigned long apc[EMI_MPU_DGROUP_NUM], sa, ea;

	int region, i;

	/* Only dump 8 regions(max: EMI_MPU_REGION_NUM --> 32) */
	for (region = 0; region < 8; ++region) {
		for (i = 0; i < EMI_MPU_DGROUP_NUM; ++i)
			apc[i] = mmio_read_32(EMI_MPU_APC(region, i));
		sa = mmio_read_32(EMI_MPU_SA(region));
		ea = mmio_read_32(EMI_MPU_EA(region));

		INFO("region %d:\n", region);
		INFO("\tsa:0x%lx, ea:0x%lx, apc0: 0x%lx apc1: 0x%lx\n",
		     sa, ea, apc[0], apc[1]);
	}
}

void emi_mpu_init(void)
{
	struct emi_region_info_t region_info;

	/* SCP DRAM */
	region_info.start = 0x50000000ULL;
	region_info.end = 0x513FFFFFULL;
	region_info.region = 2;
	SET_ACCESS_PERMISSION(region_info.apc, 1,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      NO_PROTECTION, FORBIDDEN, FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);

	/* DSP protect address */
	region_info.start = 0x60000000ULL;	/* dram base addr */
	region_info.end = 0x610FFFFFULL;
	region_info.region = 3;
	SET_ACCESS_PERMISSION(region_info.apc, 1,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);

	/* Forbidden All */
	region_info.start = 0x40000000ULL;	/* dram base addr */
	region_info.end = 0x1FFFF0000ULL;
	region_info.region = 5;
	SET_ACCESS_PERMISSION(region_info.apc, 1,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);

	dump_emi_mpu_regions();
}

static inline uint64_t get_decoded_phys_addr(uint64_t addr)
{
	return (addr << MPU_PHYSICAL_ADDR_SHIFT_BITS);
}

static inline uint32_t get_decoded_zone_id(uint32_t info)
{
	return ((info & 0xFFFF0000) >> MPU_PHYSICAL_ADDR_SHIFT_BITS);
}

int32_t emi_mpu_sip_handler(uint64_t encoded_addr, uint64_t zone_size, uint64_t zone_info)
{
	uint64_t phys_addr = get_decoded_phys_addr(encoded_addr);
	struct emi_region_info_t region_info;
	enum MPU_REQ_ORIGIN_ZONE_ID zone_id = get_decoded_zone_id(zone_info);

	INFO("encoded_addr = 0x%lx, zone_size = 0x%lx, zone_info = 0x%lx\n",
	     encoded_addr, zone_size, zone_info);

	if (zone_id != MPU_REQ_ORIGIN_TEE_ZONE_SVP) {
		ERROR("Invalid param %s, %d\n", __func__, __LINE__);
		return MTK_SIP_E_INVALID_PARAM;
	}

	/* SVP DRAM */
	region_info.start = phys_addr;
	region_info.end = phys_addr + zone_size;
	region_info.region = 4;
	SET_ACCESS_PERMISSION(region_info.apc, 1,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, SEC_RW);

	emi_mpu_set_protection(&region_info);

	return 0;
}
