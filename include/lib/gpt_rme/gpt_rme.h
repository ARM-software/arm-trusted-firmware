/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPT_RME_H
#define GPT_RME_H

#include <stdint.h>

#include <arch.h>

/******************************************************************************/
/* GPT helper macros and definitions                                          */
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

/* GPT GPI definitions */
#define GPT_GPI_NO_ACCESS		U(0x0)
#define GPT_GPI_SECURE			U(0x8)
#define GPT_GPI_NS			U(0x9)
#define GPT_GPI_ROOT			U(0xA)
#define GPT_GPI_REALM			U(0xB)
#define GPT_GPI_ANY			U(0xF)
#define GPT_GPI_VAL_MASK		UL(0xF)

#define GPT_NSE_SECURE			U(0b00)
#define GPT_NSE_ROOT			U(0b01)
#define GPT_NSE_NS			U(0b10)
#define GPT_NSE_REALM			U(0b11)

#define GPT_NSE_SHIFT                   U(62)

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
/* GPT register field definitions                                             */
/******************************************************************************/

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

/******************************************************************************/
/* GPT public APIs                                                            */
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
int gpt_init_l0_tables(gpccr_pps_e pps,
		       uintptr_t l0_mem_base,
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
int gpt_init_pas_l1_tables(gpccr_pgs_e pgs,
			   uintptr_t l1_mem_base,
			   size_t l1_mem_size,
			   pas_region_t *pas_regions,
			   unsigned int pas_count);

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
int gpt_runtime_init(void);

/*
 * Public API to enable granule protection checks once the tables have all been
 * initialized.  This function is called at first initialization and then again
 * later during warm boots of CPU cores.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_enable(void);

/*
 * Public API to disable granule protection checks.
 */
void gpt_disable(void);

/*
 * This function is the core of the granule transition service. When a granule
 * transition request occurs it is routed to this function where the request is
 * validated then fulfilled if possible.
 *
 * TODO: implement support for transitioning multiple granules at once.
 *
 * Parameters
 *   base: Base address of the region to transition, must be aligned to granule
 *         size.
 *   size: Size of region to transition, must be aligned to granule size.
 *   src_sec_state: Security state of the originating SMC invoking the API.
 *
 * Return
 *    Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_delegate_pas(uint64_t base, size_t size, unsigned int src_sec_state);
int gpt_undelegate_pas(uint64_t base, size_t size, unsigned int src_sec_state);

#endif /* GPT_RME_H */
