/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <plat/common/common_def.h>
#include <platform_def.h>
#include "../fpga_def.h"

#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"

#define PLATFORM_LINKER_ARCH		aarch64

#define PLATFORM_STACK_SIZE		UL(0x800)

#define CACHE_WRITEBACK_SHIFT		U(6)
#define CACHE_WRITEBACK_GRANULE		(U(1) << CACHE_WRITEBACK_SHIFT)

#define PLATFORM_CORE_COUNT		\
	(FPGA_MAX_CLUSTER_COUNT *	\
	 FPGA_MAX_CPUS_PER_CLUSTER *	\
	 FPGA_MAX_PE_PER_CPU)

#define PLAT_NUM_PWR_DOMAINS (FPGA_MAX_CLUSTER_COUNT + PLATFORM_CORE_COUNT + 1)

#if !ENABLE_PIE
#define BL31_BASE			UL(0x80000000)
#define BL31_LIMIT			UL(0x80070000)
#else
#define BL31_BASE			UL(0x0)
#define BL31_LIMIT			UL(0x01000000)
#endif

#define PLAT_SDEI_NORMAL_PRI		0x70

#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

/*
 * Define a list of Group 1 Secure and Group 0 interrupt properties as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLATFORM_G1S_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_LEVEL), \
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
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE)

#define PLATFORM_G0_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE)

#define PLAT_MAX_RET_STATE 		1
#define PLAT_MAX_OFF_STATE 		2

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2

#define PLAT_FPGA_HOLD_ENTRY_SHIFT	3
#define PLAT_FPGA_HOLD_STATE_WAIT	0
#define PLAT_FPGA_HOLD_STATE_GO		1

#endif
