/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPT_RME_PRIVATE_H
#define GPT_RME_PRIVATE_H

#include <arch.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/utils_def.h>

/******************************************************************************/
/* GPT descriptor definitions                                                 */
/******************************************************************************/

/* GPT level 0 descriptor bit definitions. */
#define GPT_L0_TYPE_MASK		UL(0xF)
#define GPT_L0_TYPE_SHIFT		U(0)

/* For now, we don't support contiguous descriptors, only table and block. */
#define GPT_L0_TYPE_TBL_DESC		UL(0x3)
#define GPT_L0_TYPE_BLK_DESC		UL(0x1)

#define GPT_L0_TBL_DESC_L1ADDR_MASK	UL(0xFFFFFFFFFF)
#define GPT_L0_TBL_DESC_L1ADDR_SHIFT	U(12)

#define GPT_L0_BLK_DESC_GPI_MASK	UL(0xF)
#define GPT_L0_BLK_DESC_GPI_SHIFT	U(4)

/* GPT level 1 descriptor bit definitions */
#define GPT_L1_GRAN_DESC_GPI_MASK	UL(0xF)

/*
 * This macro fills out every GPI entry in a granules descriptor to the same
 * value.
 */
#define GPT_BUILD_L1_DESC(_gpi)		(((uint64_t)(_gpi) << 4*0) | \
					 ((uint64_t)(_gpi) << 4*1) | \
					 ((uint64_t)(_gpi) << 4*2) | \
					 ((uint64_t)(_gpi) << 4*3) | \
					 ((uint64_t)(_gpi) << 4*4) | \
					 ((uint64_t)(_gpi) << 4*5) | \
					 ((uint64_t)(_gpi) << 4*6) | \
					 ((uint64_t)(_gpi) << 4*7) | \
					 ((uint64_t)(_gpi) << 4*8) | \
					 ((uint64_t)(_gpi) << 4*9) | \
					 ((uint64_t)(_gpi) << 4*10) | \
					 ((uint64_t)(_gpi) << 4*11) | \
					 ((uint64_t)(_gpi) << 4*12) | \
					 ((uint64_t)(_gpi) << 4*13) | \
					 ((uint64_t)(_gpi) << 4*14) | \
					 ((uint64_t)(_gpi) << 4*15))

/******************************************************************************/
/* GPT platform configuration                                                 */
/******************************************************************************/

/* This value comes from GPCCR_EL3 so no externally supplied definition. */
#define GPT_L0GPTSZ		((unsigned int)((read_gpccr_el3() >> \
				GPCCR_L0GPTSZ_SHIFT) & GPCCR_L0GPTSZ_MASK))

/* The "S" value is directly related to L0GPTSZ */
#define GPT_S_VAL		(GPT_L0GPTSZ + 30U)

/*
 * Map PPS values to T values.
 *
 *   PPS    Size    T
 *   0b000  4GB     32
 *   0b001  64GB    36
 *   0b010  1TB     40
 *   0b011  4TB     42
 *   0b100  16TB    44
 *   0b101  256TB   48
 *   0b110  4PB     52
 *
 * See section 15.1.27 of the RME specification.
 */
typedef enum {
	PPS_4GB_T =	32U,
	PPS_64GB_T =	36U,
	PPS_1TB_T =	40U,
	PPS_4TB_T =	42U,
	PPS_16TB_T =	44U,
	PPS_256TB_T =	48U,
	PPS_4PB_T =	52U
} gpt_t_val_e;

/*
 * Map PGS values to P values.
 *
 *   PGS    Size    P
 *   0b00   4KB     12
 *   0b10   16KB    14
 *   0b01   64KB    16
 *
 * Note that pgs=0b10 is 16KB and pgs=0b01 is 64KB, this is not a typo.
 *
 * See section 15.1.27 of the RME specification.
 */
typedef enum {
	PGS_4KB_P =	12U,
	PGS_16KB_P =	14U,
	PGS_64KB_P =	16U
} gpt_p_val_e;

/* Max valid value for PGS. */
#define GPT_PGS_MAX			(2U)

/* Max valid value for PPS. */
#define GPT_PPS_MAX			(6U)

/******************************************************************************/
/* L0 address attribute macros                                                */
/******************************************************************************/

/*
 * Width of the L0 index field.
 *
 * If S is greater than or equal to T then there is a single L0 region covering
 * the entire protected space so there is no L0 index, so the width (and the
 * derivative mask value) are both zero.  If we don't specifically handle this
 * special case we'll get a negative width value which does not make sense and
 * would cause problems.
 */
#define GPT_L0_IDX_WIDTH(_t)		(((_t) > GPT_S_VAL) ? \
					((_t) - GPT_S_VAL) : (0U))

/* Bit shift for the L0 index field in a PA. */
#define GPT_L0_IDX_SHIFT		(GPT_S_VAL)

/*
 * Mask for the L0 index field, must be shifted.
 *
 * The value 0x3FFFFF is 22 bits wide which is the maximum possible width of the
 * L0 index within a physical address. This is calculated by
 * ((t_max - 1) - s_min + 1) where t_max is 52 for 4PB, the largest PPS, and
 * s_min is 30 for 1GB, the smallest L0GPTSZ.
 */
#define GPT_L0_IDX_MASK(_t)		(0x3FFFFFUL >> (22U - \
					(GPT_L0_IDX_WIDTH(_t))))

