/*
 * Copyright (c) 2014-2022, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gic_common.h>
#include <lib/utils_def.h>

#include "zynqmp_def.h"

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#ifndef PLATFORM_STACK_SIZE
#define PLATFORM_STACK_SIZE 0x440
#endif

#define PLATFORM_CORE_COUNT		U(4)
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
#ifndef ZYNQMP_ATF_MEM_BASE
#if !DEBUG && defined(SPD_none) && !SDEI_SUPPORT
# define BL31_BASE			U(0xfffea000)
# define BL31_LIMIT			U(0x100000000)
#else
# define BL31_BASE			U(0x1000)
# define BL31_LIMIT			U(0x80000)
#endif
#else
# define BL31_BASE			U(ZYNQMP_ATF_MEM_BASE)
# define BL31_LIMIT			(UL(ZYNQMP_ATF_MEM_BASE) + U(ZYNQMP_ATF_MEM_SIZE))
# ifdef ZYNQMP_ATF_MEM_PROGBITS_SIZE
#  define BL31_PROGBITS_LIMIT		(UL(ZYNQMP_ATF_MEM_BASE) + U(ZYNQMP_ATF_MEM_PROGBITS_SIZE))
# endif
#endif

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
#ifndef ZYNQMP_BL32_MEM_BASE
# define BL32_BASE			U(0x60000000)
# define BL32_LIMIT			U(0x80000000)
#else
# define BL32_BASE			U(ZYNQMP_BL32_MEM_BASE)
# define BL32_LIMIT			(UL(ZYNQMP_BL32_MEM_BASE) + U(ZYNQMP_BL32_MEM_SIZE))
#endif

/*******************************************************************************
 * BL33 specific defines.
 ******************************************************************************/
#ifndef PRELOADED_BL33_BASE
# define PLAT_ARM_NS_IMAGE_BASE	U(0x8000000)
#else
# define PLAT_ARM_NS_IMAGE_BASE	U(PRELOADED_BL33_BASE)
#endif

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
#define XILINX_OF_BOARD_DTB_MAX_SIZE	U(0x200000)
#define PLAT_DDR_LOWMEM_MAX		U(0x80000000)
#define PLAT_OCM_BASE			U(0xFFFC0000)
#define PLAT_OCM_LIMIT			U(0xFFFFFFFF)

#define IS_TFA_IN_OCM(x)		((x >= PLAT_OCM_BASE) && (x < PLAT_OCM_LIMIT))

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)

#ifndef MAX_MMAP_REGIONS
#if (defined(XILINX_OF_BOARD_DTB_ADDR) && !IS_TFA_IN_OCM(BL31_BASE))
#define MAX_MMAP_REGIONS		8
#else
#define MAX_MMAP_REGIONS		7
#endif
#endif

#ifndef MAX_XLAT_TABLES
#if !IS_TFA_IN_OCM(BL31_BASE)
#define MAX_XLAT_TABLES			8
#else
#define MAX_XLAT_TABLES			5
#endif
#endif

#define CACHE_WRITEBACK_SHIFT   6
#define CACHE_WRITEBACK_GRANULE (1 << CACHE_WRITEBACK_SHIFT)

#define ZYNQMP_SDEI_SGI_PRIVATE		U(8)

/* Platform macros to support exception handling framework */
#define PLAT_PRI_BITS			U(3)
#define PLAT_SDEI_CRITICAL_PRI		0x10
#define PLAT_SDEI_NORMAL_PRI		0x20

#define PLAT_ARM_GICD_BASE	BASE_GICD_BASE
#define PLAT_ARM_GICC_BASE	BASE_GICC_BASE
/*
 * Define properties of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#if !ZYNQMP_WDT_RESTART
#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)
#else
#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(IRQ_TTC3_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)
#endif

#define PLAT_ARM_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI,	grp, \
			GIC_INTR_CFG_EDGE)

#endif /* PLATFORM_DEF_H */
