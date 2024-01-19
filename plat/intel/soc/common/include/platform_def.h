/*
 * Copyright (c) 2019-2022, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/interrupt_props.h>
#include <common/tbbr/tbbr_img_def.h>
#include <plat/common/common_def.h>
#include "socfpga_plat_def.h"

/* Platform Type */
#define PLAT_SOCFPGA_STRATIX10			1
#define PLAT_SOCFPGA_AGILEX			2
#define PLAT_SOCFPGA_N5X			3
#define PLAT_SOCFPGA_AGILEX5			4
#define SIMICS_RUN				1
#define MAX_IO_MTD_DEVICES			U(1)

/* sysmgr.boot_scratch_cold4 & 5 used for CPU release address for SPL */
#define PLAT_CPU_RELEASE_ADDR			0xffd12210

/* Magic word to indicate L2 reset is completed */
#define L2_RESET_DONE_STATUS			0x1228E5E7

/* Define next boot image name and offset */
/* Get non-secure image entrypoint for BL33. Zephyr and Linux */
#if	PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5

#ifndef PRELOADED_BL33_BASE
#define PLAT_NS_IMAGE_OFFSET			0x80200000
#else
#define PLAT_NS_IMAGE_OFFSET			PRELOADED_BL33_BASE
#endif
#define PLAT_HANDOFF_OFFSET 0x0003F000

#else
#define PLAT_NS_IMAGE_OFFSET			0x10000000
#define PLAT_HANDOFF_OFFSET			0xFFE3F000
#endif

#define PLAT_QSPI_DATA_BASE			(0x3C00000)
#define PLAT_NAND_DATA_BASE			(0x0200000)
#define PLAT_SDMMC_DATA_BASE			(0x0)

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT			"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH			aarch64

/* SoCFPGA supports up to 124GB RAM */
#define PLAT_PHY_ADDR_SPACE_SIZE		(1ULL << 39)
#define PLAT_VIRT_ADDR_SPACE_SIZE		(1ULL << 39)


/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
#define PLAT_SECONDARY_ENTRY_BASE		0x01f78bf0

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE			0x2000

/* PSCI related constant */
#define PLAT_NUM_POWER_DOMAINS			5
#define PLAT_MAX_PWR_LVL			1
#define PLAT_MAX_RET_STATE			1
#define PLAT_MAX_OFF_STATE			2
#define PLATFORM_SYSTEM_COUNT			U(1)
#define PLATFORM_CLUSTER_COUNT			U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT		U(4)
#define PLATFORM_CLUSTER1_CORE_COUNT		U(0)
#define PLATFORM_CORE_COUNT			(PLATFORM_CLUSTER1_CORE_COUNT + \
						PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER		U(4)

/* Interrupt related constant */

#define INTEL_SOCFPGA_IRQ_SEC_PHY_TIMER		29

#define INTEL_SOCFPGA_IRQ_SEC_SGI_0		8
#define INTEL_SOCFPGA_IRQ_SEC_SGI_1		9
#define INTEL_SOCFPGA_IRQ_SEC_SGI_2		10
#define INTEL_SOCFPGA_IRQ_SEC_SGI_3		11
#define INTEL_SOCFPGA_IRQ_SEC_SGI_4		12
#define INTEL_SOCFPGA_IRQ_SEC_SGI_5		13
#define INTEL_SOCFPGA_IRQ_SEC_SGI_6		14
#define INTEL_SOCFPGA_IRQ_SEC_SGI_7		15

#define TSP_IRQ_SEC_PHY_TIMER			INTEL_SOCFPGA_IRQ_SEC_PHY_TIMER
#define TSP_SEC_MEM_BASE			BL32_BASE
#define TSP_SEC_MEM_SIZE			(BL32_LIMIT - BL32_BASE + 1)


/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL3-1 debug size plus a
 * little space for growth.
 */

#define FIRMWARE_WELCOME_STR			"Booting Trusted Firmware\n"

#define BL1_RO_BASE				(0xffe00000)
#define BL1_RO_LIMIT				(0xffe0f000)
#define BL1_RW_BASE				(0xffe10000)
#define BL1_RW_LIMIT				(0xffe1ffff)
#define BL1_RW_SIZE				(0x14000)

#define BL_DATA_LIMIT				PLAT_HANDOFF_OFFSET

#define PLAT_CPUID_RELEASE			(BL_DATA_LIMIT - 16)
#define PLAT_SEC_ENTRY				(BL_DATA_LIMIT - 8)

#define CMP_ENTRY				0xFFE3EFF8

#define PLAT_SEC_WARM_ENTRY			0

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define MAX_XLAT_TABLES				8
#define MAX_MMAP_REGIONS			16

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT			6
#define CACHE_WRITEBACK_GRANULE			(1 << CACHE_WRITEBACK_SHIFT)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define CRASH_CONSOLE_BASE			PLAT_UART0_BASE
#define PLAT_INTEL_UART_BASE			PLAT_UART0_BASE

#define PLAT_BAUDRATE				(115200)
#define PLAT_UART_CLOCK				(100000000)

/*******************************************************************************
 * PHY related constants
 ******************************************************************************/

#define EMAC0_PHY_MODE				PHY_INTERFACE_MODE_RGMII
#define EMAC1_PHY_MODE				PHY_INTERFACE_MODE_RGMII
#define EMAC2_PHY_MODE				PHY_INTERFACE_MODE_RGMII

/*******************************************************************************
 * GIC related constants
 ******************************************************************************/
#define PLAT_INTEL_SOCFPGA_GICD_BASE		PLAT_GICD_BASE
#define PLAT_INTEL_SOCFPGA_GICC_BASE		PLAT_GICC_BASE

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_INTEL_SOCFPGA_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(INTEL_SOCFPGA_IRQ_SEC_PHY_TIMER, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(INTEL_SOCFPGA_IRQ_SEC_SGI_0, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_SOCFPGA_IRQ_SEC_SGI_1, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_SOCFPGA_IRQ_SEC_SGI_2, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_SOCFPGA_IRQ_SEC_SGI_3, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_SOCFPGA_IRQ_SEC_SGI_4, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_SOCFPGA_IRQ_SEC_SGI_5, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_SOCFPGA_IRQ_SEC_SGI_6, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_SOCFPGA_IRQ_SEC_SGI_7, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_EDGE)

#define PLAT_INTEL_SOCFPGA_G0_IRQ_PROPS(grp)

#define MAX_IO_HANDLES				4
#define MAX_IO_DEVICES				4
#define MAX_IO_BLOCK_DEVICES			2

#ifndef __ASSEMBLER__
struct socfpga_bl31_params {
	param_header_t h;
	image_info_t *bl31_image_info;
	entry_point_info_t *bl32_ep_info;
	image_info_t *bl32_image_info;
	entry_point_info_t *bl33_ep_info;
	image_info_t *bl33_image_info;
};
#endif

#endif /* PLATFORM_DEF_H */
