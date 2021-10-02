/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

/* Enable the dynamic translation tables library. */
#define PLAT_XLAT_TABLES_DYNAMIC	1

#include <common_def.h>

#include <qti_board_def.h>
#include <qtiseclib_defs_plat.h>

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*
 * MPIDR_PRIMARY_CPU
 * You just need to have the correct core_affinity_val i.e. [7:0]
 * and cluster_affinity_val i.e. [15:8]
 * the other bits will be ignored
 */
/*----------------------------------------------------------------------------*/
#define MPIDR_PRIMARY_CPU	0x0000
/*----------------------------------------------------------------------------*/

#define QTI_PWR_LVL0		MPIDR_AFFLVL0
#define QTI_PWR_LVL1		MPIDR_AFFLVL1
#define QTI_PWR_LVL2		MPIDR_AFFLVL2
#define QTI_PWR_LVL3		MPIDR_AFFLVL3

/*
 *  Macros for local power states encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define QTI_LOCAL_STATE_RUN	0
/*
 * Local power state for clock-gating. Valid only for CPU and not cluster power
 * domains
 */
#define QTI_LOCAL_STATE_STB	1
/*
 * Local power state for retention. Valid for CPU and cluster power
 * domains
 */
#define QTI_LOCAL_STATE_RET	2
/*
 * Local power state for OFF/power down. Valid for CPU, cluster, RSC and PDC
 * power domains
 */
#define QTI_LOCAL_STATE_OFF	3
/*
 * Local power state for DEEPOFF/power rail down. Valid for CPU, cluster and RSC
 * power domains
 */
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
/* One domain each to represent RSC and PDC level */
#define PLAT_PDC_COUNT			1
#define PLAT_RSC_COUNT			1

/* There is one top-level FCM cluster */
#define PLAT_CLUSTER_COUNT		1

/* No. of cores in the FCM cluster */
#define PLAT_CLUSTER0_CORE_COUNT	8

#define PLATFORM_CORE_COUNT		(PLAT_CLUSTER0_CORE_COUNT)

#define PLAT_NUM_PWR_DOMAINS		(PLAT_PDC_COUNT +\
					PLAT_RSC_COUNT	+\
					PLAT_CLUSTER_COUNT	+\
					PLATFORM_CORE_COUNT)

#define PLAT_MAX_PWR_LVL		3

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

#define QTI_GICD_BASE		BASE_GICD_BASE
#define QTI_GICR_BASE		BASE_GICR_BASE
#define QTI_GICC_BASE		BASE_GICC_BASE

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
#define QTI_DEVICE_SIZE				(0x80000000 - QTI_DEVICE_BASE)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at DDR as per memory map. BL31_BASE is calculated using the
 * current BL31 debug size plus a little space for growth.
 */
#define BL31_LIMIT				(BL31_BASE + BL31_SIZE)

/*----------------------------------------------------------------------------*/
/* AOSS registers */
/*----------------------------------------------------------------------------*/
#define QTI_PS_HOLD_REG				0x0C264000
/*----------------------------------------------------------------------------*/
/* AOP CMD DB  address space for mapping */
/*----------------------------------------------------------------------------*/
#define QTI_AOP_CMD_DB_BASE			0x80820000
#define QTI_AOP_CMD_DB_SIZE			0x00020000
/*----------------------------------------------------------------------------*/
/* SOC hw version register */
/*----------------------------------------------------------------------------*/
#define QTI_SOC_VERSION				U(0x7180)
#define QTI_SOC_VERSION_MASK			U(0xFFFF)
#define QTI_SOC_REVISION_REG			0x1FC8000
#define QTI_SOC_REVISION_MASK			U(0xFFFF)
/*----------------------------------------------------------------------------*/
/* LC PON register offsets */
/*----------------------------------------------------------------------------*/
#define PON_PS_HOLD_RESET_CTL			0x85a
#define PON_PS_HOLD_RESET_CTL2			0x85b
/*----------------------------------------------------------------------------*/

#endif /* PLATFORM_DEF_H */
