/*
 * Copyright 2024-2026 NXP
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

#define S32G_DDR0_END		UL(0xffffffff)

#define S32G_BL33_IMAGE_SIZE		(7 * SZ_1M)

/* BL2 stage */
#define BL2_BASE			UL(0x34078000)
#define BL2_LIMIT			UL(0x34100000)

/* BL33 stage */
#define BL33_BASE			(S32G_DDR0_END - S32G_BL33_IMAGE_SIZE - \
						SZ_1M + 1)
#define BL33_LIMIT			(S32G_DDR0_END)

/* BL31 stage */
#define BL31_BASE			((BL33_BASE - 1) - (2 * SZ_1M) + 1)
#define BL31_LIMIT			(BL33_BASE - 1)

/* IO buffer used to copy images from storage */
#define IO_BUFFER_BASE			UL(0x345ff000)
#define IO_BUFFER_SIZE			U(0x13000)

#define DDR_PHY_BASE_ADDR		UL(0x40380000)
#define DDR_PHY_SIZE			U(0x80000)

#define GPR_BASE_PAGE_ADDR		UL(0x4007C000)
#define GPR_SIZE			U(0x1000)

#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 36)
/* We'll be doing a 1:1 mapping anyway */
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 36)

#define MAX_MMAP_REGIONS		U(23)
#define MAX_XLAT_TABLES			U(33)

/* Console settings */
#define UART_BASE			UL(0x401C8000)
#define UART_BAUDRATE			U(115200)
#define UART_CLOCK_HZ			U(125000000)

/* SCMI SMT mailbox, owned by BL31; the non-secure "arm,scmi-shmem" must match. */
#define S32G_SCMI_SHMEM_BASE		UL(0xd0000000)
#define S32G_SCMI_SHMEM_SIZE		UL(0x80)

/* uSDHC */
#define S32G_USDHC_BASE			UL(0x402F0000)

#define S32G_FIP_BASE			UL(0x34100000)
#define S32G_FIP_SIZE			UL(0x100000)

#define MAX_IO_HANDLES			U(2)
#define MAX_IO_DEVICES			U(2)

/* uSDHC as block device */
#define MAX_IO_BLOCK_DEVICES		U(1)

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
