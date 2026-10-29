/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RME_H
#define RME_H

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/utils_def.h>

/******************************************************************************/
/* GPT helper macros and definitions                                          */
/******************************************************************************/

/* GPT GPI definitions */
#define GPT_GPI_NO_ACCESS		U(0x0)
#define GPT_GPI_SA			U(0x4)
#define GPT_GPI_NSP			U(0x5)
#define GPT_GPI_SECURE			U(0x8)
#define GPT_GPI_NS			U(0x9)
#define GPT_GPI_ROOT			U(0xA)
#define GPT_GPI_REALM			U(0xB)
#define GPT_GPI_NSO			U(0xD)
#define GPT_GPI_ANY			U(0xF)
#define GPT_GPI_VAL_MASK		UL(0xF)

#define GPT_NSE_SECURE			U(0b00)
#define GPT_NSE_ROOT			U(0b01)
#define GPT_NSE_NS			U(0b10)
#define GPT_NSE_REALM			U(0b11)

#define GPT_NSE_SHIFT                   U(62)

#define GPT_NSE2_SA			U(0b001)
#define GPT_NSE2_NSP			U(0b101)

#define GPT_NSE2_SHIFT			U(61)

/******************************************************************************/
/* GPT register field definitions                                             */
/******************************************************************************/
/* NSO bit definitions */
#define GPCCR_NSO_SHIFT		U(19)
#define GPCCR_NSO_BIT		(ULL(1) << GPCCR_NSO_SHIFT)

/* SA bit definitions */
#define GPCCR_NSP_SHIFT		U(26)
#define GPCCR_NSP_BIT		(ULL(1) << GPCCR_NSP_SHIFT)

/* SA bit definitions */
#define GPCCR_SA_SHIFT		U(25)
#define GPCCR_SA_BIT		(ULL(1) << GPCCR_SA_SHIFT)

/*
 * Least significant address bits protected by each entry in level 0 GPT. This
 * field is read-only.
 */
#define GPCCR_L0GPTSZ_SHIFT	U(20)
#define GPCCR_L0GPTSZ_MASK	U(0xF)

typedef enum {
	GPCCR_L0GPTSZ_30BITS	= U(0x0),
	GPCCR_L0GPTSZ_34BITS	= U(0x4),
	GPCCR_L0GPTSZ_36BITS	= U(0x6),
	GPCCR_L0GPTSZ_39BITS	= U(0x9)
} gpccr_l0gptsz_e;

/* Granule protection check priority bit definitions */
#define GPCCR_GPCP_SHIFT	U(17)
#define GPCCR_GPCP_BIT		(ULL(1) << GPCCR_EL3_GPCP_SHIFT)

/* Granule protection check bit definitions */
#define GPCCR_GPC_SHIFT		U(16)
#define GPCCR_GPC_BIT		(ULL(1) << GPCCR_GPC_SHIFT)

/* Physical granule size bit definitions */
#define GPCCR_PGS_SHIFT		U(14)
#define GPCCR_PGS_WIDTH		U(2)
#define GPCCR_PGS_MASK		U(0x3)
#define SET_GPCCR_PGS(x)	(((x) & GPCCR_PGS_MASK) << GPCCR_PGS_SHIFT)

typedef enum {
	GPCCR_PGS_4K		= U(0x0),
	GPCCR_PGS_64K		= U(0x1),
	GPCCR_PGS_16K		= U(0x2)
} gpccr_pgs_e;

/* GPT fetch shareability attribute bit definitions */
#define GPCCR_SH_SHIFT		U(12)
#define GPCCR_SH_MASK		U(0x3)
#define SET_GPCCR_SH(x)		(((x) & GPCCR_SH_MASK) << GPCCR_SH_SHIFT)

typedef enum {
	GPCCR_SH_NS		= U(0x0),
	GPCCR_SH_OS		= U(0x2),
	GPCCR_SH_IS		= U(0x3)
} gpccr_sh_e;

/* GPT fetch outer cacheability attribute bit definitions */
#define GPCCR_ORGN_SHIFT	U(10)
#define GPCCR_ORGN_MASK		U(0x3)
#define SET_GPCCR_ORGN(x)	(((x) & GPCCR_ORGN_MASK) << GPCCR_ORGN_SHIFT)

