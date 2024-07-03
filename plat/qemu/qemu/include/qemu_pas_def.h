/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef QEMU_PAS_DEF_H
#define QEMU_PAS_DEF_H

#include <lib/gpt_rme/gpt_rme.h>
#include "platform_def.h"

/*****************************************************************************
 * PAS regions used to initialize the Granule Protection Table (GPT)
 ****************************************************************************/

/*
 * The PA space is initially mapped in the GPT as follows:
 *
 * ===========================================================================
 * Base Addr   | Size    |L? GPT|PAS   |Content          |Comment
 * ===========================================================================
 *             | 1GB     |L0 GPT|ANY   |Flash            |
 *    00000000 |         |      |      |IO               |
 * ---------------------------------------------------------------------------
 *       224MB | 1KB     |L1 GPT|ANY   |Secure RAM (EL3) |
 *    0e000000 |         |      |      |  (shared)       |
 * ---------------------------------------------------------------------------
 *             | 1MB-1KB |L1 GPT|ROOT  |Secure RAM (EL3) |
 *    0e001000 |         |      |      |                 |
 * ---------------------------------------------------------------------------
 *       225MB | 14MB    |L1 GPT|SECURE|Secure RAM       |
 *    0e100000 |         |      |      |  (EL2, EL1)     |
 * ---------------------------------------------------------------------------
 *             | 2MB     |L1 GPT|ROOT  |L0 and L1 GPTs,  |
 *    0edfc000 |  +16KB  |      |      | bitlocks        |
 * ---------------------------------------------------------------------------
 *       240MB | 800MB   |L1 GPT|ANY   |IO               |
 *    0f000000 |         |      |      |                 |
 * ---------------------------------------------------------------------------
 *         1GB | 1MB     |L1 GPT|NS    |DRAM             |
 *    40000000 |         |      |      | (device tree)   |
 * ---------------------------------------------------------------------------
 *     1GB+1MB | 24MB    |L1 GPT|REALM |DRAM (RMM)       |
 *    40100000 |         |      |      |                 |
 * ---------------------------------------------------------------------------
 *    1GB+25MB | 3GB     |L1 GPT|NS    |DRAM (kernel)    | Limit set by
 *    41900000 |         |      |      |                 |  NS_DRAM0_SIZE
 * ---------------------------------------------------------------------------
 *       256GB | 512+GB  |L0 GPT|ANY   |IO               | Floating. Higher
 * 40000000000 |         |      |      |                 |  when RAM>256GB
 * ----------------------------------------------------------------------------
 */

/* EL3 SRAM */
#define QEMU_PAS_ROOT_BASE		BL_RAM_BASE
#define QEMU_PAS_ROOT_SIZE		BL_RAM_SIZE

/* Secure DRAM */
#define QEMU_PAS_SEC_BASE		SEC_DRAM_BASE
#define QEMU_PAS_SEC_SIZE		(SEC_DRAM_SIZE - RME_GPT_DRAM_SIZE)

/* GPTs */
#define QEMU_PAS_GPT_BASE		RME_GPT_DRAM_BASE
#define QEMU_PAS_GPT_SIZE		RME_GPT_DRAM_SIZE

/* RMM */
#define QEMU_PAS_RMM_BASE		RMM_BASE
#define QEMU_PAS_RMM_SIZE		PLAT_QEMU_RMM_SIZE

/* Shared area between EL3 and RMM */
#define QEMU_PAS_RMM_SHARED_BASE	RMM_SHARED_BASE
#define QEMU_PAS_RMM_SHARED_SIZE	RMM_SHARED_SIZE

#define QEMU_PAS_NS0_BASE		NS_DRAM0_BASE
#define QEMU_PAS_NS0_SIZE		PLAT_QEMU_DT_MAX_SIZE
#define QEMU_PAS_NS1_BASE		(REALM_DRAM_BASE + REALM_DRAM_SIZE)
#define QEMU_PAS_NS1_SIZE		(NS_DRAM0_SIZE - \
					 (QEMU_PAS_NS0_SIZE + REALM_DRAM_SIZE))

#define QEMU_PAS_ROOT			GPT_MAP_REGION_GRANULE(QEMU_PAS_ROOT_BASE, \
							       QEMU_PAS_ROOT_SIZE, \
							       GPT_GPI_ROOT)

#define QEMU_PAS_SECURE			GPT_MAP_REGION_GRANULE(QEMU_PAS_SEC_BASE, \
							       QEMU_PAS_SEC_SIZE, \
							       GPT_GPI_SECURE)

#define QEMU_PAS_GPTS			GPT_MAP_REGION_GRANULE(QEMU_PAS_GPT_BASE, \
							       QEMU_PAS_GPT_SIZE, \
							       GPT_GPI_ROOT)

#define QEMU_PAS_NS0			GPT_MAP_REGION_GRANULE(QEMU_PAS_NS0_BASE, \
							       QEMU_PAS_NS0_SIZE, \
							       GPT_GPI_NS)

#define QEMU_PAS_NS1			GPT_MAP_REGION_GRANULE(QEMU_PAS_NS1_BASE, \
							       QEMU_PAS_NS1_SIZE, \
							       GPT_GPI_NS)

#define QEMU_PAS_REALM			GPT_MAP_REGION_GRANULE(QEMU_PAS_RMM_BASE, \
							       QEMU_PAS_RMM_SIZE + \
							       QEMU_PAS_RMM_SHARED_SIZE, \
							       GPT_GPI_REALM)

/* GPT Configuration options */
#define PLATFORM_L0GPTSZ		GPCCR_L0GPTSZ_30BITS

#endif /* QEMU_PAS_DEF_H */
