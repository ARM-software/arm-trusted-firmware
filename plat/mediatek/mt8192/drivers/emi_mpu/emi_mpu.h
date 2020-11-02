/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EMI_MPU_H
#define EMI_MPU_H

#include <platform_def.h>

#define EMI_MPUP		(EMI_BASE + 0x01D8)
#define EMI_MPUQ		(EMI_BASE + 0x01E0)
#define EMI_MPUR		(EMI_BASE + 0x01E8)
#define EMI_MPUS		(EMI_BASE + 0x01F0)
#define EMI_MPUT		(EMI_BASE + 0x01F8)
#define EMI_MPUY		(EMI_BASE + 0x0220)
#define EMI_MPU_CTRL		(EMI_MPU_BASE + 0x0000)
#define EMI_MPUD0_ST		(EMI_BASE + 0x0160)
#define EMI_MPUD1_ST		(EMI_BASE + 0x0164)
#define EMI_MPUD2_ST		(EMI_BASE + 0x0168)
#define EMI_MPUD3_ST		(EMI_BASE + 0x016C)
#define EMI_MPUD0_ST2		(EMI_BASE + 0x0200)
#define EMI_MPUD1_ST2		(EMI_BASE + 0x0204)
#define EMI_MPUD2_ST2		(EMI_BASE + 0x0208)
#define EMI_MPUD3_ST2		(EMI_BASE + 0x020C)

#define EMI_PHY_OFFSET		(0x40000000UL)

#define NO_PROT 		(0)
#define SEC_RW			(1)
#define SEC_RW_NSEC_R		(2)
#define SEC_RW_NSEC_W		(3)
#define SEC_R_NSEC_R		(4)
#define FORBIDDEN		(5)
#define SEC_R_NSEC_RW		(6)

#define SECURE_OS_MPU_REGION_ID	(0)
#define ATF_MPU_REGION_ID	(1)

#define EMI_MPU_SA0		(EMI_MPU_BASE + 0x100)
#define EMI_MPU_EA0		(EMI_MPU_BASE + 0x200)
#define EMI_MPU_SA(region)	(EMI_MPU_SA0 + (region) * 4)
#define EMI_MPU_EA(region)	(EMI_MPU_EA0 + (region) * 4)

#define EMI_MPU_APC0			(EMI_MPU_BASE + 0x300)
#define EMI_MPU_APC(region, dgroup)	(EMI_MPU_APC0 + (region) * 4 + \
					(dgroup) * 0x100)

#define EMI_MPU_CTRL_D0		(EMI_MPU_BASE + 0x800)
#define EMI_MPU_CTRL_D(domain)	(EMI_MPU_CTRL_D0 + domain * 4)
#define EMI_RG_MASK_D0		(EMI_MPU_BASE + 0x900)
#define EMI_RG_MASK_D(domain)	(EMI_RG_MASK_D0 + domain * 4)

#define EMI_MPU_DOMAIN_NUM	16
#define EMI_MPU_REGION_NUM	32
#define EMI_MPU_ALIGN_BITS	16
#define DRAM_OFFSET		(0x40000000 >> EMI_MPU_ALIGN_BITS)

#define EMI_MPU_DGROUP_NUM	(EMI_MPU_DOMAIN_NUM / 8)

#if (EMI_MPU_DGROUP_NUM == 1)
#define SET_ACCESS_PERMISSION(apc_ary, lock, d7, d6, d5, d4, d3, d2, d1, d0) \
do { \
	apc_ary[0] = 0; \
	apc_ary[0] = \
		(((unsigned int)    d7) << 21) | (((unsigned int)  d6) << 18) \
		| (((unsigned int)  d5) << 15) | (((unsigned int)  d4) << 12) \
		| (((unsigned int)  d3) <<  9) | (((unsigned int)  d2) <<  6) \
		| (((unsigned int)  d1) <<  3) | ((unsigned int)   d0) \
		| (((unsigned int) lock) << 31); \
} while (0)
#elif (EMI_MPU_DGROUP_NUM == 2)
#define SET_ACCESS_PERMISSION(apc_ary, lock, d15, d14, d13, d12, d11, d10, \
			      d9, d8, d7, d6, d5, d4, d3, d2, d1, d0) \
do { \
	apc_ary[1] = \
		(((unsigned int)   d15) << 21) | (((unsigned int) d14) << 18) \
		| (((unsigned int) d13) << 15) | (((unsigned int) d12) << 12) \
		| (((unsigned int) d11) <<  9) | (((unsigned int) d10) <<  6) \
		| (((unsigned int)  d9) <<  3) |  ((unsigned int)  d8); \
	apc_ary[0] = \
		(((unsigned int)    d7) << 21) | (((unsigned int)  d6) << 18) \
		| (((unsigned int)  d5) << 15) | (((unsigned int)  d4) << 12) \
		| (((unsigned int)  d3) <<  9) | (((unsigned int)  d2) <<  6) \
		| (((unsigned int)  d1) <<  3) |  ((unsigned int)  d0) \
		| (((unsigned int) lock) << 31); \
} while (0)
#endif

struct emi_region_info_t {
	unsigned long long	start;
	unsigned long long	end;
	unsigned int		region;
	unsigned long		apc[EMI_MPU_DGROUP_NUM];
};

void emi_mpu_init(void);
int emi_mpu_set_protection(struct emi_region_info_t *region_info);
void dump_emi_mpu_regions(void);

#endif  /* __EMI_MPU_H */
