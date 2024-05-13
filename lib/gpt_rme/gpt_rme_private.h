/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPT_RME_PRIVATE_H
#define GPT_RME_PRIVATE_H

#include <arch.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>

/******************************************************************************/
/* GPT descriptor definitions                                                 */
/******************************************************************************/

/* GPT level 0 descriptor bit definitions */
#define GPT_L0_TYPE_MASK		UL(0xF)
#define GPT_L0_TYPE_SHIFT		U(0)

/* GPT level 0 table and block descriptors */
#define GPT_L0_TYPE_TBL_DESC		UL(3)
#define GPT_L0_TYPE_BLK_DESC		UL(1)

#define GPT_L0_TBL_DESC_L1ADDR_MASK	UL(0xFFFFFFFFFF)
#define GPT_L0_TBL_DESC_L1ADDR_SHIFT	U(12)

#define GPT_L0_BLK_DESC_GPI_MASK	UL(0xF)
#define GPT_L0_BLK_DESC_GPI_SHIFT	U(4)

/* GPT level 1 Contiguous descriptor */
#define GPT_L1_TYPE_CONT_DESC_MASK	UL(0xF)
#define GPT_L1_TYPE_CONT_DESC		UL(1)

/* GPT level 1 Contiguous descriptor definitions */
#define GPT_L1_CONTIG_2MB		UL(1)
#define GPT_L1_CONTIG_32MB		UL(2)
#define GPT_L1_CONTIG_512MB		UL(3)

#define GPT_L1_CONT_DESC_GPI_SHIFT	U(4)
#define GPT_L1_CONT_DESC_GPI_MASK	UL(0xF)
#define GPT_L1_CONT_DESC_CONTIG_SHIFT	U(8)
#define GPT_L1_CONT_DESC_CONTIG_MASK	UL(3)

/* GPT level 1 Granules descriptor bit definitions */
#define GPT_L1_GRAN_DESC_GPI_MASK	UL(0xF)

/* L1 Contiguous descriptors templates */
#define GPT_L1_CONT_DESC_2MB	\
			(GPT_L1_TYPE_CONT_DESC |	\
			(GPT_L1_CONTIG_2MB << GPT_L1_CONT_DESC_CONTIG_SHIFT))
#define GPT_L1_CONT_DESC_32MB	\
			(GPT_L1_TYPE_CONT_DESC |	\
			(GPT_L1_CONTIG_32MB << GPT_L1_CONT_DESC_CONTIG_SHIFT))
#define GPT_L1_CONT_DESC_512MB	\
			(GPT_L1_TYPE_CONT_DESC |	\
			(GPT_L1_CONTIG_512MB << GPT_L1_CONT_DESC_CONTIG_SHIFT))

/* Create L1 Contiguous descriptor from GPI and template */
#define GPT_L1_GPI_CONT_DESC(_gpi, _desc)	\
			((_desc) | ((uint64_t)(_gpi) << GPT_L1_CONT_DESC_GPI_SHIFT))

/* Create L1 Contiguous descriptor from Granules descriptor and size */
#define GPT_L1_CONT_DESC(_desc, _size) \
				(GPT_L1_CONT_DESC_##_size	| \
				(((_desc) & GPT_L1_GRAN_DESC_GPI_MASK) << \
				GPT_L1_CONT_DESC_GPI_SHIFT))

