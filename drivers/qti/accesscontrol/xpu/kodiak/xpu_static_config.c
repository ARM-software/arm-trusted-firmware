/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>
#include <xpu3.h>
#include <xpu_target_info.h>

#include <platform_def.h>

#define ANOC2_STATIC_REGION 3
#define MSS_Q6_STATIC_REGION 17

struct rg_domain_ownership ramblur_pimem_mpu_rgs[] = {
	{ 7, APPS_S_DOMAIN },
};

struct rg_partition_range ramblur_pimem_mpu_rg_addr[] = {
	{ 7, 0x0, 0x0 }, /* full p_imem range */
};

struct rg_domain_ownership mss_q6_mpu_rgs[MSS_Q6_STATIC_REGION + 1 +
					  NUM_MODEM_MPU_PARTITIONS] = {
	{ 0, MSA_DOMAIN },  { 1, MSA_DOMAIN },	{ 2, MSA_DOMAIN },
	{ 3, MSA_DOMAIN },  { 4, MSA_DOMAIN },	{ 5, MSA_DOMAIN },
	{ 6, MSA_DOMAIN },  { 7, MSA_DOMAIN },	{ 8, MSA_DOMAIN },
	{ 9, MSA_DOMAIN },  { 10, MSA_DOMAIN }, { 11, MSA_DOMAIN },
	{ 12, MSA_DOMAIN }, { 13, MSA_DOMAIN }, { 14, MSA_DOMAIN },
	{ 15, MSA_DOMAIN }, { 16, MSA_DOMAIN }, { XPU_UMR_RG, APPS_NS_DOMAIN },
};

struct rg_partition_range
	mss_q6_mpu_rg_addr[MSS_Q6_STATIC_REGION + NUM_MODEM_MPU_PARTITIONS] = {
		{ 0, 0x00008000, 0x007c0000 },	{ 1, 0x007e0000, 0x00940000 },
		{ 2, 0x00984000, 0x01c00000 },	{ 3, 0x01dc0000, 0x02000000 },
		{ 4, 0x04000000, 0x06002000 },	{ 5, 0x06003000, 0x08400000 },
		{ 6, 0x09000000, 0x09800000 },	{ 7, 0x0b000000, 0x0f00a000 },
		{ 8, 0x0f16d000, 0x0f177000 },	{ 9, 0x0f178000, 0x0f181000 },
		{ 10, 0x0f185000, 0x0f18b000 }, { 11, 0x16000000, 0x17000000 },
		{ 12, 0x146a8000, 0x146ab000 }, { 13, 0x80900000, 0x80b00000 },
		{ 14, 0x80860000, 0x80880000 }, { 15, 0x146a5000, 0x146a6000 },
		{ 16, 0xffffffff, 0xffffffff },
	};

struct rg_domain_ownership
	anoc2_mpu_rgs[ANOC2_STATIC_REGION + 1 + NUM_MODEM_MPU_PARTITIONS] = {
		{ 0, MSA_DOMAIN },
		{ 6, MSA_DOMAIN },
		{ 7, MSA_DOMAIN },
		{ XPU_UMR_RG, APPS_NS_DOMAIN },
	};

struct rg_partition_range
	anoc2_mpu_rg_addr[ANOC2_STATIC_REGION + NUM_MODEM_MPU_PARTITIONS] = {
		{ 0, 0x01e00000, 0x01f00000 },
		{ 6, 0x80900000, 0x80b00000 },
		{ 7, 0x80860000, 0x80880000 },
	};

