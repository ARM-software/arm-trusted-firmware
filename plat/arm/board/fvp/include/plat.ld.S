/*
 * Copyright (c) 2017-2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_LD_S
#define PLAT_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>

MEMORY {
    EL3_SEC_DRAM (rw): ORIGIN = ARM_EL3_TZC_DRAM1_BASE, LENGTH = ARM_EL3_TZC_DRAM1_SIZE
}

SECTIONS
{
	. = ARM_EL3_TZC_DRAM1_BASE;
	ASSERT(. == ALIGN(PAGE_SIZE),
	"ARM_EL3_TZC_DRAM_BASE address is not aligned on a page boundary.")
	.el3_tzc_dram (NOLOAD) : ALIGN(PAGE_SIZE) {
	__PLAT_SPMC_SHMEM_DATASTORE_START__ = .;
	*(.arm_spmc_shmem_datastore)
	__PLAT_SPMC_SHMEM_DATASTORE_END__ = .;
	__EL3_SEC_DRAM_START__ = .;
	*(.arm_el3_tzc_dram)
#if SEPARATE_SIMD_SECTION
	. = ALIGN(16);
	*(.simd_context)
#endif
	__EL3_SEC_DRAM_UNALIGNED_END__ = .;

	. = ALIGN(PAGE_SIZE);
	__EL3_SEC_DRAM_END__ = .;
	} >EL3_SEC_DRAM
}

#if RECLAIM_INIT_CODE
#include <plat/arm/common/arm_reclaim_init.ld.S>
#endif /* RECLAIM_INIT_CODE */

#endif /* PLAT_LD_S */
