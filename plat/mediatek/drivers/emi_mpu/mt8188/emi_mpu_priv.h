/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EMI_MPU_PRIV_H
#define EMI_MPU_PRIV_H

#define ENABLE_EMI_MPU_SW_LOCK		(1)

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
#define DRAM_OFFSET			(0x40000000 >> EMI_MPU_ALIGN_BITS)

#define EMI_MPU_DGROUP_NUM		(EMI_MPU_DOMAIN_NUM / 8)

#endif
