/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TZC_COMMON_H__
#define __TZC_COMMON_H__

/*
 * Offset of core registers from the start of the base of configuration
 * registers for each region.
 */

/* ID Registers */
#define PID0_OFF					0xfe0
#define PID1_OFF					0xfe4
#define PID2_OFF					0xfe8
#define PID3_OFF					0xfec
#define PID4_OFF					0xfd0
#define CID0_OFF					0xff0
#define CID1_OFF					0xff4
#define CID2_OFF					0xff8
#define CID3_OFF					0xffc

/* Bit positions of TZC_ACTION registers */
#define TZC_ACTION_RV_SHIFT				0
#define TZC_ACTION_RV_MASK				0x3
#define TZC_ACTION_RV_LOWOK				0x0
#define TZC_ACTION_RV_LOWERR				0x1
#define TZC_ACTION_RV_HIGHOK				0x2
#define TZC_ACTION_RV_HIGHERR				0x3

/* Used along with 'tzc_region_attributes_t' below */
#define TZC_REGION_ATTR_S_RD_SHIFT			30
#define TZC_REGION_ATTR_S_WR_SHIFT			31
#define TZC_REGION_ATTR_F_EN_SHIFT			0
#define TZC_REGION_ATTR_SEC_SHIFT			30
#define TZC_REGION_ATTR_S_RD_MASK			0x1
#define TZC_REGION_ATTR_S_WR_MASK			0x1
#define TZC_REGION_ATTR_SEC_MASK			0x3

#define TZC_REGION_ACCESS_WR_EN_SHIFT			16
#define TZC_REGION_ACCESS_RD_EN_SHIFT			0
#define TZC_REGION_ACCESS_ID_MASK			0xf

/* Macros for allowing Non-Secure access to a region based on NSAID */
#define TZC_REGION_ACCESS_RD(nsaid)				\
	((1 << (nsaid & TZC_REGION_ACCESS_ID_MASK)) <<		\
	 TZC_REGION_ACCESS_RD_EN_SHIFT)
#define TZC_REGION_ACCESS_WR(nsaid)				\
	((1 << (nsaid & TZC_REGION_ACCESS_ID_MASK)) <<		\
	 TZC_REGION_ACCESS_WR_EN_SHIFT)
#define TZC_REGION_ACCESS_RDWR(nsaid)				\
	(TZC_REGION_ACCESS_RD(nsaid) |				\
	TZC_REGION_ACCESS_WR(nsaid))

#ifndef __ASSEMBLY__

/* Returns offset of registers to program for a given region no */
#define TZC_REGION_OFFSET(region_size, region_no)	\
				((region_size) * (region_no))

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
typedef enum {
	TZC_ACTION_NONE = 0,
	TZC_ACTION_ERR = 1,
	TZC_ACTION_INT = 2,
	TZC_ACTION_ERR_INT = (TZC_ACTION_ERR | TZC_ACTION_INT)
} tzc_action_t;

/*
 * Controls secure access to a region. If not enabled secure access is not
 * allowed to region.
 */
typedef enum {
	TZC_REGION_S_NONE = 0,
	TZC_REGION_S_RD = 1,
	TZC_REGION_S_WR = 2,
	TZC_REGION_S_RDWR = (TZC_REGION_S_RD | TZC_REGION_S_WR)
} tzc_region_attributes_t;

#endif /* __ASSEMBLY__ */
#endif /* __TZC_COMMON_H__ */
