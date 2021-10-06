/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_PAS_DEF_H
#define ARM_PAS_DEF_H

#include <lib/gpt_rme/gpt_rme.h>
#include <plat/arm/common/arm_def.h>

/*****************************************************************************
 * PAS regions used to initialize the Granule Protection Table (GPT)
 ****************************************************************************/

/*
 * The PA space is initially mapped in the GPT as follows:
 *
 * ============================================================================
 * Base Addr| Size        |L? GPT|PAS   |Content                 |Comment
 * ============================================================================
 * 0GB      | 1GB         |L0 GPT|ANY   |TBROM (EL3 code)        |Fixed mapping
 *          |             |      |      |TSRAM (EL3 data)        |
 *          |             |      |      |IO (incl.UARTs & GIC)   |
 * ----------------------------------------------------------------------------
 * 1GB      | 1GB         |L0 GPT|ANY   |IO                      |Fixed mapping
 * ----------------------------------------------------------------------------
 * 2GB      | 1GB         |L1 GPT|NS    |DRAM (NS Kernel)        |Use T.Descrip
 * ----------------------------------------------------------------------------
 * 3GB      |1GB-64MB     |L1 GPT|NS    |DRAM (NS Kernel)        |Use T.Descrip
 * ----------------------------------------------------------------------------
 * 4GB-64MB |64MB-32MB    |      |      |                        |
 *          | -4MB        |L1 GPT|SECURE|DRAM TZC                |Use T.Descrip
 * ----------------------------------------------------------------------------
 * 4GB-32MB |             |      |      |                        |
 * -3MB-1MB |32MB         |L1 GPT|REALM |RMM                     |Use T.Descrip
 * ----------------------------------------------------------------------------
 * 4GB-3MB  |             |      |      |                        |
 * -1MB     |3MB          |L1 GPT|ROOT  |EL3 DRAM data           |Use T.Descrip
 * ----------------------------------------------------------------------------
 * 4GB-1MB  |1MB          |L1 GPT|ROOT  |DRAM (L1 GPTs, SCP TZC) |Fixed mapping
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
#define ARM_PAS_1_SIZE			((ULL(1)<<31)) /* 2GB */

/* NS memory 2GB to (end - 64MB) */
#define ARM_PAS_2_BASE			(ARM_PAS_1_BASE + ARM_PAS_1_SIZE)
#define ARM_PAS_2_SIZE			(ARM_NS_DRAM1_SIZE)

/* Secure TZC region */
#define ARM_PAS_3_BASE			(ARM_AP_TZC_DRAM1_BASE)
#define ARM_PAS_3_SIZE			(ARM_AP_TZC_DRAM1_SIZE)

#define ARM_PAS_GPI_ANY			MAP_GPT_REGION(ARM_PAS_1_BASE, \
						       ARM_PAS_1_SIZE, \
						       GPT_GPI_ANY)
#define	ARM_PAS_KERNEL			GPT_MAP_REGION_GRANULE(ARM_PAS_2_BASE, \
							       ARM_PAS_2_SIZE, \
							       GPT_GPI_NS)

#define ARM_PAS_SECURE			GPT_MAP_REGION_GRANULE(ARM_PAS_3_BASE, \
							       ARM_PAS_3_SIZE, \
							       GPT_GPI_SECURE)

#define ARM_PAS_REALM			GPT_MAP_REGION_GRANULE(ARM_REALM_BASE, \
							       ARM_REALM_SIZE, \
							       GPT_GPI_REALM)

#define ARM_PAS_EL3_DRAM		GPT_MAP_REGION_GRANULE(ARM_EL3_TZC_DRAM1_BASE, \
							       ARM_EL3_TZC_DRAM1_SIZE, \
							       GPT_GPI_ROOT)

#define	ARM_PAS_GPTS			GPT_MAP_REGION_GRANULE(ARM_L1_GPT_ADDR_BASE, \
							       ARM_L1_GPT_SIZE, \
							       GPT_GPI_ROOT)

/* GPT Configuration options */
#define PLATFORM_L0GPTSZ		GPCCR_L0GPTSZ_30BITS

#endif /* ARM_PAS_DEF_H */