/* Total number of L0 regions. */
#define GPT_L0_REGION_COUNT(_t)		((GPT_L0_IDX_MASK(_t)) + 1U)

/* Total size of each GPT L0 region in bytes. */
#define GPT_L0_REGION_SIZE		(1UL << (GPT_L0_IDX_SHIFT))

/* Total size in bytes of the whole L0 table. */
#define GPT_L0_TABLE_SIZE(_t)		((GPT_L0_REGION_COUNT(_t)) << 3U)

/******************************************************************************/
/* L1 address attribute macros                                                */
/******************************************************************************/

/*
 * Width of the L1 index field.
 *
 * This field does not have a special case to handle widths less than zero like
 * the L0 index field above since all valid combinations of PGS (p) and L0GPTSZ
 * (s) will result in a positive width value.
 */
#define GPT_L1_IDX_WIDTH(_p)		((GPT_S_VAL - 1U) - ((_p) + 3U))

/* Bit shift for the L1 index field. */
#define GPT_L1_IDX_SHIFT(_p)		((_p) + 4U)

/*
 * Mask for the L1 index field, must be shifted.
 *
 * The value 0x7FFFFF is 23 bits wide and is the maximum possible width of the
 * L1 index within a physical address. It is calculated by
 * ((s_max - 1) - (p_min + 4) + 1) where s_max is 39 for 512gb, the largest
 * L0GPTSZ, and p_min is 12 for 4KB granules, the smallest PGS.
 */
#define GPT_L1_IDX_MASK(_p)		(0x7FFFFFUL >> (23U - \
					(GPT_L1_IDX_WIDTH(_p))))

/* Bit shift for the index of the L1 GPI in a PA. */
#define GPT_L1_GPI_IDX_SHIFT(_p)	(_p)

/* Mask for the index of the L1 GPI in a PA. */
#define GPT_L1_GPI_IDX_MASK		(0xF)

/* Total number of entries in each L1 table. */
#define GPT_L1_ENTRY_COUNT(_p)		((GPT_L1_IDX_MASK(_p)) + 1U)

/* Total size in bytes of each L1 table. */
#define GPT_L1_TABLE_SIZE(_p)		((GPT_L1_ENTRY_COUNT(_p)) << 3U)

/******************************************************************************/
/* General helper macros                                                      */
/******************************************************************************/

/* Protected space actual size in bytes. */
#define GPT_PPS_ACTUAL_SIZE(_t)	(1UL << (_t))

/* Granule actual size in bytes. */
#define GPT_PGS_ACTUAL_SIZE(_p)	(1UL << (_p))

/* L0 GPT region size in bytes. */
#define GPT_L0GPTSZ_ACTUAL_SIZE	(1UL << GPT_S_VAL)

/* Get the index of the L0 entry from a physical address. */
#define GPT_L0_IDX(_pa)		((_pa) >> GPT_L0_IDX_SHIFT)

/*
 * This definition is used to determine if a physical address lies on an L0
 * region boundary.
 */
#define GPT_IS_L0_ALIGNED(_pa)	(((_pa) & (GPT_L0_REGION_SIZE - U(1))) == U(0))

/* Get the type field from an L0 descriptor. */
#define GPT_L0_TYPE(_desc)	(((_desc) >> GPT_L0_TYPE_SHIFT) & \
				GPT_L0_TYPE_MASK)

/* Create an L0 block descriptor. */
#define GPT_L0_BLK_DESC(_gpi)	(GPT_L0_TYPE_BLK_DESC | \
				(((_gpi) & GPT_L0_BLK_DESC_GPI_MASK) << \
				GPT_L0_BLK_DESC_GPI_SHIFT))

/* Create an L0 table descriptor with an L1 table address. */
#define GPT_L0_TBL_DESC(_pa)	(GPT_L0_TYPE_TBL_DESC | ((uint64_t)(_pa) & \
				(GPT_L0_TBL_DESC_L1ADDR_MASK << \
				GPT_L0_TBL_DESC_L1ADDR_SHIFT)))

/* Get the GPI from an L0 block descriptor. */
#define GPT_L0_BLKD_GPI(_desc)	(((_desc) >> GPT_L0_BLK_DESC_GPI_SHIFT) & \
				GPT_L0_BLK_DESC_GPI_MASK)

/* Get the L1 address from an L0 table descriptor. */
#define GPT_L0_TBLD_ADDR(_desc)	((uint64_t *)(((_desc) & \
				(GPT_L0_TBL_DESC_L1ADDR_MASK << \
				GPT_L0_TBL_DESC_L1ADDR_SHIFT))))

/* Get the index into the L1 table from a physical address. */
#define GPT_L1_IDX(_p, _pa)	(((_pa) >> GPT_L1_IDX_SHIFT(_p)) & \
				GPT_L1_IDX_MASK(_p))

/* Get the index of the GPI within an L1 table entry from a physical address. */
#define GPT_L1_GPI_IDX(_p, _pa)	(((_pa) >> GPT_L1_GPI_IDX_SHIFT(_p)) & \
				GPT_L1_GPI_IDX_MASK)

/* Determine if an address is granule-aligned. */
#define GPT_IS_L1_ALIGNED(_p, _pa) (((_pa) & (GPT_PGS_ACTUAL_SIZE(_p) - U(1))) \
				   == U(0))

#endif /* GPT_RME_PRIVATE_H */
