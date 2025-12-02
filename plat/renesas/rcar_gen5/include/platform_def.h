/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#ifndef __ASSEMBLER__
#include <stdlib.h>
#endif

#include <arch.h>

#include "rcar_def.h"

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE		0x800U

/*
 * R-Car X5H Cortex-A720AE
 * L1:I/64KB per core, D/64KB per core, L2:512KB L3:4MB per cluster
 */

/* Hardware environment: 8 clusters, 4 cores each */
#define PLATFORM_CLUSTER_COUNT		8
#define PLATFORM_CLUSTER0_CORE_COUNT	4
#define PLATFORM_CLUSTER1_CORE_COUNT	4
#define PLATFORM_CLUSTER2_CORE_COUNT	4
#define PLATFORM_CLUSTER3_CORE_COUNT	4
#define PLATFORM_CLUSTER4_CORE_COUNT	4
#define PLATFORM_CLUSTER5_CORE_COUNT	4
#define PLATFORM_CLUSTER6_CORE_COUNT	4
#define PLATFORM_CLUSTER7_CORE_COUNT	4
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER7_CORE_COUNT + \
					PLATFORM_CLUSTER6_CORE_COUNT + \
					PLATFORM_CLUSTER5_CORE_COUNT + \
					PLATFORM_CLUSTER4_CORE_COUNT + \
					PLATFORM_CLUSTER3_CORE_COUNT + \
					PLATFORM_CLUSTER2_CORE_COUNT + \
					PLATFORM_CLUSTER1_CORE_COUNT + \
					PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	4

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CORE_COUNT + \
					 PLATFORM_CLUSTER_COUNT + 1)

#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		2

/*
 *  Macros for local power states in ARM platforms encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define ARM_LOCAL_STATE_RUN		0
/* Local power state for retention. Valid only for CPU power domains */
#define ARM_LOCAL_STATE_RET		1
/* Local power state for OFF/power-down. Valid for CPU and cluster power domains */
#define ARM_LOCAL_STATE_OFF		2

/*
 ******************************************************************************
 * BL31 specific defines.
 ******************************************************************************
 * Put BL3-1 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL3-1 debug size plus a little space for growth.
 */
#define BL31_BASE   RCAR_TRUSTED_SRAM_BASE
#define BL31_LIMIT  (RCAR_TRUSTED_SRAM_BASE + RCAR_TRUSTED_SRAM_SIZE)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define MAX_XLAT_TABLES			8

#define PLAT_PHY_ADDR_SPACE_SIZE	BIT_64(33)
#define PLAT_VIRT_ADDR_SPACE_SIZE	BIT_64(33)

#define MAX_MMAP_REGIONS		(RCAR_MMAP_ENTRIES + RCAR_BL_REGIONS)

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two mailboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT	6
#define CACHE_WRITEBACK_GRANULE	BIT(CACHE_WRITEBACK_SHIFT)

/*
 * Define a list of Group 1 Secure and Group 0 interrupt properties as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 *
 * PLAT_ARM_G0_IRQ_PROPS(grp) is not defined, because there are no Group 0 IRQs.
 */
#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE)

#endif /* PLATFORM_DEF_H */
