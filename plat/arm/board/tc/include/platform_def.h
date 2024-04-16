/*
 * Copyright (c) 2020-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/arm/board/common/board_css_def.h>
#include <plat/arm/board/common/v2m_def.h>
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
 *  0x8000_0000  ------------------   TC_NS_DRAM1_BASE
 *               |       DTB      |
 *               |      (32K)     |
 *  0x8000_8000  ------------------
 *               | NT_FW_CONFIG   |
 *               |      (4KB)     |
 *  0x8000_9000  ------------------
 *               |       ...      |
 *  0xf8a0_0000  ------------------   TC_NS_FWU_BASE
 *               |    FWU shmem   |
 *               |      (4MB)     |
 *  0xf8e0_0000  ------------------   TC_NS_OPTEE_BASE
 *               |  OP-TEE shmem  |
 *               |      (2MB)     |
 *  0xF900_0000  ------------------   TC_TZC_DRAM1_BASE
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
#define TC_TZC_DRAM1_BASE		(ARM_AP_TZC_DRAM1_BASE -	\
					 TC_TZC_DRAM1_SIZE)
#define TC_TZC_DRAM1_SIZE		(96 * SZ_1M)	/* 96 MB */
#define TC_TZC_DRAM1_END		(TC_TZC_DRAM1_BASE +		\
					 TC_TZC_DRAM1_SIZE - 1)

#define TC_NS_DRAM1_BASE		ARM_DRAM1_BASE
#define TC_NS_DRAM1_SIZE		(ARM_DRAM1_SIZE -		\
					 ARM_TZC_DRAM1_SIZE -		\
					 TC_TZC_DRAM1_SIZE)
#define TC_NS_DRAM1_END			(TC_NS_DRAM1_BASE + TC_NS_DRAM1_SIZE - 1)

#define TC_NS_OPTEE_SIZE		(2 * SZ_1M)
#define TC_NS_OPTEE_BASE		(TC_NS_DRAM1_BASE + TC_NS_DRAM1_SIZE - TC_NS_OPTEE_SIZE)
#define TC_NS_FWU_SIZE			(4 * SZ_1M)
#define TC_NS_FWU_BASE			(TC_NS_OPTEE_BASE - TC_NS_FWU_SIZE)

/*
 * Mappings for TC DRAM1 (non-secure) and TC TZC DRAM1 (secure)
 */
#define TC_MAP_NS_DRAM1		MAP_REGION_FLAT(		\
						TC_NS_DRAM1_BASE,	\
						TC_NS_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_NS)


#define TC_MAP_TZC_DRAM1		MAP_REGION_FLAT(		\
						TC_TZC_DRAM1_BASE,	\
						TC_TZC_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)

#define PLAT_HW_CONFIG_DTB_BASE	TC_NS_DRAM1_BASE
#define PLAT_ARM_HW_CONFIG_SIZE	ULL(0x8000)

#define PLAT_DTB_DRAM_NS MAP_REGION_FLAT(	\
					PLAT_HW_CONFIG_DTB_BASE,	\
					PLAT_ARM_HW_CONFIG_SIZE,	\
					MT_MEMORY | MT_RO | MT_NS)
/*
 * Max size of SPMC is 2MB for tc. With SPMD enabled this value corresponds to
 * max size of BL32 image.
 */
#if defined(SPD_spmd)
#define TC_EL2SPMC_LOAD_ADDR		(TC_TZC_DRAM1_BASE + 0x04000000)

#define PLAT_ARM_SPMC_BASE		TC_EL2SPMC_LOAD_ADDR
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
# define PLAT_ARM_MAX_BL2_SIZE		0x29000


/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE is
 * calculated using the current BL31 PROGBITS debug size plus the sizes of
 * BL2 and BL1-RW. Current size is considering that TRUSTED_BOARD_BOOT and
 * MEASURED_BOOT is enabled.
 */
#define PLAT_ARM_MAX_BL31_SIZE		0x60000

/*
 * Size of cacheable stacks
 */
#if defined(IMAGE_BL1)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		0x1000
# else
#  define PLATFORM_STACK_SIZE		0x440
# endif
#elif defined(IMAGE_BL2)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		0x1000
# else
#  define PLATFORM_STACK_SIZE		0x400
# endif
#elif defined(IMAGE_BL2U)
# define PLATFORM_STACK_SIZE		0x400
#elif defined(IMAGE_BL31)
# if SPM_MM
#  define PLATFORM_STACK_SIZE		0x500
# else
#  define PLATFORM_STACK_SIZE		0xa00
# endif
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE		0x440
#endif

