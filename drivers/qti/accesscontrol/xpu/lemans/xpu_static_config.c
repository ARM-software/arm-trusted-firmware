/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>
#include <xpu3.h>
#include <xpu_target_info.h>

#include <platform_def.h>

/* RG 20-24 are not added as qc_sec assigns them to the modem */
struct rg_domain_ownership llcc_mpu_rgs[] = {
	{ 0, APPS_S_DOMAIN },
};

struct rg_partition_range llcc_mpu_rg_addr[] = {
	{ 0, BL31_BASE & 0xffffffffUL,
	  (BL31_BASE + BL31_SIZE) & 0xffffffffUL },
};

struct rg_partition_range gemnoc_cnoc_mpu_rg_addr[] = {
	{ 6, 0x0c200000, 0x0c300000 },
	{ 19, 0x09a36000, 0x09a37000 },
	{ 24, 0x0c263000, 0x0c264000 },
	{ 25, 0x0c265000, 0x0c266000 },
	{ 26, 0x0c251000, 0x0c253000 },
	{ 27, 0x01790000, 0x01792000 },
	{ 28, 0x09a30000, 0x09a32000 },
	{ 29, 0x016b0000, 0x016b2000 },
	{ 30, 0x0acac000, 0x0acad000 },
};

struct rg_domain_ownership gemnoc_cnoc_mpu_rgs[] = {
	{ 6, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 19, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 24, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 25, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 26, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 27, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 28, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 29, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 30, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

/*
 * For RGs 24, 25 and 29, start and end addresses are subtracted with the base
 * address 0x0b000000 as per policy:
 *   RG 24, start = 0xc200000, end = 0xc210000
 *   RG 25, start = 0xc310000, end = 0xc3f1000
 *   RG 29, start = 0xc400000, end = 0xec00000
 */
struct rg_partition_range aoss_cnoc_mpu_rg_addr[] = {
	{ 10, 0x0c266000, 0x0c270000 },
	{ 13, 0x0c2f7000, 0x0c2f8000 },
	{ 16, 0xffffffff, 0xffffffff },
	{ 18, 0x0c272000, 0x0c273000 },
	{ 21, 0x0c263000, 0x0c264000 },
	{ 24, 0x0c200000, 0x0c210000 },
};

struct rg_domain_ownership aoss_cnoc_mpu_rgs[] = {
	{ 10, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 13, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 16, APPS_S_DOMAIN },
	{ 18, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 21, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 24, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct xpu_instance msm_xpu_cfg[] = {
	{ 0x09230000, ARRAY_SIZE(llcc_mpu_rgs), llcc_mpu_rgs,
	  ARRAY_SIZE(llcc_mpu_rg_addr), llcc_mpu_rg_addr,
	  XPU_TYPE_LLCC_BROADCAST_MPU, XPU_PROTECTION_STATIC },
	{ 0x0933e000, ARRAY_SIZE(llcc_mpu_rgs), llcc_mpu_rgs,
	  ARRAY_SIZE(llcc_mpu_rg_addr), llcc_mpu_rg_addr,
	  XPU_TYPE_LLCC_BROADCAST_MPU, XPU_PROTECTION_STATIC },
	{ 0x01520000, ARRAY_SIZE(gemnoc_cnoc_mpu_rgs), gemnoc_cnoc_mpu_rgs,
	  ARRAY_SIZE(gemnoc_cnoc_mpu_rg_addr), gemnoc_cnoc_mpu_rg_addr,
	  XPU_TYPE_CNOC_GEMNOC_MPU, XPU_PROTECTION_STATIC },
	{ 0x01524000, 0, NULL, 0, NULL,
	  /* CNOC2_SS unclocked at EL3 boot: GCR0 access stalls, so skip */
	  XPU_TYPE_CNOC2_SS_MPU, 0 },
	{ 0x0eff0000, ARRAY_SIZE(aoss_cnoc_mpu_rgs), aoss_cnoc_mpu_rgs,
	  ARRAY_SIZE(aoss_cnoc_mpu_rg_addr), aoss_cnoc_mpu_rg_addr,
	  XPU_TYPE_AOSS_MPU, XPU_PROTECTION_STATIC },
};

const uint32_t msm_xpu_cfg_count = ARRAY_SIZE(msm_xpu_cfg);

/* QCS9075 does not have a modem subsystem - no modem MPU ranges */
struct mpu_ranges msm_mpu_ranges[] = {
};

const uint32_t msm_mpu_ranges_count = ARRAY_SIZE(msm_mpu_ranges);
