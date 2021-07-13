/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPT_H
#define GPT_H

#include <stdint.h>

#include <arch.h>

#include "gpt_defs.h"

#define GPT_DESC_ATTRS(_type, _gpi)		\
	((((_type) & PAS_REG_DESC_TYPE_MASK)	\
	  << PAS_REG_DESC_TYPE_SHIFT) |		\
	(((_gpi) & PAS_REG_GPI_MASK)		\
	 << PAS_REG_GPI_SHIFT))

/*
 * Macro to create a GPT entry for this PAS range either as a L0 block
 * descriptor or L1 table descriptor depending upon the size of the range.
 */
#define MAP_GPT_REGION(_pa, _sz, _gpi)					\
	{								\
		.base_pa = (_pa),					\
		.size = (_sz),						\
		.attrs = GPT_DESC_ATTRS(PAS_REG_DESC_TYPE_ANY, (_gpi)),	\
	}

/*
 * Special macro to create a L1 table descriptor at L0 for a 1GB region as
 * opposed to creating a block mapping by default.
 */
#define MAP_GPT_REGION_TBL(_pa, _sz, _gpi)				\
	{								\
		.base_pa = (_pa),					\
		.size = (_sz),						\
		.attrs = GPT_DESC_ATTRS(PAS_REG_DESC_TYPE_TBL, (_gpi)),	\
	}

/*
 * Structure for specifying a Granule range and its properties
 */
typedef struct pas_region {
	unsigned long long	base_pa;	/**< Base address for PAS. */
	size_t			size;		/**< Size of the PAS. */
	unsigned int		attrs;		/**< PAS GPI and entry type. */
} pas_region_t;

/*
 * Structure to initialise the Granule Protection Tables.
 */
typedef struct gpt_init_params {
	unsigned int pgs;	/**< Address Width of Phisical Granule Size. */
	unsigned int pps;	/**< Protected Physical Address Size.	     */
	unsigned int l0gptsz;	/**< Granule size on L0 table entry.	     */
	pas_region_t *pas_regions; /**< PAS regions to protect.		     */
	unsigned int pas_count;	/**< Number of PAS regions to initialise.    */
	uintptr_t l0_mem_base;	/**< L0 Table base address.		     */
	size_t l0_mem_size;	/**< Size of memory reserved for L0 tables.  */
	uintptr_t l1_mem_base;	/**< L1 Table base address.		     */
	size_t l1_mem_size;	/**< Size of memory reserved for L1 tables.  */
} gpt_init_params_t;

/** @brief Initialise the Granule Protection tables.
 */
int gpt_init(gpt_init_params_t *params);

/** @brief Enable the Granule Protection Checks.
 */
void gpt_enable(void);

/** @brief Disable the Granule Protection Checks.
 */
void gpt_disable(void);

/** @brief Transition a granule between security states.
 */
int gpt_transition_pas(uint64_t pa,
			unsigned int src_sec_state,
			unsigned int target_pas);

#endif /* GPT_H */