/* Create L1 Contiguous descriptor from GPI and size */
#define GPT_L1_CONT_DESC_SIZE(_gpi, _size) \
				(GPT_L1_CONT_DESC_##_size	| \
				(((uint64_t)(_gpi) << GPT_L1_CONT_DESC_GPI_SHIFT))

#define GPT_L1_GPI_BYTE(_gpi)		(uint64_t)((_gpi) | ((_gpi) << 4))
#define GPT_L1_GPI_HALF(_gpi)		(GPT_L1_GPI_BYTE(_gpi) | (GPT_L1_GPI_BYTE(_gpi) << 8))
#define GPT_L1_GPI_WORD(_gpi)		(GPT_L1_GPI_HALF(_gpi) | (GPT_L1_GPI_HALF(_gpi) << 16))

/*
 * This macro generates a Granules descriptor
 * with the same value for every GPI entry.
 */
#define GPT_BUILD_L1_DESC(_gpi)		(GPT_L1_GPI_WORD(_gpi) | (GPT_L1_GPI_WORD(_gpi) << 32))

#define GPT_L1_SECURE_DESC	GPT_BUILD_L1_DESC(GPT_GPI_SECURE)
#define GPT_L1_NS_DESC		GPT_BUILD_L1_DESC(GPT_GPI_NS)
#define GPT_L1_REALM_DESC	GPT_BUILD_L1_DESC(GPT_GPI_REALM)
#define GPT_L1_ANY_DESC		GPT_BUILD_L1_DESC(GPT_GPI_ANY)

/******************************************************************************/
/* GPT platform configuration                                                 */
/******************************************************************************/

/* This value comes from GPCCR_EL3 so no externally supplied definition */
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

#define LOCK_SIZE	sizeof(((bitlock_t *)NULL)->lock)
#define LOCK_TYPE	typeof(((bitlock_t *)NULL)->lock)
#define LOCK_BITS	(LOCK_SIZE * 8U)

/*
 * Internal structure to retrieve the values from get_gpi_params();
 */
typedef struct {
	uint64_t gpt_l1_desc;
	uint64_t *gpt_l1_addr;
	unsigned int idx;
	unsigned int gpi_shift;
	unsigned int gpi;
#if (RME_GPT_BITLOCK_BLOCK != 0)
	bitlock_t *lock;
	LOCK_TYPE mask;
#endif
} gpi_info_t;

/*
 * Look up structure for contiguous blocks and descriptors
 */
typedef struct {
	size_t size;
	unsigned int desc;
} gpt_fill_lookup_t;

typedef void (*gpt_shatter_func)(uintptr_t base, const gpi_info_t *gpi_info,
					uint64_t l1_desc);
typedef void (*gpt_tlbi_func)(uintptr_t base);

/*
 * Look-up structure for
 * invalidating TLBs of GPT entries by Physical address, last level.
 */
typedef struct {
	gpt_tlbi_func function;
	size_t mask;
} gpt_tlbi_lookup_t;

/* Max valid value for PGS */
#define GPT_PGS_MAX			(2U)

/* Max valid value for PPS */
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
#define GPT_L0_IDX_WIDTH(_t)		(((unsigned int)(_t) > GPT_S_VAL) ? \
					((unsigned int)(_t) - GPT_S_VAL) : (0U))

/* Bit shift for the L0 index field in a PA */
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

/* Total number of L0 regions */
#define GPT_L0_REGION_COUNT(_t)		((GPT_L0_IDX_MASK(_t)) + 1U)

/* Total size of each GPT L0 region in bytes */
#define GPT_L0_REGION_SIZE		(1UL << (GPT_L0_IDX_SHIFT))

/* Total size in bytes of the whole L0 table */
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
#define GPT_L1_IDX_WIDTH(_p)		((GPT_S_VAL - 1U) - \
					((unsigned int)(_p) + 3U))

/* Bit shift for the L1 index field */
#define GPT_L1_IDX_SHIFT(_p)		((unsigned int)(_p) + 4U)

/*
 * Mask for the L1 index field, must be shifted.
 *
 * The value 0x7FFFFF is 23 bits wide and is the maximum possible width of the
 * L1 index within a physical address. It is calculated by
 * ((s_max - 1) - (p_min + 4) + 1) where s_max is 39 for 512GB, the largest
 * L0GPTSZ, and p_min is 12 for 4KB granules, the smallest PGS.
 */
#define GPT_L1_IDX_MASK(_p)		(0x7FFFFFUL >> (23U - \
					(GPT_L1_IDX_WIDTH(_p))))

/* Bit shift for the index of the L1 GPI in a PA */
#define GPT_L1_GPI_IDX_SHIFT(_p)	(_p)

/* Mask for the index of the L1 GPI in a PA */
#define GPT_L1_GPI_IDX_MASK		(0xF)

/* Total number of entries in each L1 table */
#define GPT_L1_ENTRY_COUNT(_p)		((GPT_L1_IDX_MASK(_p)) + 1UL)

/* Number of L1 entries in 2MB block */
#define GPT_L1_ENTRY_COUNT_2MB(_p)	(SZ_2M >> GPT_L1_IDX_SHIFT(_p))

/* Total size in bytes of each L1 table */
#define GPT_L1_TABLE_SIZE(_p)		((GPT_L1_ENTRY_COUNT(_p)) << 3U)

/******************************************************************************/
/* General helper macros                                                      */
/******************************************************************************/

/* Protected space actual size in bytes */
#define GPT_PPS_ACTUAL_SIZE(_t)	(1UL << (unsigned int)(_t))

/* Granule actual size in bytes */
#define GPT_PGS_ACTUAL_SIZE(_p)	(1UL << (unsigned int)(_p))