struct rg_domain_ownership gemnoc_cnoc_mpu_rgs[] = {
	{ 0, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 1, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 2, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 3, MSA_DOMAIN },
	{ 4, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 5, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 6, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 7, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 8, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 9, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 10, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 11, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 12, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 13, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 14, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 15, MSA_DOMAIN },
	{ 16, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 17, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 18, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 19, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 23, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN, APPS_NS_DOMAIN },
	{ 24, APPS_S_DOMAIN, MSA_DOMAIN, MSA_DOMAIN },
	{ 25, APPS_S_DOMAIN, MSA_DOMAIN, MSA_DOMAIN },
	{ 26, APPS_S_DOMAIN },
	{ 27, APPS_S_DOMAIN, 0, APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 28, APPS_S_DOMAIN, 0, APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 29, APPS_S_DOMAIN },
	{ 30, APPS_S_DOMAIN },
	{ 31, APPS_S_DOMAIN },
	{ 32, APPS_S_DOMAIN },
	{ 33, APPS_S_DOMAIN },
	{ 34, APPS_S_DOMAIN },
	{ 35, APPS_S_DOMAIN },
	{ 36, APPS_S_DOMAIN },
	{ 37, APPS_S_DOMAIN },
	{ 38, APPS_S_DOMAIN },
	{ 39, APPS_S_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct rg_partition_range gemnoc_cnoc_mpu_rg_addr[] = {
	{ 0, 0x00080000U, 0x05000000U },  { 1, 0x0c310000U, 0x0c3f1000U },
	{ 2, 0x00800000U, 0x00b00000U },  { 3, 0x010d2000U, 0x010d3000U },
	{ 4, 0x00780000U, 0x007a0000U },  { 5, 0x09000000U, 0x09800000U },
	{ 6, 0x0c200000U, 0x0c290000U },  { 7, 0x01520000U, 0x01528000U },
	{ 8, 0x01680000U, 0x016b2000U },  { 9, 0x016e0000U, 0x01740000U },
	{ 10, 0x01c00000U, 0x01c20000U }, { 11, 0x01e00000U, 0x02000000U },
	{ 12, 0x0eff03fcU, 0x0f400000U }, { 13, 0x06000000U, 0x08000000U },
	{ 14, 0x0c400000U, 0x0ec00000U }, { 15, 0x010c3000U, 0x010c4000U },
	{ 16, 0x0b2c0000U, 0x0b2e0000U }, { 17, 0x0b4c0000U, 0x0b4d0000U },
	{ 18, 0x0bbf0000U, 0x0bbf2000U }, { 19, 0x0c2a0000U, 0x0c300000U },
	{ 23, 0x0b600000U, 0x0b7f0000U }, { 24, 0x010ca000U, 0x010cc000U },
	{ 25, 0x01530000U, 0x01533000U }, { 26, 0xffffffffU, 0xffffffffU },
	{ 27, 0x00634000U, 0x00635000U }, { 28, 0x00636000U, 0x00637000U },
	{ 29, 0xffffffffU, 0xffffffffU }, { 30, 0xffffffffU, 0xffffffffU },
	{ 31, 0xffffffffU, 0xffffffffU }, { 32, 0xffffffffU, 0xffffffffU },
	{ 33, 0xffffffffU, 0xffffffffU }, { 34, 0xffffffffU, 0xffffffffU },
	{ 35, 0xffffffffU, 0xffffffffU }, { 36, 0xffffffffU, 0xffffffffU },
	{ 37, 0xffffffffU, 0xffffffffU }, { 38, 0xffffffffU, 0xffffffffU },
	{ 39, 0xffffffffU, 0xffffffffU },
};

struct rg_partition_range cnoc2_ss_mpu_rg_addr[] = {
	{ 0, 0x01680000U, 0x01686000U },  { 1, 0x01686000U, 0x01688000U },
	{ 2, 0xffffffffU, 0xffffffffU },  { 3, 0x0168b000U, 0x0168e000U },
	{ 4, 0xffffffffU, 0xffffffffU },  { 5, 0x0168e000U, 0x01696000U },
	{ 6, 0x01500000U, 0x01506000U },  { 7, 0x01510000U, 0x0151d000U },
	{ 8, 0x016e0000U, 0x016e3000U },  { 9, 0x016f2000U, 0x01706000U },
	{ 10, 0xffffffffU, 0xffffffffU }, { 11, 0xffffffffU, 0xffffffffU },
	{ 12, 0x0170b000U, 0x01710000U }, { 13, 0xffffffffU, 0xffffffffU },
	{ 15, 0x0a0cf000U, 0x0a0d0000U }, { 16, 0x01740000U, 0x0174d000U },
	{ 17, 0xffffffffU, 0xffffffffU }, { 19, 0x06858000U, 0x06859000U },
	{ 20, 0x00100000U, 0x00500000U }, { 21, 0xffffffffU, 0xffffffffU },
	{ 22, 0xffffffffU, 0xffffffffU }, { 23, 0xffffffffU, 0xffffffffU },
	{ 24, 0x01720000U, 0x01731000U }, { 25, 0xffffffffU, 0xffffffffU },
	{ 26, 0x01758000U, 0x01760000U }, { 27, 0x016e6000U, 0x016e7000U },
	{ 28, 0x088e2000U, 0x088e3000U }, { 29, 0xffffffffU, 0xffffffffU },
	{ 30, 0x000b0000U, 0x000c0000U }, { 31, 0x010d2000U, 0x010d3000U },
	{ 32, 0xffffffffU, 0xffffffffU }, { 33, 0xffffffffU, 0xffffffffU },
	{ 34, 0x01e00000U, 0x02000000U }, { 35, 0xffffffffU, 0xffffffffU },
	{ 36, 0xffffffffU, 0xffffffffU }, { 37, 0x0c310000U, 0x0f400000U },
	{ 38, 0x08b1c000U, 0x08b1f000U }, { 39, 0x016a0000U, 0x016b2000U },
	{ 40, 0x01522000U, 0x01528000U }, { 41, 0x0152c000U, 0x01533000U },
	{ 42, 0xffffffffU, 0xffffffffU }, { 43, 0xffffffffU, 0xffffffffU },
	{ 44, 0xffffffffU, 0xffffffffU }, { 45, 0xffffffffU, 0xffffffffU },
	{ 46, 0x00634000U, 0x00635000U }, { 47, 0x00636000U, 0x00637000U },
};

struct rg_domain_ownership cnoc2_ss_mpu_rgs[] = {
	{ 0, APPS_S_DOMAIN },
	{ 1, MSA_DOMAIN },
	{ 2, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  MSA_DOMAIN | APPS_NS_DOMAIN },
	{ 3, MSA_DOMAIN },
	{ 4, APPS_S_DOMAIN },
	{ 5, APPS_S_DOMAIN },
	{ 6, APPS_S_DOMAIN },
	{ 7, APPS_S_DOMAIN },
	{ 8, APPS_S_DOMAIN },
	{ 9, APPS_S_DOMAIN },
	{ 10, APPS_S_DOMAIN },
	{ 11, APPS_S_DOMAIN },
	{ 12, APPS_S_DOMAIN },
	{ 13, APPS_S_DOMAIN },
	{ 15, APPS_S_DOMAIN },
	{ 16, APPS_S_DOMAIN },
	{ 17, MSA_DOMAIN },
	{ 19, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 20, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 21, APPS_S_DOMAIN },
	{ 22, APPS_S_DOMAIN },
	{ 23, APPS_S_DOMAIN },
	{ 24, APPS_S_DOMAIN },
	{ 25, APPS_S_DOMAIN },
	{ 26, APPS_S_DOMAIN },
	{ 27, APPS_S_DOMAIN },
	{ 28, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_S_DOMAIN },
	{ 29, APPS_S_DOMAIN },
	{ 30, MSA_DOMAIN },
	{ 31, MSA_DOMAIN },
	{ 32, APPS_S_DOMAIN },
	{ 33, APPS_S_DOMAIN },
	{ 34, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 35, APPS_S_DOMAIN },
	{ 36, APPS_S_DOMAIN },
	{ 37, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 38, APPS_S_DOMAIN },
	{ 39, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 40, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 41, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 42, APPS_S_DOMAIN },
	{ 43, APPS_S_DOMAIN },
	{ 44, APPS_S_DOMAIN },
	{ 45, APPS_S_DOMAIN },
	{ 46, APPS_S_DOMAIN, 0, APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 47, APPS_S_DOMAIN, 0, APPS_NS_DOMAIN | MSA_DOMAIN },
};

static struct rg_domain_ownership wpss_mpu_rgs[] = {
	{ 0, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 1, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 2, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 3, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 4, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 5, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 6, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 7, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 8, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 9, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 10, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 11, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 12, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 13, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 14, APPS_S_DOMAIN, APPS_NS_DOMAIN },
	{ 15, APPS_S_DOMAIN },
	{ 16, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 17, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 18, APPS_S_DOMAIN, APPS_NS_DOMAIN },
	{ 19, APPS_S_DOMAIN, APPS_NS_DOMAIN },
	{ 20, APPS_S_DOMAIN },
	{ 21, APPS_S_DOMAIN },
	{ 22, APPS_S_DOMAIN },
	{ 23, APPS_S_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN },
};

static struct rg_partition_range wpss_mpu_addr[] = {
	{ 0, 0x80c00000U, 0x81800000U },  { 1, 0x60000000U, 0x64000000U },
	{ 2, 0x16000000U, 0x17000000U },  { 3, 0x14680000U, 0x146ab000U },
	{ 4, 0x0b2f0000U, 0x0f400000U },  { 5, 0x09200000U, 0x09658000U },
	{ 6, 0x091cc000U, 0x091e3000U },  { 7, 0x09080000U, 0x09081000U },
	{ 8, 0x00634000U, 0x00635000U },  { 9, 0x06000000U, 0x08000000U },
	{ 10, 0x01c00000U, 0x01ff0000U }, { 11, 0x00636000U, 0x00637000U },
	{ 12, 0x010dc000U, 0x010dd000U }, { 13, 0x00100000U, 0x004d9000U },
	{ 14, 0x004fc000U, 0x004fd000U }, { 15, 0xffffffffU, 0xffffffffU },
	{ 16, 0x09ae0000U, 0x09c70000U }, { 17, 0x80900000U, 0x80a0d000U },
	{ 18, 0x80aff000U, 0x80b00000U }, { 19, 0x80860000U, 0x80880000U },
	{ 20, 0xffffffffU, 0xffffffffU }, { 21, 0xffffffffU, 0xffffffffU },
	{ 22, 0xffffffffU, 0xffffffffU }, { 23, 0xffffffffU, 0xffffffffU },
};

struct rg_domain_ownership aoss_cnoc_mpu_rgs[] = {
	{ 24, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 25, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
	{ 26, NO_DOMAIN, NO_DOMAIN, NO_DOMAIN },
	{ 27, NO_DOMAIN, NO_DOMAIN, NO_DOMAIN },
	{ 29, APPS_S_DOMAIN, APPS_NS_DOMAIN | MSA_DOMAIN,
	  APPS_NS_DOMAIN | MSA_DOMAIN },
};

struct rg_partition_range aoss_cnoc_mpu_rg_addr[] = {
	{ 24, 0x0c200000U, 0x0c210000U }, { 25, 0x0c310000U, 0x0c3f1000U },
	{ 26, 0xffffffffU, 0xffffffffU }, { 27, 0xffffffffU, 0xffffffffU },
	{ 29, 0x0c400000U, 0x0ec00000U },
};

struct rg_domain_ownership mss_nav_mpu_rgs[NUM_MSS_NAV_MPU_PARTITIONS + 1] = {
	{ XPU_UMR_RG, APPS_NS_DOMAIN },
};

struct rg_partition_range mss_nav_mpu_rg_addr[NUM_MSS_NAV_MPU_PARTITIONS] = {};

struct rg_domain_ownership anoc1_mpu_rgs[NUM_MSS_NAV_MPU_PARTITIONS + 1] = {
	{ XPU_UMR_RG, APPS_NS_DOMAIN },
};

struct rg_partition_range anoc1_mpu_rg_addr[NUM_MSS_NAV_MPU_PARTITIONS] = {};

struct rg_domain_ownership modem_ms_mpu_rgs[NUM_MSS_NAV_MPU_PARTITIONS + 1] = {
	{ XPU_UMR_RG, APPS_NS_DOMAIN },
};

struct rg_partition_range modem_ms_mpu_rg_addr[NUM_MSS_NAV_MPU_PARTITIONS] = {};

struct xpu_instance msm_xpu_cfg[] = {
	{
		0x00616000,
		ARRAY_SIZE(ramblur_pimem_mpu_rgs),
		ramblur_pimem_mpu_rgs,
		ARRAY_SIZE(ramblur_pimem_mpu_rg_addr),
		ramblur_pimem_mpu_rg_addr,
		XPU_TYPE_IMEM_MPU,
		XPU_PROTECTION_STATIC,
	},
	{
		0x016a8000,
		ANOC2_STATIC_REGION + 1,
		anoc2_mpu_rgs,
		ANOC2_STATIC_REGION,
		anoc2_mpu_rg_addr,
		XPU_TYPE_ANOC2_MPU,
		XPU_PROTECTION_STATIC,
	},
	{
		0x091fc000,
		MSS_Q6_STATIC_REGION + 1,
		mss_q6_mpu_rgs,
		MSS_Q6_STATIC_REGION,
		mss_q6_mpu_rg_addr,
		XPU_TYPE_MSS_Q6_MPU,
		XPU_PROTECTION_STATIC,
	},
	{
		0x0152c000,
		ARRAY_SIZE(gemnoc_cnoc_mpu_rgs),
		gemnoc_cnoc_mpu_rgs,
		ARRAY_SIZE(gemnoc_cnoc_mpu_rg_addr),
		gemnoc_cnoc_mpu_rg_addr,
		XPU_TYPE_CNOC_GEMNOC_MPU,
		XPU_PROTECTION_STATIC,
	},
	{
		0x01530000,
		ARRAY_SIZE(cnoc2_ss_mpu_rgs),
		cnoc2_ss_mpu_rgs,
		ARRAY_SIZE(cnoc2_ss_mpu_rg_addr),
		cnoc2_ss_mpu_rg_addr,
		XPU_TYPE_CNOC2_SS_MPU,
		XPU_PROTECTION_STATIC,
	},
	{
		0x08b14000,
		ARRAY_SIZE(wpss_mpu_rgs),
		wpss_mpu_rgs,
		ARRAY_SIZE(wpss_mpu_addr),
		wpss_mpu_addr,
		XPU_TYPE_WPSS_MPU,
		XPU_PROTECTION_STATIC,
	},
	{
		0x0eff0000,
		ARRAY_SIZE(aoss_cnoc_mpu_rgs),
		aoss_cnoc_mpu_rgs,
		ARRAY_SIZE(aoss_cnoc_mpu_rg_addr),
		aoss_cnoc_mpu_rg_addr,
		XPU_TYPE_AOSS_CNOC_MPU,
		XPU_PROTECTION_STATIC,
	},
};

const uint32_t msm_xpu_cfg_count = ARRAY_SIZE(msm_xpu_cfg);

static struct xpu_instance modem_mpus[] = {
	{
		0x016a8000,
		ARRAY_SIZE(anoc2_mpu_rgs),
		anoc2_mpu_rgs,
		ARRAY_SIZE(anoc2_mpu_rg_addr),
		anoc2_mpu_rg_addr,
		XPU_TYPE_ANOC2_MPU,
		0,
	},
	{
		0x091fc000,
		ARRAY_SIZE(mss_q6_mpu_rgs),
		mss_q6_mpu_rgs,
		ARRAY_SIZE(mss_q6_mpu_rg_addr),
		mss_q6_mpu_rg_addr,
		XPU_TYPE_MSS_Q6_MPU,
		0,
	},
};

static struct xpu_instance mss_nav_mpus[] = {
	{
		0x016a0000,
		ARRAY_SIZE(mss_nav_mpu_rgs),
		mss_nav_mpu_rgs,
		ARRAY_SIZE(mss_nav_mpu_rg_addr),
		mss_nav_mpu_rg_addr,
		XPU_TYPE_MSS_NAV_MPU,
		0,
	},
	{
		0x016b0000,
		ARRAY_SIZE(anoc1_mpu_rgs),
		anoc1_mpu_rgs,
		ARRAY_SIZE(anoc1_mpu_rg_addr),
		anoc1_mpu_rg_addr,
		XPU_TYPE_ANOC1_MPU,
		0,
	},
	{
		0x091fe000,
		ARRAY_SIZE(modem_ms_mpu_rgs),
		modem_ms_mpu_rgs,
		ARRAY_SIZE(modem_ms_mpu_rg_addr),
		modem_ms_mpu_rg_addr,
		XPU_TYPE_MSS_MPU,
		0,
	},
};

struct mpu_ranges msm_mpu_ranges[] = {
	{
		DEVICE_MODEM,
		ARRAY_SIZE(modem_mpus),
		NUM_MODEM_MPU_PARTITIONS,
		modem_mpus,
	},
	{
		DEVICE_MSS_NAV,
		ARRAY_SIZE(mss_nav_mpus),
		NUM_MSS_NAV_MPU_PARTITIONS,
		mss_nav_mpus,
	},
};

const uint32_t msm_mpu_ranges_count = ARRAY_SIZE(msm_mpu_ranges);
