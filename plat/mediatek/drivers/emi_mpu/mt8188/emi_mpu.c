/*
 * Copyright (c) 2022-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <emi_mpu.h>
#include <mtk_sip_svc.h>

#define MPU_PHYSICAL_ADDR_SHIFT_BITS	(16)

void set_emi_mpu_regions(void)
{
	struct emi_region_info_t region_info;

	/* SCP core0 DRAM */
	region_info.start = 0x50000000ULL;
	region_info.end = 0x528FFFFFULL;
	region_info.region = 2;
	SET_ACCESS_PERMISSION(region_info.apc, 1,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);

	/* SCP core1 DRAM */
	region_info.start = 0x70000000ULL;
	region_info.end = 0x729FFFFFULL;
	region_info.region = 3;
	SET_ACCESS_PERMISSION(region_info.apc, 1,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);

	/* DSP protect address */
	region_info.start = 0x60000000ULL;
	region_info.end = 0x610FFFFFULL;
	region_info.region = 4;
	SET_ACCESS_PERMISSION(region_info.apc, 1,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);

	/* All default settings */
	region_info.start = 0x40000000ULL;
	region_info.end = 0x1FFFF0000ULL;
	region_info.region = 31;
	SET_ACCESS_PERMISSION(region_info.apc, 1,
			      FORBIDDEN, FORBIDDEN, NO_PROTECTION, NO_PROTECTION,
			      NO_PROTECTION, FORBIDDEN, NO_PROTECTION, NO_PROTECTION,
			      NO_PROTECTION, SEC_R_NSEC_RW, NO_PROTECTION, FORBIDDEN,
			      NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);
}

int set_apu_emi_mpu_region(void)
{
	struct emi_region_info_t region_info;

	region_info.start = (unsigned long long)APUSYS_SEC_BUF_PA;
	region_info.end = (unsigned long long)(APUSYS_SEC_BUF_PA + APUSYS_SEC_BUF_SZ) - 1;
	region_info.region = APUSYS_SEC_BUF_EMI_REGION;

	SET_ACCESS_PERMISSION(region_info.apc, UNLOCK,
			      FORBIDDEN,     FORBIDDEN, FORBIDDEN,     FORBIDDEN,
			      FORBIDDEN,     FORBIDDEN, FORBIDDEN,     FORBIDDEN,
			      NO_PROTECTION, FORBIDDEN, NO_PROTECTION, FORBIDDEN,
			      FORBIDDEN,     FORBIDDEN, FORBIDDEN,     SEC_RW);

	return emi_mpu_set_protection(&region_info);
}

static inline uint64_t get_decoded_phys_addr(uint64_t addr)
{
	return (addr << MPU_PHYSICAL_ADDR_SHIFT_BITS);
}

static inline uint32_t get_decoded_zone_id(uint32_t info)
{
	return ((info & 0xFFFF0000) >> MPU_PHYSICAL_ADDR_SHIFT_BITS);
}

int emi_mpu_optee_handler(uint64_t encoded_addr, uint64_t zone_size,
						  uint64_t zone_info)
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