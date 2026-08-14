/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPT_RME_H
#define GPT_RME_H

#include <stdint.h>
#include <lib/extensions/rme.h>
#include <lib/gpt_rme/gpc_fault.h>
#include <lib/spinlock.h>

/******************************************************************************/
/* GPT helper macros and definitions for locks                                */
/******************************************************************************/

#if (RME_GPT_BITLOCK_BLOCK != 0)
#define LOCK_SIZE	sizeof(((bitlock_t *)NULL)->lock)
#define LOCK_TYPE	typeof(((bitlock_t *)NULL)->lock)
#define LOCK_BITS	(LOCK_SIZE * UL(8))

CASSERT((UL(1) == LOCK_SIZE), assert_bitlock_type_not_uint8_t);
#endif /* RME_GPT_BITLOCK_BLOCK */

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
 * Parameters
 *   l1_bitlocks_base	Base address of memory for L1 tables bitlocks.
 *   l1_bitlocks_size	Total size of memory available for L1 tables bitlocks.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
#if ENABLE_FEAT_RME
int gpt_runtime_init(uintptr_t l1_bitlocks_base, size_t l1_bitlocks_size);
#else
static inline int gpt_runtime_init(uintptr_t l1_bitlocks_base, size_t l1_bitlocks_size)
{
	return -1;
}
#endif

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
 * This function is the core of the granule transition service. When a granule
 * transition request occurs it is routed to this function where the request is
 * validated then fulfilled if possible.
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

#if FIRME_SUPPORT
/*
 * The existing bitlocks are used by FIRME to synchorize GPT updates. This
 * will removed once FIRME supports native locks on PA range.
 */
void firme_gpt_lock(uint64_t base);
void firme_gpt_unlock(uint64_t base);
#endif /* FIRME_SUPPORT */
#endif /* GPT_RME_H */
