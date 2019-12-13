/*
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/arm_spm_def.h>
#include <plat/common/common_def.h>

/* Core/Cluster/Thread counts for Corstone700 */
#define CORSTONE700_CLUSTER_COUNT		U(1)
#define CORSTONE700_MAX_CPUS_PER_CLUSTER	U(4)
#define CORSTONE700_MAX_PE_PER_CPU		U(1)
#define CORSTONE700_CORE_COUNT		(CORSTONE700_CLUSTER_COUNT *	\
					CORSTONE700_MAX_CPUS_PER_CLUSTER * \
					CORSTONE700_MAX_PE_PER_CPU)
#define PLATFORM_CORE_COUNT		CORSTONE700_CORE_COUNT
#define PLAT_ARM_CLUSTER_COUNT		CORSTONE700_CLUSTER_COUNT

/* UART related constants */
#define PLAT_ARM_BOOT_UART_BASE		0x1a510000
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	V2M_IOFPGA_UART0_CLK_IN_HZ
#define PLAT_ARM_RUN_UART_BASE		0x1a520000
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	V2M_IOFPGA_UART1_CLK_IN_HZ
#define ARM_CONSOLE_BAUDRATE		115200
#define PLAT_ARM_CRASH_UART_BASE	PLAT_ARM_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	PLAT_ARM_RUN_UART_CLK_IN_HZ

/* Memory related constants */
#define ARM_DRAM1_BASE			UL(0x80000000)
#define ARM_DRAM1_SIZE			UL(0x80000000)
#define ARM_DRAM1_END			(ARM_DRAM1_BASE +	\
					 ARM_DRAM1_SIZE - 1)
#define ARM_NS_DRAM1_BASE		ARM_DRAM1_BASE
#define ARM_NS_DRAM1_SIZE		ARM_DRAM1_SIZE
#define ARM_NS_DRAM1_END                (ARM_NS_DRAM1_BASE +	\
					ARM_NS_DRAM1_SIZE - 1)
#define ARM_TRUSTED_SRAM_BASE		UL(0x02000000)
#define ARM_SHARED_RAM_BASE		ARM_TRUSTED_SRAM_BASE
#define ARM_SHARED_RAM_SIZE		UL(0x00001000)  /* 4 KB */
#define PLAT_ARM_TRUSTED_SRAM_SIZE	0x00040000	/* 256 KB */

/* The remaining Trusted SRAM is used to load the BL images */
#define ARM_BL_RAM_BASE			(ARM_SHARED_RAM_BASE +  \
					ARM_SHARED_RAM_SIZE)
#define ARM_BL_RAM_SIZE			(PLAT_ARM_TRUSTED_SRAM_SIZE -   \
					ARM_SHARED_RAM_SIZE)

/*
 * SP_MIN is the only BL image in SRAM. Allocate the whole of SRAM (excluding
 * the page reserved for fw_configs) to BL32
 */
#define BL32_BASE			(ARM_BL_RAM_BASE + PAGE_SIZE)
#define BL32_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE		(U(1) << ARM_CACHE_WRITEBACK_SHIFT)
#define ARM_CACHE_WRITEBACK_SHIFT	6

/*
 * To enable TB_FW_CONFIG to be loaded by BL1, define the corresponding base
 * and limit. Leave enough space for BL2 meminfo.
 */
#define ARM_TB_FW_CONFIG_BASE		(ARM_BL_RAM_BASE + sizeof(meminfo_t))
#define ARM_TB_FW_CONFIG_LIMIT		(ARM_BL_RAM_BASE + (PAGE_SIZE / 2U))

/*
 * The max number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#define ARM_BL_REGIONS			2
#define PLAT_ARM_MMAP_ENTRIES		8
#define MAX_XLAT_TABLES			5
#define MAX_MMAP_REGIONS		(PLAT_ARM_MMAP_ENTRIES +        \
					ARM_BL_REGIONS)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE			0x1C010000
#define PLAT_ARM_GICC_BASE			0x1C02F000

/* Timer/watchdog related constants */
#define ARM_SYS_CNTCTL_BASE			UL(0x1a200000)
#define ARM_SYS_CNTREAD_BASE			UL(0x1a210000)
#define ARM_SYS_TIMCTL_BASE			UL(0x1a220000)
#define CORSTONE700_TIMER_BASE_FREQUENCY	UL(24000000)
#define CORSTONE700_IRQ_TZ_WDOG			32
#define CORSTONE700_IRQ_SEC_SYS_TIMER		34