typedef enum {
	GPCCR_ORGN_NC		= U(0x0),
	GPCCR_ORGN_WB_RA_WA	= U(0x1),
	GPCCR_ORGN_WT_RA_NWA	= U(0x2),
	GPCCR_ORGN_WB_RA_NWA	= U(0x3)
} gpccr_orgn_e;

/* GPT fetch inner cacheability attribute bit definitions */
#define GPCCR_IRGN_SHIFT	U(8)
#define GPCCR_IRGN_MASK		U(0x3)
#define SET_GPCCR_IRGN(x)	(((x) & GPCCR_IRGN_MASK) << GPCCR_IRGN_SHIFT)

typedef enum {
	GPCCR_IRGN_NC		= U(0x0),
	GPCCR_IRGN_WB_RA_WA	= U(0x1),
	GPCCR_IRGN_WT_RA_NWA	= U(0x2),
	GPCCR_IRGN_WB_RA_NWA	= U(0x3)
} gpccr_irgn_e;

/* Protected physical address size bit definitions */
#define GPCCR_PPS_SHIFT		U(0)
#define GPCCR_PPS_WIDTH		U(3)
#define GPCCR_PPS_MASK		U(0x7)
#define SET_GPCCR_PPS(x)	(((x) & GPCCR_PPS_MASK) << GPCCR_PPS_SHIFT)

typedef enum {
	GPCCR_PPS_4GB		= U(0x0),
	GPCCR_PPS_64GB		= U(0x1),
	GPCCR_PPS_1TB		= U(0x2),
	GPCCR_PPS_4TB		= U(0x3),
	GPCCR_PPS_16TB		= U(0x4),
	GPCCR_PPS_256TB		= U(0x5),
	GPCCR_PPS_4PB		= U(0x6)
} gpccr_pps_e;

/* Base Address for the GPT bit definitions */
#define GPTBR_BADDR_SHIFT	U(0)
#define GPTBR_BADDR_VAL_SHIFT	U(12)
#define GPTBR_BADDR_MASK	ULL(0xffffffffff)

/* Max valid value for PPS */
#define GPT_PPS_MAX			(6U)

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

/* Max valid value for PGS */
#define GPT_PGS_MAX			(2U)

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
#define GPT_L1_NSO_DESC		GPT_BUILD_L1_DESC(GPT_GPI_NSO)
#define GPT_L1_ROOT_DESC	GPT_BUILD_L1_DESC(GPT_GPI_ROOT)
#define GPT_L1_SA_DESC		GPT_BUILD_L1_DESC(GPT_GPI_SA)
#define GPT_L1_NSP_DESC		GPT_BUILD_L1_DESC(GPT_GPI_NSP)
#define GPT_L1_ANY_DESC		GPT_BUILD_L1_DESC(GPT_GPI_ANY)
#define GPT_L1_INVALID_DESC	UL(0xFFFFFFFFFFFFFFFF)

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

/******************************************************************************/
/* GPT platform configuration                                                 */
/******************************************************************************/

/* This value comes from GPCCR_EL3 so no externally supplied definition */
#define GPT_L0GPTSZ		((unsigned int)((read_gpccr_el3() >> \
				GPCCR_L0GPTSZ_SHIFT) & GPCCR_L0GPTSZ_MASK))

/* The "S" value is directly related to L0GPTSZ */
#define GPT_S_VAL		(GPT_L0GPTSZ + 30U)

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
/* GPT helper macros and definitions for pas mappings                         */
/******************************************************************************/

/*
 * Structure for specifying a mapping range and it's properties. This should not
 * be manually initialized, using the MAP_GPT_REGION_x macros is recommended as
 * to avoid potential incompatibilities in the future.
 */
typedef struct pas_region {
	uintptr_t	base_pa;	/* Base address for PAS. */
	size_t		size;		/* Size of the PAS. */
	unsigned int	attrs;		/* PAS GPI and entry type. */
} pas_region_t;

/* PAS attribute GPI definitions. */
#define GPT_PAS_ATTR_GPI_SHIFT		U(0)
#define GPT_PAS_ATTR_GPI_MASK		U(0xF)
#define GPT_PAS_ATTR_GPI(_attrs)	(((_attrs)			\
					>> GPT_PAS_ATTR_GPI_SHIFT)	\
					& GPT_PAS_ATTR_GPI_MASK)

