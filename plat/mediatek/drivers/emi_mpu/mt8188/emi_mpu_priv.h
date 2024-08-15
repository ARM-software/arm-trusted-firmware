/*
 * Copyright (c) 2022-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EMI_MPU_PRIV_H
#define EMI_MPU_PRIV_H

#define ENABLE_EMI_MPU_SW_LOCK		(0)

#define EMI_MPU_CTRL			(EMI_MPU_BASE + 0x000)
#define EMI_MPU_DBG			(EMI_MPU_BASE + 0x004)
#define EMI_MPU_SA0			(EMI_MPU_BASE + 0x100)
#define EMI_MPU_EA0			(EMI_MPU_BASE + 0x200)
#define EMI_MPU_SA(region)		(EMI_MPU_SA0 + (region * 4))
#define EMI_MPU_EA(region)		(EMI_MPU_EA0 + (region * 4))
#define EMI_MPU_APC0			(EMI_MPU_BASE + 0x300)
#define EMI_MPU_APC(region, dgroup)	(EMI_MPU_APC0 + (region * 4) + (dgroup * 0x100))
#define EMI_MPU_CTRL_D0			(EMI_MPU_BASE + 0x800)
#define EMI_MPU_CTRL_D(domain)		(EMI_MPU_CTRL_D0 + (domain * 4))
#define EMI_RG_MASK_D0			(EMI_MPU_BASE + 0x900)
#define EMI_RG_MASK_D(domain)		(EMI_RG_MASK_D0 + (domain * 4))

#define SUB_EMI_MPU_CTRL		(SUB_EMI_MPU_BASE + 0x000)
#define SUB_EMI_MPU_DBG			(SUB_EMI_MPU_BASE + 0x004)
#define SUB_EMI_MPU_SA0			(SUB_EMI_MPU_BASE + 0x100)
#define SUB_EMI_MPU_EA0			(SUB_EMI_MPU_BASE + 0x200)
#define SUB_EMI_MPU_SA(region)		(SUB_EMI_MPU_SA0 + (region * 4))
#define SUB_EMI_MPU_EA(region)		(SUB_EMI_MPU_EA0 + (region * 4))
#define SUB_EMI_MPU_APC0		(SUB_EMI_MPU_BASE + 0x300)
#define SUB_EMI_MPU_APC(region, dgroup)	(SUB_EMI_MPU_APC0 + (region * 4) + (dgroup * 0x100))
#define SUB_EMI_MPU_CTRL_D0		(SUB_EMI_MPU_BASE + 0x800)
#define SUB_EMI_MPU_CTRL_D(domain)	(SUB_EMI_MPU_CTRL_D0 + (domain * 4))
#define SUB_EMI_RG_MASK_D0		(SUB_EMI_MPU_BASE + 0x900)
#define SUB_EMI_RG_MASK_D(domain)	(SUB_EMI_RG_MASK_D0 + (domain * 4))

#define EMI_MPU_DOMAIN_NUM		(16)
#define EMI_MPU_REGION_NUM		(32)
#define EMI_MPU_ALIGN_BITS		(16)
#define DRAM_START_ADDR                 (0x40000000ULL)
#define DRAM_OFFSET			(DRAM_START_ADDR >> EMI_MPU_ALIGN_BITS)
#define DRAM_MAX_SIZE			(0x200000000ULL)
#define BL32_REGION_BASE		(0x43000000ULL)
#define BL32_REGION_SIZE		(0x4600000ULL)
#define SCP_CORE0_REGION_BASE		(0x50000000ULL)
#define SCP_CORE0_REGION_SIZE		(0x800000ULL)
#define SCP_CORE1_REGION_BASE		(0x70000000ULL)
#define SCP_CORE1_REGION_SIZE		(0xa000000ULL)
#define DSP_PROTECT_REGION_BASE		(0x60000000ULL)
#define DSP_PROTECT_REGION_SIZE		(0x1100000ULL)

#define EMI_MPU_DGROUP_NUM		(EMI_MPU_DOMAIN_NUM / 8)

/* APU EMI MPU Setting */
#define APUSYS_SEC_BUF_PA		(0x55000000)
#define APUSYS_SEC_BUF_SZ		(0x100000)

#define SVP_DRAM_REGION_COUNT		(10)

enum region_ids {
	BL31_EMI_REGION_ID = 0,
	BL32_REGION_ID,
	SCP_CORE0_REGION_ID,
	SCP_CORE1_REGION_ID,
	DSP_PROTECT_REGION_ID,
	SVP_DRAM_REGION_ID_START = 5,
	SVP_DRAM_REGION_ID_END = SVP_DRAM_REGION_ID_START + SVP_DRAM_REGION_COUNT - 1,

	APUSYS_SEC_BUF_EMI_REGION_ID = 21,

	ALL_DEFAULT_REGION_ID = 31,
};
#endif