#define PLAT_MAX_PWR_LVL			2
/*
 * Macros mapping the MPIDR Affinity levels to ARM Platform Power levels. The
 * power levels have a 1:1 mapping with the MPIDR affinity levels.
 */
#define ARM_PWR_LVL0		MPIDR_AFFLVL0
#define ARM_PWR_LVL1		MPIDR_AFFLVL1
#define ARM_PWR_LVL2		MPIDR_AFFLVL2

/*
 *  Macros for local power states in ARM platforms encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define ARM_LOCAL_STATE_RUN	U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define ARM_LOCAL_STATE_RET	U(1)
/* Local power state for OFF/power-down. Valid for CPU and cluster
 * power domains
 */
#define ARM_LOCAL_STATE_OFF	U(2)

#define PLAT_ARM_TRUSTED_MAILBOX_BASE	ARM_TRUSTED_SRAM_BASE
#define PLAT_ARM_NSTIMER_FRAME_ID	U(1)

#define PLAT_ARM_NS_IMAGE_OFFSET	(ARM_DRAM1_BASE + UL(0x8000000))

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)

/*
 * This macro defines the deepest retention state possible. A higher state
 * ID will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		1

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		2

#define PLATFORM_STACK_SIZE		UL(0x440)

#define ARM_MAP_SHARED_RAM		MAP_REGION_FLAT(		\
						ARM_SHARED_RAM_BASE,	\
						ARM_SHARED_RAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_MAP_NS_DRAM1		MAP_REGION_FLAT(		\
						ARM_NS_DRAM1_BASE,	\
						ARM_NS_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_NS)

#define ARM_MAP_BL_RO			MAP_REGION_FLAT(		\
						BL_CODE_BASE,		\
						BL_CODE_END		\
							- BL_CODE_BASE,	\
						MT_CODE | MT_SECURE),	\
					MAP_REGION_FLAT(		\
						BL_RO_DATA_BASE,	\
						BL_RO_DATA_END		\
						- BL_RO_DATA_BASE,	\
						MT_RO_DATA | MT_SECURE)
#if USE_COHERENT_MEM
#define ARM_MAP_BL_COHERENT_RAM		MAP_REGION_FLAT(		\
						BL_COHERENT_RAM_BASE,	\
						BL_COHERENT_RAM_END	\
						- BL_COHERENT_RAM_BASE,	\
						MT_DEVICE | MT_RW | MT_SECURE)
#endif

#define CORSTONE700_DEVICE_BASE		(0x1A000000)
#define CORSTONE700_DEVICE_SIZE		(0x26000000)
#define CORSTONE700_MAP_DEVICE	MAP_REGION_FLAT(			\
					CORSTONE700_DEVICE_BASE,	\
					CORSTONE700_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

/*
 * Define a list of Group 1 Secure and Group 0 interrupt properties as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,	\
		(grp), GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,	\
		(grp), GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,	\
		(grp), GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,	\
		(grp), GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,	\
		(grp), GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,	\
		(grp), GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,	\
		(grp), GIC_INTR_CFG_EDGE)

#define ARM_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_EDGE)

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_ARM_G1S_IRQ_PROPS(grp)	\
	ARM_G1S_IRQ_PROPS(grp),	\
	INTR_PROP_DESC(CORSTONE700_IRQ_TZ_WDOG,	\
		GIC_HIGHEST_SEC_PRIORITY, (grp), GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(CORSTONE700_IRQ_SEC_SYS_TIMER,	\
		GIC_HIGHEST_SEC_PRIORITY, (grp), GIC_INTR_CFG_LEVEL)	\

#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#endif /* PLATFORM_DEF_H */
