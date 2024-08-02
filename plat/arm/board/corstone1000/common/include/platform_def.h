/*
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/arm/board/common/rotpk/rotpk_def.h>
#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/arm_spm_def.h>
#include <plat/arm/common/smccc_def.h>
#include <plat/common/common_def.h>
#include <plat/arm/soc/common/soc_css_def.h>

/* Special value used to verify platform parameters from BL2 to BL31 */
#define ARM_BL31_PLAT_PARAM_VAL		ULL(0x0f1e2d3c4b5a6978)

/* PL011 UART related constants */
#ifdef V2M_IOFPGA_UART0_CLK_IN_HZ
#undef V2M_IOFPGA_UART0_CLK_IN_HZ
#endif

#ifdef V2M_IOFPGA_UART1_CLK_IN_HZ
#undef V2M_IOFPGA_UART1_CLK_IN_HZ
#endif

#define V2M_IOFPGA_UART0_CLK_IN_HZ	50000000
#define V2M_IOFPGA_UART1_CLK_IN_HZ	50000000

/* Core/Cluster/Thread counts for corstone1000 */
#define CORSTONE1000_CLUSTER_COUNT		U(1)
#define CORSTONE1000_MAX_CPUS_PER_CLUSTER	U(4)
#define CORSTONE1000_MAX_PE_PER_CPU		U(1)
#define CORSTONE1000_PRIMARY_CPU		U(0)

#define PLAT_ARM_CLUSTER_COUNT		CORSTONE1000_CLUSTER_COUNT

#define PLATFORM_CORE_COUNT		(PLAT_ARM_CLUSTER_COUNT * \
					 CORSTONE1000_MAX_CPUS_PER_CLUSTER * \
					 CORSTONE1000_MAX_PE_PER_CPU)

/* UART related constants */
#define PLAT_ARM_BOOT_UART_BASE		0x1a510000
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	V2M_IOFPGA_UART0_CLK_IN_HZ
#define PLAT_ARM_RUN_UART_BASE		0x1a520000
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	V2M_IOFPGA_UART1_CLK_IN_HZ
#define ARM_CONSOLE_BAUDRATE		115200
#define PLAT_ARM_CRASH_UART_BASE	PLAT_ARM_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	PLAT_ARM_RUN_UART_CLK_IN_HZ

/* Memory related constants */

/* Memory mappings of where the BLs in the FIP are copied to
 *
 * <ARM_TRUSTED_SRAM_BASE> = 0x02000000
 *	partition size: sizeof(meminfo_t) = 16 bytes
 *	content: memory info area used by the next BL
 *
 * <ARM_FW_CONFIG_BASE> = 0x02000010
 *	partition size: 4080 bytes
 *
 * <ARM_BL2_MEM_DESC_BASE> = 0x02001000
 *	partition size: 4 KB
 *	content: Area where BL2 copies the images descriptors
 *
 * <ARM_BL_RAM_BASE> = <BL32_BASE> = 0x02002000
 *	partition size: 3752 KB
 *	content: BL32 (optee-os)
 *
 * <CORSTONE1000_TOS_FW_CONFIG_BASE> = 0x023AC000
 *	partition size: 8 KB
 *	content: BL32 config (TOS_FW_CONFIG)
 *
 * <BL31_BASE> = 0x023AE000
 *	partition size: 140 KB
 *	content: BL31
 *
 * <BL2_SIGNATURE_BASE> = 0x023D1000
 *	partition size: 4 KB
 *	content: MCUBOOT data needed to verify TF-A BL2
 *
 * <BL2_BASE> = 0x023D2000
 *	partition size: 176 KB
 *	content: BL2
 *
 * <BL33_BASE> = 0x80000000
 *	partition size: 12 MB
 *	content: BL33 (U-Boot)
 */

/* DDR memory */
#define ARM_DRAM1_BASE		UL(0x80000000)
#define ARM_DRAM1_SIZE		(SZ_2G)  /* 2GB*/
#define ARM_DRAM1_END		(ARM_DRAM1_BASE + ARM_DRAM1_SIZE - 1)

