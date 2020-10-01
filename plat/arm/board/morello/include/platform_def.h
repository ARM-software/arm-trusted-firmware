/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/arm_def.h>
#include <plat/arm/css/common/css_def.h>

/* UART related constants */
#define PLAT_ARM_BOOT_UART_BASE 		ULL(0x2A400000)
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ		U(50000000)

#define PLAT_ARM_RUN_UART_BASE			ULL(0x2A410000)
#define PLAT_ARM_RUN_UART_CLK_IN_HZ		U(50000000)

#define PLAT_ARM_CRASH_UART_BASE		PLAT_ARM_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ		PLAT_ARM_RUN_UART_CLK_IN_HZ

#define PLAT_ARM_DRAM2_BASE			ULL(0x8080000000)
#define PLAT_ARM_DRAM2_SIZE			ULL(0xF80000000)

/*
 * To access the complete DDR memory along with remote chip's DDR memory,
 * which is at 4 TB offset, physical and virtual address space limits are
 * extended to 43-bits.
 */
#define PLAT_PHY_ADDR_SPACE_SIZE		(1ULL << 43)
#define PLAT_VIRT_ADDR_SPACE_SIZE		(1ULL << 43)

#if CSS_USE_SCMI_SDS_DRIVER
#define MORELLO_SCMI_PAYLOAD_BASE		ULL(0x45400000)
#else
#define PLAT_CSS_SCP_COM_SHARED_MEM_BASE	ULL(0x45400000)
#endif

#define PLAT_ARM_TRUSTED_SRAM_SIZE		UL(0x00080000)
#define PLAT_ARM_MAX_BL31_SIZE			UL(0x20000)

/*******************************************************************************
 * MORELLO topology related constants
 ******************************************************************************/
#define MORELLO_MAX_CPUS_PER_CLUSTER		U(2)
#define PLAT_ARM_CLUSTER_COUNT			U(2)
#define PLAT_MORELLO_CHIP_COUNT			U(1)
#define MORELLO_MAX_CLUSTERS_PER_CHIP		U(2)
#define MORELLO_MAX_PE_PER_CPU			U(1)

#define PLATFORM_CORE_COUNT			(PLAT_MORELLO_CHIP_COUNT *	\
						PLAT_ARM_CLUSTER_COUNT *	\
						MORELLO_MAX_CPUS_PER_CLUSTER *	\
						MORELLO_MAX_PE_PER_CPU)

/* System power domain level */
#define CSS_SYSTEM_PWR_DMN_LVL			ARM_PWR_LVL3

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#define PLAT_ARM_MMAP_ENTRIES			U(9)
#define MAX_XLAT_TABLES				U(10)

#define PLATFORM_STACK_SIZE			U(0x400)

#define PLAT_ARM_NSTIMER_FRAME_ID		U(0)
#define PLAT_CSS_MHU_BASE			UL(0x45000000)
#define PLAT_MHUV2_BASE				PLAT_CSS_MHU_BASE
#define PLAT_MAX_PWR_LVL			U(2)

#define PLAT_ARM_G1S_IRQ_PROPS(grp)		CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)		ARM_G0_IRQ_PROPS(grp)

#define MORELLO_DEVICE_BASE			ULL(0x08000000)
#define MORELLO_DEVICE_SIZE			ULL(0x48000000)

#define MORELLO_MAP_DEVICE			MAP_REGION_FLAT(	\
						MORELLO_DEVICE_BASE,	\
						MORELLO_DEVICE_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_MAP_DRAM1				MAP_REGION_FLAT(	\
						ARM_DRAM1_BASE,		\
						ARM_DRAM1_SIZE,		\
						MT_MEMORY | MT_RW | MT_NS)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE			UL(0x30000000)
#define PLAT_ARM_GICC_BASE			UL(0x2C000000)
#define PLAT_ARM_GICR_BASE			UL(0x300C0000)

/* Number of SCMI channels on the platform */
#define PLAT_ARM_SCMI_CHANNEL_COUNT		U(1)

#endif /* PLATFORM_DEF_H */
