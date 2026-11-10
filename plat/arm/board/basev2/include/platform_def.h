/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * BaseV2 AEM configuration: 4 cores per cluster, 2 clusters.
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/arm/board/common/board_css_def.h>
#include <plat/arm/board/common/v2m_def.h>

/*
 * arm_def.h depends on the platform system counter macros, so must define the
 * platform macros before including arm_def.h.
 */
#ifdef ARM_SYS_CNTCTL_BASE
#error "error: ARM_SYS_CNTCTL_BASE is defined prior to the PLAT_ARM_SYS_CNTCTL_BASE definition"
#endif
#define PLAT_ARM_SYS_CNTCTL_BASE	UL(0x47000000)
#define PLAT_ARM_SYS_CNTREAD_BASE	UL(0x47010000)

#include <plat/arm/common/arm_def.h>

#include <plat/arm/common/arm_spm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <plat/arm/soc/common/soc_css_def.h>
#include <plat/common/common_def.h>

#define PLAT_ARM_TRUSTED_SRAM_SIZE	0x00080000	/* 512 KB */

/*
 * The top 16MB of ARM_DRAM1 is configured as secure access only using the TZC,
 * its base is ARM_AP_TZC_DRAM1_BASE.
 *
 * Reserve 96 MB below ARM_AP_TZC_DRAM1_BASE for:
 *   - BL32_BASE when SPD_spmd is enabled
 *   - Region to load secure partitions
 *
 *
 *  0x8000_0000  ------------------   BASE_NS_DRAM1_BASE
 *               |       DTB      |
 *               |      (32K)     |
 *  0x8000_8000  ------------------
 *               | NT_FW_CONFIG   |
 *               |      (4KB)     |
 *  0x8000_9000  ------------------
 *               |       ...      |
 *  0xf8e0_0000  ------------------   BASE_NS_OPTEE_BASE
 *               |  OP-TEE shmem  |
 *               |      (2MB)     |
 *  0xF900_0000  ------------------   BASE_TZC_DRAM1_BASE
 *               |                |
 *               |      SPMC      |
 *               |       SP       |
 *               |     (96MB)     |
 *  0xFF00_0000  ------------------   ARM_AP_TZC_DRAM1_BASE
 *               |       AP       |
 *               |   EL3 Monitor  |
 *               |       SCP      |
 *               |     (16MB)     |
 *  0xFFFF_FFFF  ------------------
 *
 *
 */
#define BASE_TZC_DRAM1_BASE		(ARM_AP_TZC_DRAM1_BASE -	\
					 BASE_TZC_DRAM1_SIZE)
#define BASE_TZC_DRAM1_SIZE		(96 * SZ_1M)	/* 96 MB */
#define BASE_TZC_DRAM1_END		(BASE_TZC_DRAM1_BASE +		\
					 BASE_TZC_DRAM1_SIZE - 1)

#define BASE_NS_DRAM1_BASE		ARM_DRAM1_BASE
#define BASE_NS_DRAM1_SIZE		(ARM_DRAM1_SIZE -		\
					 ARM_TZC_DRAM1_SIZE -		\
					 BASE_TZC_DRAM1_SIZE)
#define BASE_NS_DRAM1_END			(BASE_NS_DRAM1_BASE + BASE_NS_DRAM1_SIZE - 1)

#define BASE_NS_OPTEE_SIZE		(2 * SZ_1M)
#define BASE_NS_OPTEE_BASE		(BASE_NS_DRAM1_BASE + BASE_NS_DRAM1_SIZE - BASE_NS_OPTEE_SIZE)

/*
 * Mappings for Base DRAM1 (non-secure) and Base TZC DRAM1 (secure)
 */
#define BASE_MAP_NS_DRAM1		MAP_REGION_FLAT(		\
						BASE_NS_DRAM1_BASE,	\
						BASE_NS_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_NS)


#define BASE_MAP_TZC_DRAM1		MAP_REGION_FLAT(		\
						BASE_TZC_DRAM1_BASE,	\
						BASE_TZC_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)