/* DRAM1 and DRAM2 are the same for corstone1000 */
#define ARM_DRAM2_BASE		ARM_DRAM1_BASE
#define ARM_DRAM2_SIZE		ARM_DRAM1_SIZE
#define ARM_DRAM2_END		ARM_DRAM1_END

#define ARM_NS_DRAM1_BASE	ARM_DRAM1_BASE
#define ARM_NS_DRAM1_SIZE	ARM_DRAM1_SIZE
#define ARM_NS_DRAM1_END	(ARM_NS_DRAM1_BASE + ARM_NS_DRAM1_SIZE - 1)

/* The first 8 KB of Trusted SRAM are used as shared memory */
#define ARM_TRUSTED_SRAM_BASE	UL(0x02000000)
#define ARM_SHARED_RAM_SIZE	(SZ_8K)  /* 8 KB */
#define ARM_SHARED_RAM_BASE	ARM_TRUSTED_SRAM_BASE

/* The remaining Trusted SRAM is used to load the BL images */
#define TOTAL_SRAM_SIZE		(SZ_4M)  /* 4 MB */


#define PLAT_ARM_TRUSTED_SRAM_SIZE	(TOTAL_SRAM_SIZE - \
					 ARM_SHARED_RAM_SIZE)

#define PLAT_ARM_MAX_BL2_SIZE	(180 * SZ_1K)  /* 180 KB */

#define PLAT_ARM_MAX_BL31_SIZE	(140 * SZ_1K)  /* 140 KB */

#define ARM_BL_RAM_BASE		(ARM_SHARED_RAM_BASE + ARM_SHARED_RAM_SIZE)
#define ARM_BL_RAM_SIZE		(PLAT_ARM_TRUSTED_SRAM_SIZE - \
				 ARM_SHARED_RAM_SIZE)

#define BL2_SIGNATURE_SIZE	(SZ_4K)  /* 4 KB */

#define BL2_SIGNATURE_BASE	(BL2_LIMIT - PLAT_ARM_MAX_BL2_SIZE)
#define BL2_BASE		(BL2_LIMIT - \
				 PLAT_ARM_MAX_BL2_SIZE + \
				 BL2_SIGNATURE_SIZE)
#define BL2_LIMIT		(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)

#define BL31_BASE		(BL2_SIGNATURE_BASE - PLAT_ARM_MAX_BL31_SIZE)
#define BL31_LIMIT		BL2_SIGNATURE_BASE

#define CORSTONE1000_TOS_FW_CONFIG_BASE		(BL31_BASE - \
						 CORSTONE1000_TOS_FW_CONFIG_SIZE)
#define CORSTONE1000_TOS_FW_CONFIG_SIZE		(SZ_8K)  /* 8 KB */
#define CORSTONE1000_TOS_FW_CONFIG_LIMIT	BL31_BASE

#define BL32_BASE		ARM_BL_RAM_BASE
#define PLAT_ARM_MAX_BL32_SIZE	(CORSTONE1000_TOS_FW_CONFIG_BASE - BL32_BASE)

#define BL32_LIMIT		(BL32_BASE + PLAT_ARM_MAX_BL32_SIZE)

/* SPD_spmd settings */

#define PLAT_ARM_SPMC_BASE	BL32_BASE
#define PLAT_ARM_SPMC_SIZE	PLAT_ARM_MAX_BL32_SIZE

/* NS memory */

#define BL33_BASE		ARM_DRAM1_BASE
#define PLAT_ARM_MAX_BL33_SIZE	(12 * SZ_1M)  /* 12 MB*/
#define BL33_LIMIT		(ARM_DRAM1_BASE + PLAT_ARM_MAX_BL33_SIZE)

/* end of the definition of SRAM memory layout */

/* NOR Flash */

#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(SZ_32M)  /* 32 MB */
#define PLAT_ARM_FIP_MAX_SIZE		UL(0x1ff000)  /* 1.996 MB */
#define PLAT_ARM_FLASH_IMAGE_BASE	UL(0x08000000)
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	PLAT_ARM_FIP_MAX_SIZE
#define PLAT_ARM_FIP_OFFSET_IN_GPT	(0x86000)