/* PAS attribute mapping type definitions */
#define GPT_PAS_ATTR_MAP_TYPE_BLOCK	U(0x0)
#define GPT_PAS_ATTR_MAP_TYPE_GRANULE	U(0x1)
#define GPT_PAS_ATTR_MAP_TYPE_SHIFT	U(4)
#define GPT_PAS_ATTR_MAP_TYPE_MASK	U(0x1)
#define GPT_PAS_ATTR_MAP_TYPE(_attrs)	(((_attrs)			\
					>> GPT_PAS_ATTR_MAP_TYPE_SHIFT)	\
					& GPT_PAS_ATTR_MAP_TYPE_MASK)

/*
 * Macro to initialize the attributes field in the pas_region_t structure.
 * [31:5] Reserved
 * [4]    Mapping type (GPT_PAS_ATTR_MAP_TYPE_x definitions)
 * [3:0]  PAS GPI type (GPT_GPI_x definitions)
 */
#define GPT_PAS_ATTR(_type, _gpi)					\
	((((_type) & GPT_PAS_ATTR_MAP_TYPE_MASK)			\
	  << GPT_PAS_ATTR_MAP_TYPE_SHIFT) |				\
	(((_gpi) & GPT_PAS_ATTR_GPI_MASK)				\
	 << GPT_PAS_ATTR_GPI_SHIFT))

/*
 * Macro to create a GPT entry for this PAS range as a block descriptor. If this
 * region does not fit the requirements for a block descriptor then GPT
 * initialization will fail.
 */
#define GPT_MAP_REGION_BLOCK(_pa, _sz, _gpi)				\
	{								\
		.base_pa = (_pa),					\
		.size = (_sz),						\
		.attrs = GPT_PAS_ATTR(GPT_PAS_ATTR_MAP_TYPE_BLOCK, (_gpi)), \
	}

/*
 * Macro to create a GPT entry for this PAS range as a table descriptor. If this
 * region does not fit the requirements for a table descriptor then GPT
 * initialization will fail.
 */
#define GPT_MAP_REGION_GRANULE(_pa, _sz, _gpi)				\
	{								\
		.base_pa = (_pa),					\
		.size = (_sz),						\
		.attrs = GPT_PAS_ATTR(GPT_PAS_ATTR_MAP_TYPE_GRANULE, (_gpi)), \
	}

/******************************************************************************/
/* RME public APIs to initialize L0, L1 tables and enable GPC                 */
/******************************************************************************/

/*
 * Public API that initializes the entire protected space to GPT_GPI_ANY using
 * the L0 tables (block descriptors).  Ideally, this function is invoked prior
 * to DDR discovery and initialization.  The MMU must be initialized before
 * calling this function.
 *
 * Parameters
 *   pps		PPS value to use for table generation
 *   l0_mem_base	Base address of L0 tables in memory.
 *   l0_mem_size	Total size of memory available for L0 tables.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int rme_init_gpt_l0_tables(gpccr_pps_e pps, uintptr_t l0_mem_base,
			   size_t l0_mem_size);

/*
 * Public API that carves out PAS regions from the L0 tables and builds any L1
 * tables that are needed.  This function ideally is run after DDR discovery and
 * initialization.  The L0 tables must have already been initialized to GPI_ANY
 * when this function is called.
 *
 * Parameters
 *   pgs		PGS value to use for table generation.
 *   l1_mem_base	Base address of memory used for L1 tables.
 *   l1_mem_size	Total size of memory available for L1 tables.
 *   *pas_regions	Pointer to PAS regions structure array.
 *   pas_count		Total number of PAS regions.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int rme_init_gpt_l1_tables(gpccr_pgs_e pgs, uintptr_t l1_mem_base,
			   size_t l1_mem_size, pas_region_t *pas_regions,
			   unsigned int pas_count);

/*
 * Public API to enable granule protection checks once the tables have all been
 * initialized.  This function is called at first initialization and then again
 * later during warm boots of CPU cores.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int rme_gpc_enable(void);

/*
 * Public API to initialize the runtime gpt_config structure based on the values
 * present in the GPTBR_EL3 and GPCCR_EL3 registers. GPT initialization
 * typically happens in a bootloader stage prior to setting up the EL3 runtime
 * environment for the granule transition service so this function detects the
 * initialization from a previous stage. Granule protection checks must be
 * enabled already or this function will return an error.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
#if ENABLE_FEAT_RME
int rme_sync_gpt_config_for_warmboot(void);
#else
static inline int rme_sync_gpt_config_for_warmboot(void)
{
	return -EPERM;
}
#endif
#endif /* RME_H */
