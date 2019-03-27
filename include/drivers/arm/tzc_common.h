/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TZC_COMMON_H
#define TZC_COMMON_H

#include <lib/utils_def.h>

/*
 * Offset of core registers from the start of the base of configuration
 * registers for each region.
 */

/* ID Registers */
#define PID0_OFF					U(0xfe0)
#define PID1_OFF					U(0xfe4)
#define PID2_OFF					U(0xfe8)
#define PID3_OFF					U(0xfec)
#define PID4_OFF					U(0xfd0)
#define CID0_OFF					U(0xff0)
#define CID1_OFF					U(0xff4)
#define CID2_OFF					U(0xff8)
#define CID3_OFF					U(0xffc)

/*
 * What type of action is expected when an access violation occurs.
 * The memory requested is returned as zero. But we can also raise an event to
 * let the system know it happened.
 * We can raise an interrupt(INT) and/or cause an exception(ERR).
 *  TZC_ACTION_NONE    - No interrupt, no Exception
 *  TZC_ACTION_ERR     - No interrupt, raise exception -> sync external
 *                       data abort
 *  TZC_ACTION_INT     - Raise interrupt, no exception
 *  TZC_ACTION_ERR_INT - Raise interrupt, raise exception -> sync
 *                       external data abort
 */
#define TZC_ACTION_NONE			U(0)
#define TZC_ACTION_ERR			U(1)
#define TZC_ACTION_INT			U(2)
#define TZC_ACTION_ERR_INT		(TZC_ACTION_ERR | TZC_ACTION_INT)

/* Bit positions of TZC_ACTION registers */
#define TZC_ACTION_RV_SHIFT				0
#define TZC_ACTION_RV_MASK				U(0x3)
#define TZC_ACTION_RV_LOWOK				U(0x0)
#define TZC_ACTION_RV_LOWERR				U(0x1)
#define TZC_ACTION_RV_HIGHOK				U(0x2)
#define TZC_ACTION_RV_HIGHERR				U(0x3)

/*
 * Controls secure access to a region. If not enabled secure access is not
 * allowed to region.
 */
#define TZC_REGION_S_NONE		U(0)
#define TZC_REGION_S_RD			U(1)
#define TZC_REGION_S_WR			U(2)
#define TZC_REGION_S_RDWR		(TZC_REGION_S_RD | TZC_REGION_S_WR)

#define TZC_REGION_ATTR_S_RD_SHIFT			30
#define TZC_REGION_ATTR_S_WR_SHIFT			31
#define TZC_REGION_ATTR_F_EN_SHIFT			0
#define TZC_REGION_ATTR_SEC_SHIFT			30
#define TZC_REGION_ATTR_S_RD_MASK			U(0x1)
#define TZC_REGION_ATTR_S_WR_MASK			U(0x1)
#define TZC_REGION_ATTR_SEC_MASK			U(0x3)

#define TZC_REGION_ACCESS_WR_EN_SHIFT			16
#define TZC_REGION_ACCESS_RD_EN_SHIFT			0
#define TZC_REGION_ACCESS_ID_MASK			U(0xf)

/* Macros for allowing Non-Secure access to a region based on NSAID */
#define TZC_REGION_ACCESS_RD(nsaid)				\
	((U(1) << ((nsaid) & TZC_REGION_ACCESS_ID_MASK)) <<	\
	 TZC_REGION_ACCESS_RD_EN_SHIFT)
#define TZC_REGION_ACCESS_WR(nsaid)				\
	((U(1) << ((nsaid) & TZC_REGION_ACCESS_ID_MASK)) <<	\
	 TZC_REGION_ACCESS_WR_EN_SHIFT)
#define TZC_REGION_ACCESS_RDWR(nsaid)				\
	(TZC_REGION_ACCESS_RD(nsaid) |				\
	TZC_REGION_ACCESS_WR(nsaid))

/* Returns offset of registers to program for a given region no */
#define TZC_REGION_OFFSET(region_size, region_no)	\
				((region_size) * (region_no))

#endif /* TZC_COMMON_H */
