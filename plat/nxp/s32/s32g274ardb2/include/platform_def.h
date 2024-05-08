/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <plat/common/common_def.h>

#define PLATFORM_STACK_SIZE		U(0x1000)

/* Caches */
#define CACHE_WRITEBACK_SHIFT		U(6)
#define CACHE_WRITEBACK_GRANULE		(U(1) << CACHE_WRITEBACK_SHIFT)

/* CPU Topology */
#define PLATFORM_CORE_COUNT		U(4)
#define PLATFORM_SYSTEM_COUNT		U(1)
#define PLATFORM_CLUSTER_COUNT		U(2)
#define PLATFORM_PRIMARY_CPU		U(0)
#define PLATFORM_MPIDR_CPU_MASK_BITS	U(1)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(2)

/* Power Domains */
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_SYSTEM_COUNT + \
					 PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2
#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_PWR_LVL_STATES		U(2)

/* BL2 stage */
#define BL2_BASE			UL(0x34078000)
#define BL2_LIMIT			UL(0x34100000)

/* BL31 stage */
#define BL31_BASE			UL(0x34200000)
#define BL31_LIMIT			UL(0x34300000)

/* It is a dummy value for now, given the missing DDR */
#define BL33_BASE			UL(0x34500000)
#define BL33_LIMIT			UL(0x345FF000)

#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 36)
/* We'll be doing a 1:1 mapping anyway */
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 36)

#define MAX_MMAP_REGIONS		U(8)
#define MAX_XLAT_TABLES			U(32)

/* Console settings */
#define UART_BASE			UL(0x401C8000)
#define UART_BAUDRATE			U(115200)
/* FIRC clock */
#define UART_CLOCK_HZ			U(48000000)

#define S32G_FIP_BASE			UL(0x34100000)
#define S32G_FIP_SIZE			UL(0x100000)

#define MAX_IO_HANDLES			U(2)
#define MAX_IO_DEVICES			U(2)

/* GIC settings */
#define S32G_GIC_BASE			UL(0x50800000)
#define PLAT_GICD_BASE			S32G_GIC_BASE
#define PLAT_GICR_BASE			(S32G_GIC_BASE + UL(0x80000))

/* Generic timer frequency; this goes directly into CNTFRQ_EL0.
 * Its end-value is 5MHz; this is based on the assumption that
 * GPR00[CA53_COUNTER_CLK_DIV_VAL] contains the reset value of 0x7, hence
 * producing a divider value of 8, applied to the FXOSC frequency of 40MHz.
 */
#define COUNTER_FREQUENCY		U(5000000)

#endif /* PLATFORM_DEF_H */