#define PLAT_HW_CONFIG_DTB_BASE	BASE_NS_DRAM1_BASE
#define PLAT_ARM_HW_CONFIG_SIZE	ULL(0x8000)

#define PLAT_DTB_DRAM_NS MAP_REGION_FLAT(	\
					PLAT_HW_CONFIG_DTB_BASE,	\
					PLAT_ARM_HW_CONFIG_SIZE,	\
					MT_MEMORY | MT_RO | MT_NS)
/*
 * Max size of SPMC is 2MB for BaseV2. With SPMD enabled this value corresponds to
 * max size of BL32 image.
 */
#if defined(SPD_spmd)
#define BASE_EL2SPMC_LOAD_ADDR		(BASE_TZC_DRAM1_BASE + 0x04000000)

#define PLAT_ARM_SPMC_BASE		BASE_EL2SPMC_LOAD_ADDR
#define PLAT_ARM_SPMC_SIZE		UL(0x200000)  /* 2 MB */
#endif

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#if defined(IMAGE_BL31)
# if SPM_MM
#  define PLAT_ARM_MMAP_ENTRIES		9
#  define MAX_XLAT_TABLES		7
#  define PLAT_SP_IMAGE_MMAP_REGIONS	7
#  define PLAT_SP_IMAGE_MAX_XLAT_TABLES	10
# else
#  define PLAT_ARM_MMAP_ENTRIES		8
#  define MAX_XLAT_TABLES		8
# endif
#elif defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		8
# define MAX_XLAT_TABLES		5
#elif !USE_ROMLIB
# define PLAT_ARM_MMAP_ENTRIES		11
# define MAX_XLAT_TABLES		7
#else
# define PLAT_ARM_MMAP_ENTRIES		12
# define MAX_XLAT_TABLES		6
#endif

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#define PLAT_ARM_MAX_BL1_RW_SIZE	0x12000

/*
 * PLAT_ARM_MAX_ROMLIB_RW_SIZE is define to use a full page
 */

#if USE_ROMLIB
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	0x1000
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	0xe000
#else
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	0
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	0
#endif

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth. Current size is considering that TRUSTED_BOARD_BOOT
 * and MEASURED_BOOT is enabled.
 */
# define PLAT_ARM_MAX_BL2_SIZE		0x2C000


/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE is
 * calculated using the current BL31 PROGBITS debug size plus the sizes of
 * BL2 and BL1-RW. Current size is considering that TRUSTED_BOARD_BOOT and
 * MEASURED_BOOT is enabled.
 */
#define PLAT_ARM_MAX_BL31_SIZE		0x64000

/*
 * Size of cacheable stacks
 */
#if defined(IMAGE_BL1)
#  define PLATFORM_STACK_SIZE		0x1000
#elif defined(IMAGE_BL2)
#  define PLATFORM_STACK_SIZE		0x1000
#elif defined(IMAGE_BL2U)
# define PLATFORM_STACK_SIZE		0x400
#elif defined(IMAGE_BL31)
# if SPM_MM
#  define PLATFORM_STACK_SIZE		0x500
# else
#  define PLATFORM_STACK_SIZE		0xb00
# endif
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE		0x440
#endif

/*
 * In the current implementation the RoT Service request that requires the
 * biggest message buffer is the RSE_DELEGATED_ATTEST_GET_PLATFORM_TOKEN. The
 * maximum required buffer size is calculated based on the platform-specific
 * needs of this request.
 */
#define PLAT_RSE_COMMS_PAYLOAD_MAX_SIZE	0x500

/* Core topology configuration - templated based on build configuration */
#define PLAT_ARM_CLUSTER_COUNT		U(2)
#define PLAT_MAX_CPUS_PER_CLUSTER	U(4)
#define PLAT_MAX_PE_PER_CPU		U(1)

#define PLATFORM_CORE_COUNT		(PLAT_MAX_CPUS_PER_CLUSTER * PLAT_ARM_CLUSTER_COUNT)

