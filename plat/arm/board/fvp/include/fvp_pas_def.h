/*
 * Copyright (c) 2021-2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FVP_PAS_DEF_H
#define FVP_PAS_DEF_H

#include <lib/gpt_rme/gpt_rme.h>
#include <platform_def.h>

/*****************************************************************************
 * PAS regions used to initialize the Granule Protection Table (GPT)
 ****************************************************************************/

/*
 * The PA space is initially mapped in the GPT as follows:
 *
 * ============================================================================
 * Base Addr | Size        |L? GPT|PAS   |Content                 |Comment
 * ============================================================================
 * 0GB       | 1GB         |L0 GPT|ANY   |TBROM (EL3 code)        |Fixed mapping
 *           |             |      |      |TSRAM (EL3 data)        |
 * 00000000  | 40000000    |      |      |IO (incl.UARTs & GIC)   |
 * ----------------------------------------------------------------------------
 * 1GB       | 1GB         |L0 GPT|ANY   |IO                      |Fixed mapping
 * 40000000  | 40000000    |      |      |                        |
 * ----------------------------------------------------------------------------
 * 1GB+256MB | 256MB       |L1 GPT|NS    |PCI Memory Region 1     |Use T.Descrip
 * 50000000  | 10000000    |      |      |                        |
 * ----------------------------------------------------------------------------
 * 2GB       | 2GB-64MB    |L1 GPT|NS    |DRAM (NS Kernel)        |Use T.Descrip
 * 80000000  | 7C000000    |      |      |                        |
 * ----------------------------------------------------------------------------
 * 4GB-64MB  |64MB-32MB-4MB|L1 GPT|SECURE|DRAM TZC                |Use T.Descrip
 * FC000000  | 1C00000     |      |      |                        |
 * ----------------------------------------------------------------------------
 * 4GB-32MB  |             |      |      |                        |
 * -3MB-1MB  | 32MB        |L1 GPT|REALM |RMM                     |Use T.Descrip
 * FDC00000  | 2000000     |      |      |                        |
 * ----------------------------------------------------------------------------
 * 4GB-3MB   |             |      |      |                        |
 * -1MB      | 4MB         |L1 GPT|ROOT  |EL3 DRAM data, L1 GPTs, |Use T.Descrip
 * FFC00000  | 400000      |      |      |SCP TZC                 |
 * ----------------------------------------------------------------------------
 * 34GB      | 2GB         |L1 GPT|NS    |DRAM (NS Kernel)        |Use T.Descrip
 * 880000000 | 80000000    |      |      |                        |
 * ----------------------------------------------------------------------------
 * 256GB     | 3GB         |L1 GPT|NS    |PCI Memory Region 2     |Use T.Descrip
 * 4000000000| C0000000    |      |      |(first 3GB only)        |
 * ============================================================================
 *
 * - 4KB of L0 GPT reside in TSRAM, on top of the CONFIG section.
 * - ~1MB of L1 GPTs reside at the top of DRAM1 (TZC area).
 * - The first 1GB region has GPT_GPI_ANY and, therefore, is not protected by
 *   the GPT.
 * - The DRAM TZC area is split into three regions: the L1 GPT region and
 *   3MB of region below that are defined as GPT_GPI_ROOT, 32MB Realm region
 *   below that is defined as GPT_GPI_REALM and the rest of it is defined as
 *   GPT_GPI_SECURE.
 */

/* TODO: This might not be the best way to map the PAS */

/* Device memory 0 to 2GB */
#define ARM_PAS_1_BASE			(U(0))
#define ARM_PAS_1_SIZE			(SZ_2G) /* 2GB */

/* NS memory 2GB to (end - 64MB) */
#define ARM_PAS_2_BASE			(ARM_PAS_1_BASE + ARM_PAS_1_SIZE)
#define ARM_PAS_2_SIZE			(ARM_NS_DRAM1_SIZE)

/* Shared area between EL3 and RMM */
#define ARM_PAS_SHARED_BASE		(ARM_EL3_RMM_SHARED_BASE)
#define ARM_PAS_SHARED_SIZE		(ARM_EL3_RMM_SHARED_SIZE)

/* Secure TZC region */
#define ARM_PAS_3_BASE			(ARM_AP_TZC_DRAM1_BASE)
#define ARM_PAS_3_SIZE			(ARM_AP_TZC_DRAM1_SIZE)

/* NS memory 2GB */
#define	ARM_PAS_4_BASE			ARM_DRAM2_BASE
#define	ARM_PAS_4_SIZE			(SZ_2G)	/* 2GB */

#define	ARM_PAS_KERNEL			GPT_MAP_REGION_GRANULE(ARM_PAS_2_BASE, \
							       ARM_PAS_2_SIZE, \
							       GPT_GPI_NS)

#define ARM_PAS_SECURE			GPT_MAP_REGION_GRANULE(ARM_PAS_3_BASE, \
							       ARM_PAS_3_SIZE, \
							       GPT_GPI_SECURE)

#define	ARM_PAS_KERNEL_1		GPT_MAP_REGION_GRANULE(ARM_PAS_4_BASE, \
							       ARM_PAS_4_SIZE, \
							       GPT_GPI_NS)

#define ARM_PAS_PCI_MEM_1		GPT_MAP_REGION_GRANULE(PLAT_ARM_PCI_MEM_1_BASE, \
							       PLAT_ARM_PCI_MEM_1_SIZE, \
							       GPT_GPI_NS)

#define	ARM_PAS_PCI_MEM_2		GPT_MAP_REGION_GRANULE(PLAT_ARM_PCI_MEM_2_BASE, \
							       PLAT_ARM_PCI_MEM_2_SIZE, \
							       GPT_GPI_NS)
/*
 * REALM and Shared area share the same PAS, so consider them a single
 * PAS region to configure in GPT.
 */
#define ARM_PAS_REALM			GPT_MAP_REGION_GRANULE(ARM_REALM_BASE, \
							       (ARM_PAS_SHARED_SIZE + \
								ARM_REALM_SIZE), \
							       GPT_GPI_REALM)
/* Check if the EL3 TZC DRAM is contiguous with L1 GPT region. */
#if (ARM_L1_GPT_BASE != (ARM_EL3_TZC_DRAM1_BASE + ARM_EL3_TZC_DRAM1_SIZE))
#define ARM_PAS_EL3_DRAM		GPT_MAP_REGION_GRANULE(ARM_EL3_TZC_DRAM1_BASE, \
							       ARM_EL3_TZC_DRAM1_SIZE, \
							       GPT_GPI_ROOT)

#define	ARM_PAS_GPTS			GPT_MAP_REGION_GRANULE(ARM_L1_GPT_BASE, \
							       ARM_L1_GPT_SIZE, \
							       GPT_GPI_ROOT)
#else
/* Contiguous ROOT region */
#define ARM_PAS_EL3_DRAM		GPT_MAP_REGION_GRANULE(ARM_EL3_TZC_DRAM1_BASE,	\
							       ARM_EL3_TZC_DRAM1_SIZE +	\
							       ARM_L1_GPT_SIZE, \
							       GPT_GPI_ROOT)
#endif

/* GPT Configuration options */
#define PLATFORM_L0GPTSZ		GPCCR_L0GPTSZ_30BITS

#endif /* FVP_PAS_DEF_H */