/* FIP Information */
#define FIP_SIGNATURE_AREA_SIZE         (0x1000)      /* 4 KB */

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE		(U(1) << ARM_CACHE_WRITEBACK_SHIFT)
#define ARM_CACHE_WRITEBACK_SHIFT	6

/*
 * Define FW_CONFIG area base and limit. Leave enough space for BL2 meminfo.
 * FW_CONFIG is intended to host the device tree. Currently, This area is not
 * used because corstone1000 platform doesn't use a device tree at TF-A level.
 */
#define ARM_FW_CONFIG_BASE	(ARM_SHARED_RAM_BASE + sizeof(meminfo_t))
#define ARM_FW_CONFIG_LIMIT	(ARM_SHARED_RAM_BASE + \
				 (ARM_SHARED_RAM_SIZE >> 1))

/*
 * Boot parameters passed from BL2 to BL31/BL32 are stored here
 */
#define ARM_BL2_MEM_DESC_BASE	ARM_FW_CONFIG_LIMIT
#define ARM_BL2_MEM_DESC_LIMIT	ARM_BL_RAM_BASE

/*
 * The max number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#define ARM_BL_REGIONS		3
#define PLAT_ARM_MMAP_ENTRIES	8
#define MAX_XLAT_TABLES		5
#define MAX_MMAP_REGIONS	(PLAT_ARM_MMAP_ENTRIES + ARM_BL_REGIONS)
#define MAX_IO_DEVICES		2
#define MAX_IO_HANDLES		3
#define MAX_IO_BLOCK_DEVICES	1

/* GIC related constants */
#define PLAT_ARM_GICD_BASE	0x1C010000
#define PLAT_ARM_GICC_BASE	0x1C02F000

/* MHUv2 Secure Channel receiver and sender */
#define PLAT_SDK700_MHU0_SEND	0x1B800000
#define PLAT_SDK700_MHU0_RECV	0x1B810000

/* Timer/watchdog related constants */
#define ARM_SYS_CNTCTL_BASE	UL(0x1a200000)
#define ARM_SYS_CNTREAD_BASE	UL(0x1a210000)
#define ARM_SYS_TIMCTL_BASE	UL(0x1a220000)

#define SECURE_WATCHDOG_ADDR_CTRL_REG	0x1A320000
#define SECURE_WATCHDOG_ADDR_VAL_REG	0x1A320008
#define SECURE_WATCHDOG_MASK_ENABLE	0x01
#define SECURE_WATCHDOG_COUNTDOWN_VAL	0x1000

#define SYS_COUNTER_FREQ_IN_TICKS	UL(50000000)  /* 50MHz */

#define CORSTONE1000_IRQ_TZ_WDOG	32
#define CORSTONE1000_IRQ_SEC_SYS_TIMER	34

#define PLAT_MAX_PWR_LVL	2
/*
 * Macros mapping the MPIDR Affinity levels to ARM Platform Power levels. The
 * power levels have a 1:1 mapping with the MPIDR affinity levels.
 */
#define ARM_PWR_LVL0	MPIDR_AFFLVL0
#define ARM_PWR_LVL1	MPIDR_AFFLVL1
#define ARM_PWR_LVL2	MPIDR_AFFLVL2

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

#if defined(CORSTONE1000_FVP_MULTICORE)
/* The secondary core entrypoint address points to bl31_warm_entrypoint
 * and the address size is 8 bytes */
#define CORSTONE1000_SECONDARY_CORE_ENTRYPOINT_ADDRESS_SIZE    UL(0x8)

#define CORSTONE1000_SECONDARY_CORE_HOLD_BASE	(PLAT_ARM_TRUSTED_MAILBOX_BASE + \
						CORSTONE1000_SECONDARY_CORE_ENTRYPOINT_ADDRESS_SIZE)