/* Device base addresses - dynamically allocated based on cluster count */
#define BASE_DEVICE_BASE0			0x21000000
#define BASE_DEVICE_BASE1			0x121000000

#define BASE_DEVICE_SIZE			0x5f000000

// BASE_MAP_DEVICEx covers different peripherals available to the platform
#define BASE_MAP_DEVICE0	MAP_REGION_FLAT(		\
					BASE_DEVICE_BASE0,	\
					BASE_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)
#define BASE_MAP_DEVICE1	MAP_REGION_FLAT(		\
					BASE_DEVICE_BASE1,	\
					BASE_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

#define BASE_FLASH0_RO	MAP_REGION_FLAT(V2M_FLASH0_BASE,\
						V2M_FLASH0_SIZE,	\
						MT_DEVICE | MT_RO | MT_SECURE)

#define BASE_MAP_V2M_IO		MAP_REGION_FLAT(		\
					(V2M_OFFSET + UL(0x1c000000)), \
					UL(0x03000000),		\
					MT_DEVICE | MT_RW | MT_SECURE)
#define PLAT_ARM_NSTIMER_FRAME_ID	U(1)

#define PLAT_ARM_TRUSTED_ROM_BASE	0x0

/* PLAT_ARM_TRUSTED_ROM_SIZE 512KB minus ROM base. */
#define PLAT_ARM_TRUSTED_ROM_SIZE	(0x00080000 - PLAT_ARM_TRUSTED_ROM_BASE)

#define PLAT_ARM_NSRAM_BASE		0x06000000
#define PLAT_ARM_NSRAM_SIZE		0x00080000	/* 512KB */

/*
 *  Memory Layout "Buildroot In RAM" with MTE carveout
 *
 *  0x8800_0000_0  ------------------   DDK_BASE_ADDR
 *                 |                |
 *                 |      DDK       |
 *                 |   (512 MB)     |
 *  0x8A00_0000_0  ------------------   FREE_REGION_BASE_ADDR
 *                 |                |
 *                 |     Free       |
 *                 |    (13 GB)     |
 *  0xBE00_0000_0  ------------------   MTE_BASE_ADDR
 *                 |                |
 *                 |      MTE       |
 *                 |   TAGS SPACE   |
 *                 |   (512 MB)     |
 *  0xC000_0000_0  ------------------   PLAT_ARM_DRAM2_END
 */

#define BASE_DRAM2_BASE			ULL(0x880000000)
#define BASE_TOTAL_DRAM2_SIZE		ULL(0x380000000)

#define DRAM_FS_SIZE			ULL(0)

#define BASE_MTE_SIZE_TOTAL		ULL(0)

#define PLAT_ARM_DRAM2_BASE		((BASE_DRAM2_BASE) + (DRAM_FS_SIZE))
#define PLAT_ARM_DRAM2_SIZE		((BASE_TOTAL_DRAM2_SIZE) - (DRAM_FS_SIZE) - (BASE_MTE_SIZE_TOTAL))

#define PLAT_ARM_DRAM2_END		(PLAT_ARM_DRAM2_BASE + PLAT_ARM_DRAM2_SIZE - 1ULL)

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_INT_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp),	\
					INTR_PROP_DESC(SBSA_SECURE_WDOG_INTID,	\
						GIC_HIGHEST_SEC_PRIORITY, grp, \
						GIC_INTR_CFG_LEVEL)

#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +	\
					 PLAT_SP_IMAGE_NS_BUF_SIZE)

#define PLAT_ARM_SP_MAX_SIZE		U(0x2000000)

/*******************************************************************************
 * Memprotect definitions
 ******************************************************************************/
/* PSCI memory protect definitions:
 * This variable is stored in a non-secure flash because some ARM reference
 * platforms do not have secure NVRAM. Real systems that provided MEM_PROTECT
 * support must use a secure NVRAM to store the PSCI MEM_PROTECT definitions.
 */
#define PLAT_ARM_MEM_PROT_ADDR		(V2M_FLASH0_BASE + \
					 V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

