/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef BRUIN_DEF_H
#define BRUIN_DEF_H

#include <common_def.h>

#include <qti_board_def.h>

/*
 * Definitions shared by every bruin SoC. The SoC header (shikra_def.h,
 * agatti_def.h) includes this file and adds the SoC addresses.
 */

/*----------------------------------------------------------------------------*/
/*
 * MPIDR_PRIMARY_CPU
 * The core_affinity_val is represented by [7:0] and cluster_affinity_val is
 * represented by [15:8] while the rest bits are ignored
 */
/*----------------------------------------------------------------------------*/
#define MPIDR_PRIMARY_CPU		0x0000

#define QTI_PWR_LVL0			MPIDR_AFFLVL0
#define QTI_PWR_LVL1			MPIDR_AFFLVL1
#define QTI_PWR_LVL2			MPIDR_AFFLVL2
#define QTI_PWR_LVL3			MPIDR_AFFLVL3

/*
 *  Macros for local power states encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define QTI_LOCAL_STATE_RUN		0
/*
 * Local power state for clock-gating. Valid only for CPU and not cluster power
 * domains
 */
#define QTI_LOCAL_STATE_STB		1
/*
 * Local power state for retention. Valid for CPU and cluster power
 * domains
 */
#define QTI_LOCAL_STATE_RET		2
/*
 * Local power state for OFF/power down. Valid for CPU, cluster power
 * domains
 */
#define QTI_LOCAL_STATE_OFF		3
/*
 * Local power state for DEEPOFF/power rail down. Valid for CPU, cluster
 * power domains
 */
#define QTI_LOCAL_STATE_DEEPOFF		4

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		QTI_LOCAL_STATE_RET

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		QTI_LOCAL_STATE_DEEPOFF

/*
 * Platform specific page table and MMU setup constants.
 */
#define MAX_MMAP_REGIONS		(PLAT_QTI_MMAP_ENTRIES)

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
/* There is one top-level FCM cluster */
#define PLAT_CLUSTER_COUNT		1

/* No. of cores in the FCM cluster */
#define PLAT_CLUSTER0_CORE_COUNT	4

#define PLATFORM_CORE_COUNT		(PLAT_CLUSTER0_CORE_COUNT)

/*
 * PLAT_NUM_PWR_DOMAINS must count every node in the PSCI tree:
 *   level 2 (top)  : 1 node  (the SoC/system node)
 *   level 1        : PLAT_CLUSTER_COUNT nodes
 *   level 0 (CPUs) : PLATFORM_CORE_COUNT nodes
 * Non-CPU nodes = 1 + PLAT_CLUSTER_COUNT = 2; total = 2 + 4 = 6.
 */
#define PLAT_NUM_PWR_DOMAINS		(1 + PLAT_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

#define PLAT_MAX_PWR_LVL		2

/*----------------------------------------------------------------------------*/
/* IP protected memory (TA execution area) base                                */
/*----------------------------------------------------------------------------*/
#define QTI_PIMEM_BASE			0x10000000

/*----------------------------------------------------------------------------*/
/* GENI UART register layout */
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

/*----------------------------------------------------------------------------*/
/* PS HOLD register */
/*----------------------------------------------------------------------------*/
#define QTI_PS_HOLD_REG				0x0440B000

/*----------------------------------------------------------------------------*/
/* SOC hw version register */
/*----------------------------------------------------------------------------*/
#define QTI_SOC_VERSION_MASK			0xFFFF
#define QTI_SOC_REVISION_REG			0x003C8000
#define QTI_SOC_REVISION_MASK			0xFFFF

/*----------------------------------------------------------------------------*/
/* PMIC SPMI arbitrator base address                                          */
/*----------------------------------------------------------------------------*/
#define QTI_PMIC_ARB_BASE			0x01C00000

/*----------------------------------------------------------------------------*/
/* SMEM size                                                                  */
/*----------------------------------------------------------------------------*/
#define QTI_SMEM_SIZE				0x00200000

/*----------------------------------------------------------------------------*/
/* Interrupt IDs */
/*----------------------------------------------------------------------------*/
#define PLAT_INT_ID_RESET_SGI			0xf
#define PLAT_INT_ID_CPU_WAKEUP_SGI		0x8

#endif /* BRUIN_DEF_H */