#define CORSTONE1000_SECONDARY_CORE_STATE_WAIT	ULL(0)
#define CORSTONE1000_SECONDARY_CORE_STATE_GO	ULL(1)
#define CORSTONE1000_SECONDARY_CORE_HOLD_SHIFT	ULL(3)
#endif

#define PLAT_ARM_NSTIMER_FRAME_ID	U(1)

#define PLAT_ARM_NS_IMAGE_BASE		(BL33_BASE)

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)

/*
 * This macro defines the deepest retention state possible. A higher state
 * ID will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE	1

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE	2

#define PLATFORM_STACK_SIZE	UL(0x440)

#define CORSTONE1000_EXTERNAL_FLASH	MAP_REGION_FLAT( \
					PLAT_ARM_NVM_BASE, \
					PLAT_ARM_NVM_SIZE, \
					MT_DEVICE | MT_RO | MT_SECURE)

#define ARM_MAP_SHARED_RAM	MAP_REGION_FLAT( \
				ARM_SHARED_RAM_BASE, \
				ARM_SHARED_RAM_SIZE, \
				MT_MEMORY | MT_RW | MT_SECURE)

#define ARM_MAP_NS_DRAM1	MAP_REGION_FLAT( \
				ARM_NS_DRAM1_BASE, \
				ARM_NS_DRAM1_SIZE, \
				MT_MEMORY | MT_RW | MT_NS)

#define ARM_MAP_BL_RO		MAP_REGION_FLAT( \
				BL_CODE_BASE, \
				(BL_CODE_END - BL_CODE_BASE), \
				MT_CODE | MT_SECURE), \
				MAP_REGION_FLAT( \
				BL_RO_DATA_BASE, \
				(BL_RO_DATA_END - BL_RO_DATA_BASE), \
				MT_RO_DATA | MT_SECURE)
#if USE_COHERENT_MEM
#define ARM_MAP_BL_COHERENT_RAM		MAP_REGION_FLAT( \
					BL_COHERENT_RAM_BASE, \
					(BL_COHERENT_RAM_END \
					 - BL_COHERENT_RAM_BASE), \
					MT_DEVICE | MT_RW | MT_SECURE)
#endif

/*
 * Map the region for the optional device tree configuration with read and
 * write permissions
 */
#define ARM_MAP_BL_CONFIG_REGION	MAP_REGION_FLAT( \
					ARM_FW_CONFIG_BASE, \
					(ARM_FW_CONFIG_LIMIT \
					 - ARM_FW_CONFIG_BASE), \
					MT_MEMORY | MT_RW | MT_SECURE)

#define CORSTONE1000_DEVICE_BASE	(0x1A000000)
#define CORSTONE1000_DEVICE_SIZE	(0x26000000)
#define CORSTONE1000_MAP_DEVICE		MAP_REGION_FLAT( \
					CORSTONE1000_DEVICE_BASE, \
					CORSTONE1000_DEVICE_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_IRQ_SEC_PHY_TIMER	29

#define ARM_IRQ_SEC_SGI_0	8
#define ARM_IRQ_SEC_SGI_1	9
#define ARM_IRQ_SEC_SGI_2	10
#define ARM_IRQ_SEC_SGI_3	11
#define ARM_IRQ_SEC_SGI_4	12
#define ARM_IRQ_SEC_SGI_5	13
#define ARM_IRQ_SEC_SGI_6	14
#define ARM_IRQ_SEC_SGI_7	15

/*
 * Define a list of Group 1 Secure and Group 0 interrupt properties as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, \
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
#define PLAT_ARM_G1S_IRQ_PROPS(grp)				\
		ARM_G1S_IRQ_PROPS(grp),				\
		INTR_PROP_DESC(CORSTONE1000_IRQ_TZ_WDOG,	\
			GIC_HIGHEST_SEC_PRIORITY,		\
			(grp), GIC_INTR_CFG_LEVEL),		\
		INTR_PROP_DESC(CORSTONE1000_IRQ_SEC_SYS_TIMER,	\
			GIC_HIGHEST_SEC_PRIORITY,		\
			(grp), GIC_INTR_CFG_LEVEL)

#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#endif /* PLATFORM_DEF_H */
