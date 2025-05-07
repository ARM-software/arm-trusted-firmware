/*
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <plat_common.h>
#include "versal_def.h"

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#ifndef PLATFORM_STACK_SIZE
#define PLATFORM_STACK_SIZE	U(0x440)
#endif

#define PLATFORM_CORE_COUNT		U(2)
#define PLAT_MAX_PWR_LVL		U(1)
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL31 debug size plus a
 * little space for growth.
 */
#ifndef VERSAL_ATF_MEM_BASE
# define BL31_BASE			U(0xfffe0000)
# define BL31_LIMIT			U(0x100000000)
#else
# define BL31_BASE			U(VERSAL_ATF_MEM_BASE)
# define BL31_LIMIT			U(VERSAL_ATF_MEM_BASE + VERSAL_ATF_MEM_SIZE)
# ifdef VERSAL_ATF_MEM_PROGBITS_SIZE
#  define BL31_PROGBITS_LIMIT		U(VERSAL_ATF_MEM_BASE + VERSAL_ATF_MEM_PROGBITS_SIZE)
# endif
#endif

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
#ifndef VERSAL_BL32_MEM_BASE
# define BL32_BASE			U(0x60000000)
# define BL32_LIMIT			U(0x80000000)
#else
# define BL32_BASE			U(VERSAL_BL32_MEM_BASE)
# define BL32_LIMIT			U(VERSAL_BL32_MEM_BASE + VERSAL_BL32_MEM_SIZE)
#endif

/*******************************************************************************
 * BL33 specific defines.
 ******************************************************************************/
#ifndef PRELOADED_BL33_BASE
# define PLAT_ARM_NS_IMAGE_BASE		U(0x8000000)
#else
# define PLAT_ARM_NS_IMAGE_BASE		U(PRELOADED_BL33_BASE)
#endif

/*******************************************************************************
 * HIGH and LOW DDR MAX definitions
 ******************************************************************************/
#define PLAT_DDR_LOWMEM_MAX		U(0x80000000)
#define PLAT_DDR_HIGHMEM_MAX		U(0x100000000)

/*******************************************************************************
 * TSP  specific defines.
 ******************************************************************************/
#define TSP_SEC_MEM_BASE		BL32_BASE
#define TSP_SEC_MEM_SIZE		(BL32_LIMIT - BL32_BASE)

/* ID of the secure physical generic timer interrupt used by the TSP */
#define TSP_IRQ_SEC_PHY_TIMER		ARM_IRQ_SEC_PHY_TIMER

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/

#if (BL31_BASE >= (1ULL << 32U))
/* Address range in High DDR and HBM memory range */
#define PLAT_ADDR_SPACE_SHIFT		U(42)
#else
/* Address range in OCM and Low DDR memory range */
#define PLAT_ADDR_SPACE_SHIFT		U(32)
#endif

#define PLAT_PHY_ADDR_SPACE_SIZE        (1ull << PLAT_ADDR_SPACE_SHIFT)
#define PLAT_VIRT_ADDR_SPACE_SIZE       (1ull << PLAT_ADDR_SPACE_SHIFT)

#define XILINX_OF_BOARD_DTB_MAX_SIZE	U(0x200000)

#define PLAT_OCM_BASE			U(0xFFFE0000)
#define PLAT_OCM_LIMIT			U(0xFFFFFFFF)

#define IS_TFA_IN_OCM(x)	((x >= PLAT_OCM_BASE) && (x < PLAT_OCM_LIMIT))

#ifndef MAX_MMAP_REGIONS
#if (defined(XILINX_OF_BOARD_DTB_ADDR) && !IS_TFA_IN_OCM(BL31_BASE))
#define MAX_MMAP_REGIONS		9
#else
#define MAX_MMAP_REGIONS		8
#endif
#endif

#ifndef MAX_XLAT_TABLES
#if !IS_TFA_IN_OCM(BL31_BASE)
#define MAX_XLAT_TABLES		9
#else
#define MAX_XLAT_TABLES		5
#endif
#endif

#define CACHE_WRITEBACK_SHIFT	6
#define CACHE_WRITEBACK_GRANULE	(1 << CACHE_WRITEBACK_SHIFT)

#define PLAT_ARM_GICD_BASE	U(0xF9000000)
#define PLAT_ARM_GICR_BASE	U(0xF9080000)

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_VERSAL_G1S_IRQS	VERSAL_IRQ_SEC_PHY_TIMER
#define PLAT_VERSAL_G0_IRQS	VERSAL_IRQ_SEC_PHY_TIMER
#define PLAT_VERSAL_IPI_IRQ	U(62)

#define PLAT_VERSAL_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(VERSAL_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)

#define PLAT_VERSAL_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(PLAT_VERSAL_IPI_IRQ, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(CPU_PWR_DOWN_REQ_INTR, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

#define IRQ_MAX		142U

#endif /* PLATFORM_DEF_H */
