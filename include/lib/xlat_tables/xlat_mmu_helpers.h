/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_MMU_HELPERS_H__
#define __XLAT_MMU_HELPERS_H__

/*
 * The following flags are passed to enable_mmu_xxx() to override the default
 * values used to program system registers while enabling the MMU.
 */

/*
 * When this flag is used, all data access to Normal memory from this EL and all
 * Normal memory accesses to the translation tables of this EL are non-cacheable
 * for all levels of data and unified cache until the caches are enabled by
 * setting the bit SCTLR_ELx.C.
 */
#define DISABLE_DCACHE			(U(1) << 0)

/*
 * Mark the translation tables as non-cacheable for the MMU table walker, which
 * is a different observer from the PE/CPU. If the flag is not specified, the
 * tables are cacheable for the MMU table walker.
 *
 * Note that, as far as the PE/CPU observer is concerned, the attributes used
 * are the ones specified in the translation tables themselves. The MAIR
 * register specifies the cacheability through the field AttrIndx of the lower
 * attributes of the translation tables. The shareability is specified in the SH
 * field of the lower attributes.
 *
 * The MMU table walker uses the attributes specified in the fields ORGNn, IRGNn
 * and SHn of the TCR register to access the translation tables.
 *
 * The attributes specified in the TCR register and the tables can be different
 * as there are no checks to prevent that. Special care must be taken to ensure
 * that there aren't mismatches. The behaviour in that case is described in the
 * sections 'Mismatched memory attributes' in the ARMv8 ARM.
 */
#define XLAT_TABLE_NC			(U(1) << 1)

/* Offsets into mmu_cfg_params array. All parameters are 32 bits wide. */
#define MMU_CFG_MAIR0		0
#define MMU_CFG_TCR		1
#define MMU_CFG_TTBR0_LO	2
#define MMU_CFG_TTBR0_HI	3
#define MMU_CFG_PARAM_MAX	4

#ifndef __ASSEMBLY__

#include <sys/types.h>

/* Parameters of register values required when enabling MMU */
extern uint32_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

#ifdef AARCH32
/* AArch32 specific translation table API */
void enable_mmu_secure(unsigned int flags);

void enable_mmu_direct(unsigned int flags);
#else
/* AArch64 specific translation table APIs */
void enable_mmu_el1(unsigned int flags);
void enable_mmu_el3(unsigned int flags);

void enable_mmu_direct_el1(unsigned int flags);
void enable_mmu_direct_el3(unsigned int flags);
#endif /* AARCH32 */

int xlat_arch_is_granule_size_supported(size_t size);
size_t xlat_arch_get_max_supported_granule_size(void);

#endif /* __ASSEMBLY__ */

#endif /* __XLAT_MMU_HELPERS_H__ */
