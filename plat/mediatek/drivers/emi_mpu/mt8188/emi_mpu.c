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

	/* BL31 address */
	region_info.start = TZRAM_BASE;
	region_info.end = TZRAM_BASE + TZRAM_SIZE - 1;
	region_info.region = BL31_EMI_REGION_ID;
	SET_ACCESS_PERMISSION(region_info.apc, LOCK,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, SEC_RW);
	emi_mpu_set_protection(&region_info);

	/* BL32 address */
	region_info.start = BL32_REGION_BASE;
	region_info.end = BL32_REGION_BASE + BL32_REGION_SIZE - 1;
	region_info.region = BL32_REGION_ID;
	SET_ACCESS_PERMISSION(region_info.apc, LOCK,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, SEC_RW, SEC_RW);
	emi_mpu_set_protection(&region_info);

	/* SCP core0 DRAM */
	region_info.start = SCP_CORE0_REGION_BASE;
	region_info.end = SCP_CORE0_REGION_BASE + SCP_CORE0_REGION_SIZE - 1;
	region_info.region = SCP_CORE0_REGION_ID;
	SET_ACCESS_PERMISSION(region_info.apc, LOCK,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);

	/* SCP core1 DRAM */
	region_info.start = SCP_CORE1_REGION_BASE;
	region_info.end = SCP_CORE1_REGION_BASE + SCP_CORE1_REGION_SIZE - 1;
	region_info.region = SCP_CORE1_REGION_ID;
	SET_ACCESS_PERMISSION(region_info.apc, LOCK,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);

	/* DSP protect address */
	region_info.start = DSP_PROTECT_REGION_BASE;
	region_info.end = DSP_PROTECT_REGION_BASE + DSP_PROTECT_REGION_SIZE - 1;
	region_info.region = DSP_PROTECT_REGION_ID;
	SET_ACCESS_PERMISSION(region_info.apc, LOCK,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);

	/* All default settings */
	region_info.start = DRAM_START_ADDR;
	region_info.end = DRAM_START_ADDR + DRAM_MAX_SIZE - 1;
	region_info.region = ALL_DEFAULT_REGION_ID;
	SET_ACCESS_PERMISSION(region_info.apc, LOCK,
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
	region_info.region = APUSYS_SEC_BUF_EMI_REGION_ID;

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

static inline uint32_t get_decoded_set_clear_info(uint32_t info)
{
	return (info & 0x0000FFFF);
}

int emi_mpu_optee_handler(uint64_t encoded_addr, uint64_t zone_size,
						  uint64_t zone_info)
{
	uint64_t phys_addr = get_decoded_phys_addr(encoded_addr);
	struct emi_region_info_t region_info;
	enum MPU_REQ_ORIGIN_ZONE_ID zone_id = get_decoded_zone_id(zone_info);
	uint32_t is_set = get_decoded_set_clear_info(zone_info);

	INFO("encoded_addr = 0x%lx, zone_size = 0x%lx, zone_info = 0x%lx\n",
	     encoded_addr, zone_size, zone_info);

	if (zone_id != MPU_REQ_ORIGIN_TEE_ZONE_SVP) {
		ERROR("Invalid param %s, %d\n", __func__, __LINE__);
		return MTK_SIP_E_INVALID_PARAM;
	}

	if (is_set > 0) {
		/* SVP DRAM */
		region_info.start = phys_addr;
		region_info.end = phys_addr + zone_size - 1;
		region_info.region = SVP_DRAM_REGION_ID;
		SET_ACCESS_PERMISSION(region_info.apc, UNLOCK,
					  FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
					  FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
					  FORBIDDEN, SEC_RW, FORBIDDEN, FORBIDDEN,
					  FORBIDDEN, FORBIDDEN, SEC_RW, SEC_RW);

		emi_mpu_set_protection(&region_info);
	} else { /* clear region protection */
		emi_mpu_clear_protection(SVP_DRAM_REGION_ID);
	}

	return 0;
}
