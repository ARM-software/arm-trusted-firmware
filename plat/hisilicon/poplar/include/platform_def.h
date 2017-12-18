/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include <common_def.h>
#include <gic_common.h>
#include <interrupt_props.h>
#include <tbbr/tbbr_img_def.h>
#include "hi3798cv200.h"
#include "poplar_layout.h"		/* BL memory region sizes, etc */

#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

#define PLAT_ARM_CRASH_UART_BASE	PL011_UART0_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	PL011_UART0_CLK_IN_HZ
#define ARM_CONSOLE_BAUDRATE		PL011_BAUDRATE

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
#define MAX_IO_BLOCK_DEVICES		(2)

/* Memory map related constants */
#define DDR_BASE			(0x00000000)
#define DDR_SIZE			(0x40000000)

#define DEVICE_BASE			(0xF0000000)
#define DEVICE_SIZE			(0x0F000000)

#define TEE_SEC_MEM_BASE		(0x70000000)
#define TEE_SEC_MEM_SIZE		(0x10000000)

/* Memory location options for TSP */
#define POPLAR_SRAM_ID	0
#define POPLAR_DRAM_ID	1

/*
 * DDR for OP-TEE (28MB from 0x02200000 -0x04000000) is divided in several
 * regions:
 *   - Secure DDR (default is the top 16MB) used by OP-TEE
 *   - Non-secure DDR (4MB) reserved for OP-TEE's future use
 *   - Secure DDR (4MB aligned on 4MB) for OP-TEE's "Secure Data Path" feature
 *   - Non-secure DDR used by OP-TEE (shared memory and padding) (4MB)
 *   - Non-secure DDR (2MB) reserved for OP-TEE's future use
 */
#define DDR_SEC_SIZE			0x01000000
#define DDR_SEC_BASE			0x03000000

#define BL_MEM_BASE			(BL1_RO_BASE)
#define BL_MEM_LIMIT			(BL31_LIMIT)
#define BL_MEM_SIZE			(BL_MEM_LIMIT - BL_MEM_BASE)

/*
 * BL3-2 specific defines.
 */

/*
 * The TSP currently executes from TZC secured area of DRAM.
 */
#define BL32_DRAM_BASE			0x03000000
#define BL32_DRAM_LIMIT			0x04000000

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

#define PLAT_POPLAR_NS_IMAGE_OFFSET	0x37000000

/* Page table and MMU setup constants */
#define ADDR_SPACE_SIZE			(1ull << 32)
#define MAX_XLAT_TABLES			(4)
#define MAX_MMAP_REGIONS		(16)

#define CACHE_WRITEBACK_SHIFT		(6)
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

/* Power states */
#define PLAT_MAX_PWR_LVL		(MPIDR_AFFLVL1)
#define PLAT_MAX_OFF_STATE		2
#define PLAT_MAX_RET_STATE		1

/* Interrupt controller */
#define PLAT_ARM_GICD_BASE	GICD_BASE
#define PLAT_ARM_GICC_BASE	GICC_BASE

#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
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

#define PLAT_ARM_G0_IRQ_PROPS(grp)

#endif /* __PLATFORM_DEF_H__ */
