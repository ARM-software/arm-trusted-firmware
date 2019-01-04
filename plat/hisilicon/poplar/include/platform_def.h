/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/interrupt_props.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/gic_common.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

#include "hi3798cv200.h"
#include "poplar_layout.h"		/* BL memory region sizes, etc */

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define POPLAR_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

#define POPLAR_CRASH_UART_BASE		PL011_UART0_BASE
#define POPLAR_CRASH_UART_CLK_IN_HZ	PL011_UART0_CLK_IN_HZ
#define POPLAR_CONSOLE_BAUDRATE		PL011_BAUDRATE

/* Generic platform constants */
#define PLATFORM_STACK_SIZE		(0x800)

#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"
#define BOOT_EMMC_NAME			"l-loader.bin"

#define PLATFORM_CACHE_LINE_SIZE	(64)
#define PLATFORM_CLUSTER_COUNT		(1)
#define PLATFORM_CORE_COUNT		(4)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	(4)

/* IO framework user */
#define MAX_IO_DEVICES			(4)
#define MAX_IO_HANDLES			(4)
#define MAX_IO_BLOCK_DEVICES		U(2)

/* Memory size options */
#define POPLAR_DRAM_SIZE_1G	0
#define POPLAR_DRAM_SIZE_2G	1

/* Memory map related constants */
#define DDR_BASE			(0x00000000)

#if (POPLAR_DRAM_SIZE_ID == POPLAR_DRAM_SIZE_2G)
#define DDR_SIZE			(0x80000000)
#elif (POPLAR_DRAM_SIZE_ID == POPLAR_DRAM_SIZE_1G)
#define DDR_SIZE			(0x40000000)
#else
#error "Currently unsupported POPLAR_DRAM_SIZE_ID value"
#endif

#define DEVICE_BASE			(0xF0000000)
#define DEVICE_SIZE			(0x0F000000)

#define TEE_SEC_MEM_BASE		(0x70000000)
#define TEE_SEC_MEM_SIZE		(0x10000000)

/* Memory location options for TSP */
#define POPLAR_SRAM_ID	0
#define POPLAR_DRAM_ID	1

/*
 * DDR for OP-TEE (26MB from 0x02400000 -0x04000000) is divided in several
 * regions:
 *   - Secure DDR (default is the top 16MB) used by OP-TEE
 *   - Non-secure DDR (4MB) reserved for OP-TEE's future use
 *   - Secure DDR (4MB aligned on 4MB) for OP-TEE's "Secure Data Path" feature
 *   - Non-secure DDR used by OP-TEE (shared memory and padding) (4MB)
 */
#define DDR_SEC_SIZE			0x01000000
#define DDR_SEC_BASE			0x03000000

/*
 * BL3-2 specific defines.
 */

/*
 * The TSP currently executes from TZC secured area of DRAM.
 */
#define BL32_DRAM_BASE			0x03000000
#define BL32_DRAM_LIMIT			0x04000000

#ifdef SPD_opteed
/* Load pageable part of OP-TEE at end of allocated DRAM space for BL32 */
#define POPLAR_OPTEE_PAGEABLE_LOAD_SIZE	0x400000 /* 4MB */
#define POPLAR_OPTEE_PAGEABLE_LOAD_BASE	(BL32_DRAM_LIMIT - POPLAR_OPTEE_PAGEABLE_LOAD_SIZE) /* 0x03C0_0000 */
#endif

#if (POPLAR_TSP_RAM_LOCATION_ID == POPLAR_DRAM_ID)
#define TSP_SEC_MEM_BASE		BL32_DRAM_BASE
#define TSP_SEC_MEM_SIZE		(BL32_DRAM_LIMIT - BL32_DRAM_BASE)
#define BL32_BASE			BL32_DRAM_BASE
#define BL32_LIMIT			BL32_DRAM_LIMIT
#elif (POPLAR_TSP_RAM_LOCATION_ID == POPLAR_SRAM_ID)
#error "SRAM storage of TSP payload is currently unsupported"
#else
#error "Currently unsupported POPLAR_TSP_LOCATION_ID value"
#endif

/* BL32 is mandatory in AArch32 */
#ifndef AARCH32
#ifdef SPD_none
#undef BL32_BASE
#endif /* SPD_none */
#endif

#define POPLAR_EMMC_DATA_BASE U(0x02200000)
#define POPLAR_EMMC_DATA_SIZE EMMC_DESC_SIZE
#define POPLAR_EMMC_DESC_BASE (POPLAR_EMMC_DATA_BASE + POPLAR_EMMC_DATA_SIZE)
#define POPLAR_EMMC_DESC_SIZE EMMC_DESC_SIZE

#define PLAT_POPLAR_NS_IMAGE_OFFSET	0x37000000

/* Page table and MMU setup constants */
#define PLAT_VIRT_ADDR_SPACE_SIZE   (1ULL << 32)
#define PLAT_PHY_ADDR_SPACE_SIZE    (1ULL << 32)
#define MAX_XLAT_TABLES			(4)
#define MAX_MMAP_REGIONS		(16)

#define CACHE_WRITEBACK_SHIFT		(6)
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

/* Power states */
#define PLAT_MAX_PWR_LVL		(MPIDR_AFFLVL1)
#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_RET_STATE		U(1)

/* Interrupt controller */
#define POPLAR_GICD_BASE	GICD_BASE
#define POPLAR_GICC_BASE	GICC_BASE

#define POPLAR_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(HISI_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_TIMER0, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_TIMER1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_TIMER2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_TIMER3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(HISI_IRQ_SEC_AXI, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)

#define POPLAR_G0_IRQ_PROPS(grp)

#endif /* PLATFORM_DEF_H */
