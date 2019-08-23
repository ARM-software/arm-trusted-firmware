/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __EMI_MPU_H
#define __EMI_MPU_H

#include <platform_def.h>

#define EMI_MPUP		(EMI_BASE + 0x01D8)
#define EMI_MPUQ		(EMI_BASE + 0x01E0)
#define EMI_MPUR		(EMI_BASE + 0x01E8)
#define EMI_MPUS		(EMI_BASE + 0x01F0)
#define EMI_MPUT		(EMI_BASE + 0x01F8)
#define EMI_MPUY		(EMI_BASE + 0x0220)
#define EMI_MPU_CTRL	(EMI_MPU_BASE + 0x0000)
#define EMI_MPUD0_ST	(EMI_BASE + 0x0160)
#define EMI_MPUD1_ST	(EMI_BASE + 0x0164)
#define EMI_MPUD2_ST	(EMI_BASE + 0x0168)
#define EMI_MPUD3_ST	(EMI_BASE + 0x016C)
#define EMI_MPUD0_ST2	(EMI_BASE + 0x0200)
#define EMI_MPUD1_ST2	(EMI_BASE + 0x0204)
#define EMI_MPUD2_ST2	(EMI_BASE + 0x0208)
#define EMI_MPUD3_ST2	(EMI_BASE + 0x020C)

#define EMI_PHY_OFFSET	(0x40000000UL)
#define EIGHT_DOMAIN

#define NO_PROTECTION	(0)
#define SEC_RW			(1)
#define SEC_RW_NSEC_R	(2)
#define SEC_RW_NSEC_W	(3)
#define SEC_R_NSEC_R	(4)
#define FORBIDDEN		(5)
#define SEC_R_NSEC_RW	(6)

#define SECURE_OS_MPU_REGION_ID	(0)
#define ATF_MPU_REGION_ID		(1)

#ifdef EIGHT_DOMAIN
#define SET_ACCESS_PERMISSON(d7, d6, d5, d4, d3, d2, d1, d0) \
	(((d7) << 21) | ((d6) << 18) | ((d5) << 15) | ((d4) << 12) \
	| ((d3) << 9) | ((d2) << 6) | ((d1) << 3) | (d0))
#else
#define SET_ACCESS_PERMISSON(d3, d2, d1, d0) \
	(((d3) << 9) | ((d2) << 6) | ((d1) << 3) | (d0))
#endif

//#define EMI_MPU_BASE                (0x1020E000U)

#define EMI_MPU_SA0                 (EMI_MPU_BASE + 0x100)
#define EMI_MPU_SA1                 (EMI_MPU_BASE + 0x104)
#define EMI_MPU_SA2                 (EMI_MPU_BASE + 0x108)
#define EMI_MPU_SA3                 (EMI_MPU_BASE + 0x10C)
#define EMI_MPU_SA4                 (EMI_MPU_BASE + 0x110)
#define EMI_MPU_SA5                 (EMI_MPU_BASE + 0x114)
#define EMI_MPU_SA6                 (EMI_MPU_BASE + 0x118)
#define EMI_MPU_SA7                 (EMI_MPU_BASE + 0x11C)

#define EMI_MPU_EA0                 (EMI_MPU_BASE + 0x200)
#define EMI_MPU_EA1                 (EMI_MPU_BASE + 0x204)
#define EMI_MPU_EA2                 (EMI_MPU_BASE + 0x208)
#define EMI_MPU_EA3                 (EMI_MPU_BASE + 0x20C)
#define EMI_MPU_EA4                 (EMI_MPU_BASE + 0x210)
#define EMI_MPU_EA5                 (EMI_MPU_BASE + 0x214)
#define EMI_MPU_EA6                 (EMI_MPU_BASE + 0x218)
#define EMI_MPU_EA7                 (EMI_MPU_BASE + 0x21C)

#define EMI_MPU_APC0                (EMI_MPU_BASE + 0x300)
#define EMI_MPU_APC1                (EMI_MPU_BASE + 0x304)
#define EMI_MPU_APC2                (EMI_MPU_BASE + 0x308)
#define EMI_MPU_APC3                (EMI_MPU_BASE + 0x30C)
#define EMI_MPU_APC4                (EMI_MPU_BASE + 0x310)
#define EMI_MPU_APC5                (EMI_MPU_BASE + 0x314)
#define EMI_MPU_APC6                (EMI_MPU_BASE + 0x318)
#define EMI_MPU_APC7                (EMI_MPU_BASE + 0x31C)

#define EMI_MPU_CTRL_D0             (EMI_MPU_BASE + 0x800)
#define EMI_MPU_CTRL_D1             (EMI_MPU_BASE + 0x804)
#define EMI_MPU_CTRL_D2             (EMI_MPU_BASE + 0x808)
#define EMI_MPU_CTRL_D3             (EMI_MPU_BASE + 0x80C)
#define EMI_MPU_CTRL_D4             (EMI_MPU_BASE + 0x810)
#define EMI_MPU_CTRL_D5             (EMI_MPU_BASE + 0x814)
#define EMI_MPU_CTRL_D6             (EMI_MPU_BASE + 0x818)
#define EMI_MPU_CTRL_D7             (EMI_MPU_BASE + 0x81C)

#define EMI_MPU_MASK_D0             (EMI_MPU_BASE + 0x900)
#define EMI_MPU_MASK_D1             (EMI_MPU_BASE + 0x904)
#define EMI_MPU_MASK_D2             (EMI_MPU_BASE + 0x908)
#define EMI_MPU_MASK_D3             (EMI_MPU_BASE + 0x90C)
#define EMI_MPU_MASK_D4             (EMI_MPU_BASE + 0x910)
#define EMI_MPU_MASK_D5             (EMI_MPU_BASE + 0x914)
#define EMI_MPU_MASK_D6             (EMI_MPU_BASE + 0x918)
#define EMI_MPU_MASK_D7             (EMI_MPU_BASE + 0x91C)

int emi_mpu_set_region_protection(
	unsigned long start, unsigned long end,
	int region,
	unsigned int access_permission);

void dump_emi_mpu_regions(void);
void emi_mpu_init(void);

#endif  /* __EMI_MPU_H */
