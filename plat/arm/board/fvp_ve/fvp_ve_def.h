/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_VE_DEF_H
#define FVP_VE_DEF_H

#include <lib/utils_def.h>

/* Default cluster count for FVP VE */
#define FVP_VE_CLUSTER_COUNT		U(1)

/* Default number of CPUs per cluster on FVP VE */
#define FVP_VE_MAX_CPUS_PER_CLUSTER	U(1)

/* Default number of threads per CPU on FVP VE */
#define FVP_VE_MAX_PE_PER_CPU		U(1)

#define FVP_VE_CORE_COUNT		U(1)

#define FVP_VE_PRIMARY_CPU		0x0

/*******************************************************************************
 * FVP memory map related constants
 ******************************************************************************/

#define FLASH1_BASE			0x0c000000
#define FLASH1_SIZE			0x04000000

/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE			0x20000000
#define DEVICE0_SIZE			0x0c200000

#define NSRAM_BASE			0x2e000000
#define NSRAM_SIZE			0x10000

#define PCIE_EXP_BASE			0x40000000
#define TZRNG_BASE			0x7fe60000

#define ARCH_MODEL_VE			0x5

/* FVP Power controller base address*/
#define PWRC_BASE			UL(0x1c100000)

/* FVP SP804 timer frequency is 35 MHz*/
#define SP804_TIMER_CLKMULT		1
#define SP804_TIMER_CLKDIV		35

/* SP810 controller. FVP specific flags */
#define FVP_SP810_CTRL_TIM0_OV		(1 << 16)
#define FVP_SP810_CTRL_TIM1_OV		(1 << 18)
#define FVP_SP810_CTRL_TIM2_OV		(1 << 20)
#define FVP_SP810_CTRL_TIM3_OV		(1 << 22)

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
/* VE compatible GIC memory map */
#define VE_GICD_BASE			0x2c001000
#ifdef ARM_CORTEX_A5
#define VE_GICC_BASE			0x2c000100
#else
#define VE_GICC_BASE			0x2c002000
#endif
#define VE_GICH_BASE			0x2c004000
#define VE_GICV_BASE			0x2c006000

#define FVP_VE_IRQ_TZ_WDOG			56
#define FVP_VE_IRQ_SEC_SYS_TIMER		57

#define V2M_FLASH1_BASE			UL(0x0C000000)
#define V2M_FLASH1_SIZE			UL(0x04000000)

#define V2M_MAP_FLASH1_RW		MAP_REGION_FLAT(V2M_FLASH1_BASE,\
						V2M_FLASH1_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define V2M_MAP_FLASH1_RO		MAP_REGION_FLAT(V2M_FLASH1_BASE,\
						V2M_FLASH1_SIZE,	\
						MT_RO_DATA | MT_SECURE)

#endif /* FVP_VE_DEF_H */
