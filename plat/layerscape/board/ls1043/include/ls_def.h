/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LS_DEF_H
#define LS_DEF_H

#include <platform_def.h>

#include <arch.h>
#include <common/tbbr/tbbr_img_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

/******************************************************************************
 * Definitions common to all ARM standard platforms
 *****************************************************************************/
/* Special value used to verify platform parameters from BL2 to BL31 */
#define LS_BL31_PLAT_PARAM_VAL		0x0f1e2d3c4b5a6978ULL

#define LS_CACHE_WRITEBACK_SHIFT	6

/*
 * Macros mapping the MPIDR Affinity levels to Layerscape Platform Power levels. The
 * power levels have a 1:1 mapping with the MPIDR affinity levels.
 */
#define LS_PWR_LVL0		MPIDR_AFFLVL0
#define LS_PWR_LVL1		MPIDR_AFFLVL1
#define LS_PWR_LVL2		MPIDR_AFFLVL2

/*
 *  Macros for local power states in Layerscape platforms encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define LS_LOCAL_STATE_RUN	U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define LS_LOCAL_STATE_RET	U(1)
/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains
 */
#define LS_LOCAL_STATE_OFF	U(2)

#define LS_MAP_NS_DRAM		MAP_REGION_FLAT( \
					(LS_NS_DRAM_BASE), \
					LS_DRAM1_SIZE, \
					MT_DEVICE | MT_RW | MT_NS)

#define LS_MAP_TSP_SEC_MEM	MAP_REGION_FLAT( \
					TSP_SEC_MEM_BASE, \
					TSP_SEC_MEM_SIZE, \
				MT_DEVICE | MT_RW | MT_SECURE)


#define LS_MAP_FLASH0_RW	MAP_REGION_FLAT(PLAT_LS_FLASH_BASE,\
					PLAT_LS_FLASH_SIZE, \
					MT_DEVICE | MT_RW)

#define LS_MAP_CCSR		MAP_REGION_FLAT(PLAT_LS_CCSR_BASE, \
					PLAT_LS_CCSR_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)


#define LS_MAP_CONSOLE		MAP_REGION_FLAT(PLAT_LS1043_DUART1_BASE, \
					PLAT_LS1043_DUART_SIZE, \
					MT_DEVICE | MT_RW | MT_NS)

/*
 * The number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
/******************************************************************************
 * Required platform porting definitions common to all ARM standard platforms
 *****************************************************************************/

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 32)

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		LS_LOCAL_STATE_RET

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		LS_LOCAL_STATE_OFF

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE		(1 << LS_CACHE_WRITEBACK_SHIFT)

/*
 * One cache line needed for bakery locks on Layerscape platforms
 */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE		(1 * CACHE_WRITEBACK_GRANULE)

#endif /* LS_DEF_H */