/*
 * In the current implementation the RoT Service request that requires the
 * biggest message buffer is the RSS_DELEGATED_ATTEST_GET_PLATFORM_TOKEN. The
 * maximum required buffer size is calculated based on the platform-specific
 * needs of this request.
 */
#define PLAT_RSS_COMMS_PAYLOAD_MAX_SIZE	0x500

#define TC_DEVICE_BASE			0x21000000
#define TC_DEVICE_SIZE			0x5f000000

#if defined(TARGET_FLAVOUR_FPGA)
#undef V2M_FLASH0_BASE
#undef V2M_FLASH0_SIZE
#define V2M_FLASH0_BASE			UL(0x0C000000)
#define V2M_FLASH0_SIZE			UL(0x02000000)
#endif

// TC_MAP_DEVICE covers different peripherals
// available to the platform
#define TC_MAP_DEVICE	MAP_REGION_FLAT(		\
					TC_DEVICE_BASE,	\
					TC_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)


#define TC_FLASH0_RO	MAP_REGION_FLAT(V2M_FLASH0_BASE,\
						V2M_FLASH0_SIZE,	\
						MT_DEVICE | MT_RO | MT_SECURE)

#define PLAT_ARM_NSTIMER_FRAME_ID	0

#define PLAT_ARM_TRUSTED_ROM_BASE	0x0

/* PLAT_ARM_TRUSTED_ROM_SIZE 512KB minus ROM base. */
#define PLAT_ARM_TRUSTED_ROM_SIZE	(0x00080000 - PLAT_ARM_TRUSTED_ROM_BASE)

#define PLAT_ARM_NSRAM_BASE		0x06000000
#if TARGET_FLAVOUR_FVP
#define PLAT_ARM_NSRAM_SIZE		0x00080000	/* 512KB */
#else /* TARGET_FLAVOUR_FPGA */
#define PLAT_ARM_NSRAM_SIZE		0x00008000	/* 64KB */
#endif /* TARGET_FLAVOUR_FPGA */

#if TARGET_PLATFORM <= 2
#define PLAT_ARM_DRAM2_BASE		ULL(0x8080000000)
#elif TARGET_PLATFORM == 3
#define PLAT_ARM_DRAM2_BASE		ULL(0x880000000)
#endif /* TARGET_PLATFORM == 3 */
#define PLAT_ARM_DRAM2_SIZE		ULL(0x180000000)
#define PLAT_ARM_DRAM2_END		(PLAT_ARM_DRAM2_BASE + PLAT_ARM_DRAM2_SIZE - 1ULL)

#define TC_NS_MTE_SIZE			(256 * SZ_1M)
/* the SCP puts the carveout at the end of DRAM2 */
#define TC_NS_DRAM2_SIZE		(PLAT_ARM_DRAM2_SIZE - TC_NS_MTE_SIZE)

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
/* Index of SDS region used in the communication with RSS */
#define SDS_RSS_AP_REGION_ID		U(1)
/*
 * Memory region for RSS's shared data storage (SDS)
 * It is placed right after the SCMI payload area.
 */
#define PLAT_ARM_RSS_AP_SDS_MEM_BASE	(CSS_SCMI_PAYLOAD_BASE + \
					 CSS_SCMI_PAYLOAD_SIZE_MAX)

#define PLAT_ARM_CLUSTER_COUNT		U(1)
#if TARGET_FLAVOUR_FPGA && TARGET_PLATFORM == 2
#define PLAT_MAX_CPUS_PER_CLUSTER	U(14)
#else /* TARGET_FLAVOUR_FPGA && TARGET_PLATFORM == 2 */
#define PLAT_MAX_CPUS_PER_CLUSTER	U(8)
#endif /* TARGET_FLAVOUR_FPGA && TARGET_PLATFORM == 2 */
#define PLAT_MAX_PE_PER_CPU		U(1)

#define PLATFORM_CORE_COUNT		(PLAT_MAX_CPUS_PER_CLUSTER * PLAT_ARM_CLUSTER_COUNT)

/* Message Handling Unit (MHU) base addresses */
#if TARGET_PLATFORM <= 2
	#define PLAT_CSS_MHU_BASE		UL(0x45400000)
#elif TARGET_PLATFORM == 3
	#define PLAT_CSS_MHU_BASE		UL(0x46000000)