/* Number of granules in 2MB block */
#define GPT_PGS_COUNT_2MB(_p)	(1UL << (21U - (unsigned int)(_p)))

/* L0 GPT region size in bytes */
#define GPT_L0GPTSZ_ACTUAL_SIZE	(1UL << GPT_S_VAL)

/* Get the index of the L0 entry from a physical address */
#define GPT_L0_IDX(_pa)		((_pa) >> GPT_L0_IDX_SHIFT)

/*
 * This definition is used to determine if a physical address lies on an L0
 * region boundary.
 */
#define GPT_IS_L0_ALIGNED(_pa)	\
	(((_pa) & (GPT_L0_REGION_SIZE - UL(1))) == UL(0))

/* Get the type field from an L0 descriptor */
#define GPT_L0_TYPE(_desc)	(((_desc) >> GPT_L0_TYPE_SHIFT) & \
				GPT_L0_TYPE_MASK)

/* Create an L0 block descriptor */
#define GPT_L0_BLK_DESC(_gpi)	(GPT_L0_TYPE_BLK_DESC | \
				(((_gpi) & GPT_L0_BLK_DESC_GPI_MASK) << \
				GPT_L0_BLK_DESC_GPI_SHIFT))

/* Create an L0 table descriptor with an L1 table address */
#define GPT_L0_TBL_DESC(_pa)	(GPT_L0_TYPE_TBL_DESC | ((uint64_t)(_pa) & \
				(GPT_L0_TBL_DESC_L1ADDR_MASK << \
				GPT_L0_TBL_DESC_L1ADDR_SHIFT)))

/* Get the GPI from an L0 block descriptor */
#define GPT_L0_BLKD_GPI(_desc)	(((_desc) >> GPT_L0_BLK_DESC_GPI_SHIFT) & \
				GPT_L0_BLK_DESC_GPI_MASK)

/* Get the L1 address from an L0 table descriptor */
#define GPT_L0_TBLD_ADDR(_desc)	((uint64_t *)(((_desc) & \
				(GPT_L0_TBL_DESC_L1ADDR_MASK << \
				GPT_L0_TBL_DESC_L1ADDR_SHIFT))))

/* Get the GPI from L1 Contiguous descriptor */
#define GPT_L1_CONT_GPI(_desc)		\
	(((_desc) >> GPT_L1_CONT_DESC_GPI_SHIFT) & GPT_L1_CONT_DESC_GPI_MASK)

/* Get the GPI from L1 Granules descriptor */
#define GPT_L1_GRAN_GPI(_desc)	((_desc) & GPT_L1_GRAN_DESC_GPI_MASK)

/* Get the Contig from L1 Contiguous descriptor */
#define GPT_L1_CONT_CONTIG(_desc)	\
	(((_desc) >> GPT_L1_CONT_DESC_CONTIG_SHIFT) & \
					GPT_L1_CONT_DESC_CONTIG_MASK)

/* Get the index into the L1 table from a physical address */
#define GPT_L1_IDX(_p, _pa)		\
	(((_pa) >> GPT_L1_IDX_SHIFT(_p)) & GPT_L1_IDX_MASK(_p))

/* Get the index of the GPI within an L1 table entry from a physical address */
#define GPT_L1_GPI_IDX(_p, _pa)		\
	(((_pa) >> GPT_L1_GPI_IDX_SHIFT(_p)) & GPT_L1_GPI_IDX_MASK)

/* Determine if an address is granule-aligned */
#define GPT_IS_L1_ALIGNED(_p, _pa)	\
	(((_pa) & (GPT_PGS_ACTUAL_SIZE(_p) - UL(1))) == UL(0))

/* Get aligned addresses */
#define ALIGN_2MB(_addr)	((_addr) & ~(SZ_2M - 1UL))
#define ALIGN_32MB(_addr)	((_addr) & ~(SZ_32M - 1UL))
#define ALIGN_512MB(_addr)	((_addr) & ~(SZ_512M - 1UL))

/* Determine if region is contiguous */
#define GPT_REGION_IS_CONT(_len, _addr, _size)	\
	(((_len) >= (_size)) && (((_addr) & ((_size) - UL(1))) == UL(0)))

/* Get 32MB block number in 512MB block: 0-15 */
#define GET_32MB_NUM(_addr)	((_addr >> 25) & 0xF)

/* Get 2MB block number in 32MB block: 0-15 */
#define GET_2MB_NUM(_addr)	((_addr >> 21) & 0xF)

#endif /* GPT_RME_PRIVATE_H */
