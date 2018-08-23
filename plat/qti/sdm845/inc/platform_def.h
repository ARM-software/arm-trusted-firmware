/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

/* Enable the dynamic translation tables library. */
#define PLAT_XLAT_TABLES_DYNAMIC     1

#include <board_qti_def.h>
#include <common_def.h>

/*----------------------------------------------------------------------------*/
/* SOC_VERSION definitions */
/*----------------------------------------------------------------------------*/
#define SOC_ID_SDM845		0x60000100

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* MPIDR_PRIMARY_CPU
 * You just need to have the correct core_affinity_val i.e. [7:0]
 * and cluster_affinity_val i.e. [15:8]
 * the other bits will be ignored */
/*----------------------------------------------------------------------------*/
#define MPIDR_PRIMARY_CPU	0x0000
/*----------------------------------------------------------------------------*/

/*
 *  Macros for local power states encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define QTI_LOCAL_STATE_RUN	0
/* Local power state for clock-gating. Valid only for CPU and not cluster power
   domains */
#define QTI_LOCAL_STATE_STB	1
/* Local power state for retention. Valid for CPU and cluster power
   domains */
#define QTI_LOCAL_STATE_RET	2
/* Local power state for OFF/power-down. Valid for CPU and cluster power
   domains */
#define QTI_LOCAL_STATE_OFF	3
/* Local power state for OFF/power-down. Valid for CPU and cluster power
   domains */
#define QTI_LOCAL_STATE_DEEPOFF	4

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE	QTI_LOCAL_STATE_RET

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE	QTI_LOCAL_STATE_DEEPOFF

/******************************************************************************
 * Required platform porting definitions common to all ARM standard platforms
 *****************************************************************************/

/*
 * Platform specific page table and MMU setup constants.
 */
#define MAX_MMAP_REGIONS	(PLAT_QTI_MMAP_ENTRIES)

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 36)

#define ARM_CACHE_WRITEBACK_SHIFT	6

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE		(1 << ARM_CACHE_WRITEBACK_SHIFT)

/*
 * One cache line needed for bakery locks on ARM platforms
 */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE	(1 * CACHE_WRITEBACK_GRANULE)

/*----------------------------------------------------------------------------*/
/* PSCI power domain topology definitions */
/*----------------------------------------------------------------------------*/
/* One domain to represent Cx level */
#define PLAT_CX_RAIL_COUNT		1

/* There is one top-level FCM cluster */
#define PLAT_CLUSTER_COUNT       	1

/* No. of cores in the FCM cluster */
#define PLAT_CLUSTER0_CORE_COUNT	8

#define PLATFORM_CORE_COUNT		(PLAT_CLUSTER0_CORE_COUNT)

#define PLAT_NUM_PWR_DOMAINS		(PLAT_CX_RAIL_COUNT	+\
					PLAT_CLUSTER_COUNT	+\
					PLATFORM_CORE_COUNT)

#define PLAT_MAX_PWR_LVL		2

/*****************************************************************************/
/* Memory mapped Generic timer interfaces  */
/*****************************************************************************/

/*----------------------------------------------------------------------------*/
/* GIC-600 constants */
/*----------------------------------------------------------------------------*/
#define BASE_GICD_BASE		0x17A00000
#define BASE_GICR_BASE		0x17A60000
#define BASE_GICC_BASE		0x0
#define BASE_GICH_BASE		0x0
#define BASE_GICV_BASE		0x0

#define QTI_GICD_BASE      	BASE_GICD_BASE
#define QTI_GICR_BASE		BASE_GICR_BASE
#define QTI_GICC_BASE      	BASE_GICC_BASE

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* UART related constants. */
/*----------------------------------------------------------------------------*/
/* BASE ADDRESS OF DIFFERENT REGISTER SPACES IN HW */
#define GENI4_CFG				0x0
#define GENI4_IMAGE_REGS			0x100
#define GENI4_DATA				0x600

/* COMMON STATUS/CONFIGURATION REGISTERS AND MASKS */
#define GENI_STATUS_REG				(GENI4_CFG + 0x00000040)
#define GENI_STATUS_M_GENI_CMD_ACTIVE_MASK	(0x1)
#define UART_TX_TRANS_LEN_REG			(GENI4_IMAGE_REGS + 0x00000170)
/* MASTER/TX ENGINE REGISTERS */
#define GENI_M_CMD0_REG				(GENI4_DATA + 0x00000000)
/* FIFO, STATUS REGISTERS AND MASKS */
#define GENI_TX_FIFOn_REG			(GENI4_DATA + 0x00000100)

#define GENI_M_CMD_TX				(0x08000000)

/*----------------------------------------------------------------------------*/
/* Device address space for mapping. Excluding starting 4K */
/*----------------------------------------------------------------------------*/
#define QTI_DEVICE_BASE				0x1000
#define QTI_DEVICE_SIZE				(0x19000000 - QTI_DEVICE_BASE)

#define QTI_SHARED_IMEM_BASE			0x146BF000
#define QTI_SHARED_IMEM_TF_STACK_CANARY_ADDR	(QTI_SHARED_IMEM_BASE + 0x7F0)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at DDR as per memory map. BL31_BASE is calculated using the
 * current BL31 debug size plus a little space for growth.
 */
#define BL31_BASE						0x86300000
#define BL31_SIZE						0x00200000
#define QTI_TRUSTED_MAILBOX_SIZE				0x1000
#define BL31_LIMIT						(BL31_BASE + BL31_SIZE - QTI_TRUSTED_MAILBOX_SIZE)

/*----------------------------------------------------------------------------*/
/* Mailbox base address */
/*----------------------------------------------------------------------------*/
#define QTI_TRUSTED_MAILBOX_BASE		(BL31_BASE + BL31_SIZE - QTI_TRUSTED_MAILBOX_SIZE)
/*----------------------------------------------------------------------------*/

#endif /* __PLATFORM_DEF_H__ */