/* Secure Watchdog Constants */
#define SBSA_SECURE_WDOG_CONTROL_BASE	UL(0x2A480000)
#define SBSA_SECURE_WDOG_REFRESH_BASE	UL(0x2A490000)
#define SBSA_SECURE_WDOG_TIMEOUT	UL(100)
#define SBSA_SECURE_WDOG_INTID		86

#define PLAT_ARM_SCMI_CHANNEL_COUNT	1

/* Index of SDS region used in the communication with SCP */
#define SDS_SCP_AP_REGION_ID		U(0)
/* Index of SDS region used in the communication with RSE */
#define SDS_RSE_AP_REGION_ID		U(1)
/*
 * Memory region for RSE's shared data storage (SDS)
 * It is placed right after the SCMI payload area.
 */
#define PLAT_ARM_RSE_AP_SDS_MEM_BASE	(CSS_SCMI_PAYLOAD_BASE + \
					 CSS_SCMI_PAYLOAD_SIZE_MAX)

#define PLAT_NUM_PWR_DOMAINS		(ARM_SYSTEM_COUNT + \
					 PLAT_ARM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

/* Message Handling Unit (MHU) base addresses */
#define PLAT_CSS_MHU_BASE		UL(0x46000000)
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

/* AP<->RSS MHUs */

#define PLAT_RSE_AP_SND_MHU_BASE	UL(0x49000000)
#define PLAT_RSE_AP_RCV_MHU_BASE	UL(0x49010000)

#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL2

/*
 * Physical and virtual address space limits for MMU in AARCH64
 */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 36)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE		UL(0x30000000)
#define PLAT_ARM_GICC_BASE		UL(0x2C000000)
#define PLAT_ARM_GICR_BASE		UL(0x30080000)

/*
 * PLAT_CSS_MAX_SCP_BL2_SIZE is calculated using the current
 * SCP_BL2 size plus a little space for growth.
 */
#define PLAT_CSS_MAX_SCP_BL2_SIZE	0x30000

/*
 * PLAT_CSS_MAX_SCP_BL2U_SIZE is calculated using the current
 * SCP_BL2U size plus a little space for growth.
 */
#define PLAT_CSS_MAX_SCP_BL2U_SIZE	0x30000

/* virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xc0000000)

#if ARM_GPT_SUPPORT
/*
 * This overrides the default PLAT_ARM_FIP_OFFSET_IN_GPT in board_css_def.h.
 * Offset of the FIP in the GPT image. BL1 component uses this option
 * as it does not load the partition table to get the FIP base
 * address. At sector 48 for Base to align with ATU page size boundaries (8KiB)
 * (i.e. after reserved sectors 0-47).
 * Offset = 48 * 512 = 0x6000
 */
#undef PLAT_ARM_FIP_OFFSET_IN_GPT
#define PLAT_ARM_FIP_OFFSET_IN_GPT		0x6000
#endif /* ARM_GPT_SUPPORT */

/* UART related constants */

/*
 * TODO: if any more undefs are needed, it's better to consider dropping the
 * board_css_def.h include above
 */
#undef PLAT_ARM_BOOT_UART_BASE
#undef PLAT_ARM_RUN_UART_BASE

#undef PLAT_ARM_CRASH_UART_BASE
#undef PLAT_ARM_BOOT_UART_CLK_IN_HZ
#undef PLAT_ARM_RUN_UART_CLK_IN_HZ

#undef  ARM_CONSOLE_BAUDRATE
#define ARM_CONSOLE_BAUDRATE		38400

#define BASE_UART1			0x2a410000

#define BASE_UARTCLK			4000000

#define PLAT_ARM_BOOT_UART_BASE		BASE_UART1

#define PLAT_ARM_RUN_UART_BASE		PLAT_ARM_BOOT_UART_BASE
#define PLAT_ARM_CRASH_UART_BASE	PLAT_ARM_RUN_UART_BASE

#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	BASE_UARTCLK
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	BASE_UARTCLK

#define NCI_BASE_ADDR			UL(0x4F000000)

#endif /* PLATFORM_DEF_H */