#endif /* TARGET_PLATFORM == 3 */
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

/* TC2: AP<->RSS MHUs */
#define PLAT_RSS_AP_SND_MHU_BASE	UL(0x2A840000)
#define PLAT_RSS_AP_RCV_MHU_BASE	UL(0x2A850000)

#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1

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
#define PLAT_CSS_MAX_SCP_BL2_SIZE	0x20000

/*
 * PLAT_CSS_MAX_SCP_BL2U_SIZE is calculated using the current
 * SCP_BL2U size plus a little space for growth.
 */
#define PLAT_CSS_MAX_SCP_BL2U_SIZE	0x20000

/* TZC Related Constants */
#define PLAT_ARM_TZC_BASE		UL(0x25000000)
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT(0)

#define TZC400_OFFSET			UL(0x1000000)
#define TZC400_COUNT			4

#define TZC400_BASE(n)			(PLAT_ARM_TZC_BASE + \
					 (n * TZC400_OFFSET))

#define TZC_NSAID_DEFAULT		U(0)

#define PLAT_ARM_TZC_NS_DEV_ACCESS	\
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_DEFAULT))

/*
 * The first region below, TC_TZC_DRAM1_BASE (0xf9000000) to
 * ARM_SCP_TZC_DRAM1_END (0xffffffff) will mark the last 112 MB of DRAM as
 * secure. The second and third regions gives non secure access to rest of DRAM.
 */
#define TC_TZC_REGIONS_DEF	\
	{TC_TZC_DRAM1_BASE, ARM_SCP_TZC_DRAM1_END,	\
		TZC_REGION_S_RDWR, PLAT_ARM_TZC_NS_DEV_ACCESS},	\
	{TC_NS_DRAM1_BASE, TC_NS_DRAM1_END, ARM_TZC_NS_DRAM_S_ACCESS,	\
		PLAT_ARM_TZC_NS_DEV_ACCESS},	\
	{PLAT_ARM_DRAM2_BASE, PLAT_ARM_DRAM2_END,	\
		ARM_TZC_NS_DRAM_S_ACCESS, PLAT_ARM_TZC_NS_DEV_ACCESS}

/* virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xc0000000)

#if ARM_GPT_SUPPORT
/*
 * This overrides the default PLAT_ARM_FIP_OFFSET_IN_GPT in board_css_def.h.
 * Offset of the FIP in the GPT image. BL1 component uses this option
 * as it does not load the partition table to get the FIP base
 * address. At sector 48 for TC to align with ATU page size boundaries (8KiB)
 * (i.e. after reserved sectors 0-47).
 * Offset = 48 * 512 = 0x6000
 */
#undef PLAT_ARM_FIP_OFFSET_IN_GPT
#define PLAT_ARM_FIP_OFFSET_IN_GPT		0x6000
#endif /* ARM_GPT_SUPPORT */

/* UART related constants */

#define TC_UART0			0x2a400000
#define TC_UART1			0x2a410000

/*
 * TODO: if any more undefs are needed, it's better to consider dropping the
 * board_css_def.h include above
 */
#undef PLAT_ARM_BOOT_UART_BASE
#undef PLAT_ARM_RUN_UART_BASE

#undef PLAT_ARM_CRASH_UART_BASE
#undef PLAT_ARM_BOOT_UART_CLK_IN_HZ
#undef PLAT_ARM_RUN_UART_CLK_IN_HZ

#if TARGET_FLAVOUR_FVP
#define PLAT_ARM_BOOT_UART_BASE		TC_UART1
#define TC_UARTCLK			7372800
#else /* TARGET_FLAVOUR_FPGA */
#define PLAT_ARM_BOOT_UART_BASE		TC_UART0
#if TARGET_PLATFORM <= 2
#define TC_UARTCLK			5000000
#elif TARGET_PLATFORM >= 3
#define TC_UARTCLK			3750000
#endif /* TARGET_PLATFORM >= 3 */
#undef  ARM_CONSOLE_BAUDRATE
#define ARM_CONSOLE_BAUDRATE		38400
#endif /* TARGET_FLAVOUR_FPGA */

#define PLAT_ARM_RUN_UART_BASE		TC_UART0
#define PLAT_ARM_CRASH_UART_BASE	PLAT_ARM_RUN_UART_BASE

#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	TC_UARTCLK
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	TC_UARTCLK

#endif /* PLATFORM_DEF_H */
