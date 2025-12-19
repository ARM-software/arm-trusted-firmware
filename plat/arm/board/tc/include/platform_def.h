/*
 * Copyright (c) 2020-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <cortex_a520.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/arm/board/common/board_css_def.h>
#include <plat/arm/board/common/v2m_def.h>

/*
 * arm_def.h depends on the platform system counter macros, so must define the
 * platform macros before including arm_def.h.
 */
#if TARGET_PLATFORM == 4
#ifdef ARM_SYS_CNTCTL_BASE
#error "error: ARM_SYS_CNTCTL_BASE is defined prior to the PLAT_ARM_SYS_CNTCTL_BASE definition"
#endif
#define PLAT_ARM_SYS_CNTCTL_BASE	UL(0x47000000)
#define PLAT_ARM_SYS_CNTREAD_BASE	UL(0x47010000)
#endif

#include <plat/arm/common/arm_def.h>

#include <plat/arm/common/arm_spm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <plat/arm/soc/common/soc_css_def.h>
#include <plat/common/common_def.h>

#define PLAT_ARM_TRUSTED_SRAM_SIZE	0x00080000	/* 512 KB */

#if TRANSFER_LIST
/*
 * Summation of data size of all Transfer Entries included in the Transfer list.
 * Note: Update this field whenever new Transfer Entries are added in future.
 */
#define PLAT_ARM_FW_HANDOFF_SIZE	U(0x9000)
#define PLAT_ARM_EL3_FW_HANDOFF_BASE	ARM_BL_RAM_BASE
#define PLAT_ARM_EL3_FW_HANDOFF_LIMIT	PLAT_ARM_EL3_FW_HANDOFF_BASE + PLAT_ARM_FW_HANDOFF_SIZE
#define FW_NS_HANDOFF_BASE		(PLAT_ARM_NS_IMAGE_BASE - PLAT_ARM_FW_HANDOFF_SIZE)

/* Mappings for Secure and Non-secure Transfer_list */
#define TC_MAP_EL3_FW_HANDOFF		MAP_REGION_FLAT(		\
					PLAT_ARM_EL3_FW_HANDOFF_BASE,	\
					PLAT_ARM_FW_HANDOFF_SIZE,	\
					MT_MEMORY | MT_RW | EL3_PAS)

#define TC_MAP_FW_NS_HANDOFF		MAP_REGION_FLAT(		\
					FW_NS_HANDOFF_BASE,		\
					PLAT_ARM_FW_HANDOFF_SIZE,	\
					MT_MEMORY | MT_RW | MT_NS)
#endif /* TRANSFER_LIST */

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

#define TC_DEVICE_BASE			0x21000000
#define TC_DEVICE_SIZE			0x5f000000

#if defined(TARGET_FLAVOUR_FPGA)
#undef V2M_FLASH0_BASE
#undef V2M_FLASH0_SIZE
#if TC_FPGA_FIP_IMG_IN_RAM
/*
 * Note that this is just used for the FIP, which is not required
 * anymore once Linux has commenced booting. So we are safe allowing
 * Linux to also make use of this memory and it doesn't need to be
 * carved out of the devicetree.
 *
 * This only needs to match the RAM load address that we give the FIP
 * on either the FPGA or FVP command line so there is no need to link
 * it to say halfway through the RAM or anything like that.
 */
#define V2M_FLASH0_BASE			UL(0xB0000000)
#else
#define V2M_FLASH0_BASE			UL(0x0C000000)
#endif
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
#define PLAT_ARM_NSTIMER_FRAME_ID	U(1)

#define PLAT_ARM_TRUSTED_ROM_BASE	0x0

/* PLAT_ARM_TRUSTED_ROM_SIZE 512KB minus ROM base. */
#define PLAT_ARM_TRUSTED_ROM_SIZE	(0x00080000 - PLAT_ARM_TRUSTED_ROM_BASE)

#define PLAT_ARM_NSRAM_BASE		0x06000000
#if TARGET_FLAVOUR_FVP
#define PLAT_ARM_NSRAM_SIZE		0x00080000	/* 512KB */
#else /* TARGET_FLAVOUR_FPGA */
#define PLAT_ARM_NSRAM_SIZE		0x00008000	/* 64KB */
#endif /* TARGET_FLAVOUR_FPGA */

/*
 *  Memory Layout "Android Loaded into MMC Card" with MTE carveout
 *
 *  0x8_8000_0000  ------------------   PLAT_ARM_DRAM2_BASE
 *                 |                |
 *                 |  TC_NS_DRAM2   |
 *                 |    (13.5GB)    |
 *                 |                |
 *                 |                |
 *                 |                |
 *                 |                |
 *  0xB_E000_0000  ------------------   PLAT_ARM_DRAM2_END
 *                 |      MTE       |
 *                 |   TAGS SPACE   |
 *                 |    (512MB)     |
 *  0xC_0000_0000  ------------------
 *
 *  ********************************************************
 *
 *  Memory Layout "Android Loaded into MMC Card" without MTE carveout
 *
 *  0x8_8000_0000  ------------------   PLAT_ARM_DRAM2_BASE
 *                 |                |
 *                 |  TC_NS_DRAM2   |
 *                 |    (14GB)      |
 *                 |                |
 *                 |                |
 *                 |                |
 *                 |                |
 *  0xC_0000_0000  ------------------   PLAT_ARM_DRAM2_END
 *
 *  ********************************************************
 *
 *  Memory Layout "Android In RAM" with MTE carveout
 *
 *  0x8_8000_0000  ------------------   ANDROID_FS_BASE
 *                 |                |
 *                 |                |
 *                 |  ANDROID_IMG   |
 *                 |    (8.5GB)     |
 *                 |                |
 *                 |                |
 *  0xA_A000_0000  ------------------   PLAT_ARM_DRAM2_BASE
 *                 |                |
 *                 |  TC_NS_DRAM2   |
 *                 |     (5GB)      |
 *                 |                |
 *  0xB_E000_0000  ------------------   PLAT_ARM_DRAM2_END
 *                 |      MTE       |
 *                 |   TAGS SPACE   |
 *                 |    (512MB)     |
 *  0xC_0000_0000  ------------------
 */

#define TC_DRAM2_BASE			ULL(0x880000000)
#define TC_TOTAL_DRAM2_SIZE		ULL(0x380000000)

#if TC_FPGA_FS_IMG_IN_RAM
/* 8.5GB reserved for system+userdata+vendor images */
#define SYSTEM_IMAGE_SIZE		ULL(0xC0000000)		/* 3GB */
#define USERDATA_IMAGE_SIZE		ULL(0x140000000)	/* 5GB */
#define VENDOR_IMAGE_SIZE		ULL(0x20000000)		/* 512MB */
#define ANDROID_FS_SIZE			(SYSTEM_IMAGE_SIZE + \
					USERDATA_IMAGE_SIZE + \
					VENDOR_IMAGE_SIZE)
#else
#define ANDROID_FS_SIZE			ULL(0)
#endif /* TC_FPGA_FS_IMG_IN_RAM */

#if defined(TARGET_FLAVOUR_FPGA) && (TARGET_PLATFORM == 4)
/* To make optimal use of memory, set this to the address equivalent to the top
 * 3.125% of the available downstream size.
 * Note that this assumes total memory of 16GiB split across 8 MCN nodes.
 */
#define TC_MTU_TAG_ADDR_BASE		ULL(0x7C000000)

/* Calculate total amount of RAM given over to MTE carveout based on the carveout
 * address
 */
#define TC_TOTAL_DRAM_AVAILABLE		(TC_TOTAL_DRAM2_SIZE + ARM_DRAM1_SIZE)
#define TC_DRAM_SIZE_PER_MCN_INST	((TC_TOTAL_DRAM_AVAILABLE) / (MCN_INSTANCES))
#define TC_MTE_SIZE_PER_MCN_INST	(TC_DRAM_SIZE_PER_MCN_INST - TC_MTU_TAG_ADDR_BASE)
#define TC_MTE_SIZE_TOTAL		((TC_MTE_SIZE_PER_MCN_INST) * (MCN_INSTANCES))
#else
#define TC_MTE_SIZE_TOTAL		ULL(0)
#endif /* defined(TARGET_FLAVOUR_FPGA) && (TARGET_PLATFORM == 4) */

#define PLAT_ARM_DRAM2_BASE		((TC_DRAM2_BASE) + (ANDROID_FS_SIZE))
#define PLAT_ARM_DRAM2_SIZE				\
	((TC_TOTAL_DRAM2_SIZE) - (ANDROID_FS_SIZE) - (TC_MTE_SIZE_TOTAL))

#define PLAT_ARM_DRAM2_END		(PLAT_ARM_DRAM2_BASE + PLAT_ARM_DRAM2_SIZE)

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_INT_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp),	\
					INTR_PROP_DESC(SBSA_SECURE_WDOG_INTID,	\
						GIC_HIGHEST_SEC_PRIORITY, grp, \
						GIC_INTR_CFG_LEVEL)

#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SPM_BUF_BASE +	\
					 PLAT_SPM_BUF_SIZE)

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

#define PLAT_ARM_CLUSTER_COUNT		U(1)
#define PLAT_MAX_CPUS_PER_CLUSTER	U(8)
#define PLAT_MAX_PE_PER_CPU		U(1)

#define PLATFORM_CORE_COUNT		(PLAT_MAX_CPUS_PER_CLUSTER * PLAT_ARM_CLUSTER_COUNT)

/* Message Handling Unit (MHU) base addresses */
#define PLAT_CSS_MHU_BASE		UL(0x46000000)
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

/* AP<->RSS MHUs */
#if TARGET_PLATFORM == 3
#define PLAT_RSE_AP_SND_MHU_BASE	UL(0x49000000)
#define PLAT_RSE_AP_RCV_MHU_BASE	UL(0x49100000)
#elif TARGET_PLATFORM == 4
#define PLAT_RSE_AP_SND_MHU_BASE	UL(0x49000000)
#define PLAT_RSE_AP_RCV_MHU_BASE	UL(0x49010000)
#endif

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

#undef  ARM_CONSOLE_BAUDRATE
#define ARM_CONSOLE_BAUDRATE		38400

#if TARGET_PLATFORM == 3
#define TC_UARTCLK			3750000
#elif TARGET_PLATFORM == 4
#define TC_UARTCLK			4000000
#endif /* TARGET_PLATFORM == 3 */


#if TARGET_FLAVOUR_FVP
#define PLAT_ARM_BOOT_UART_BASE		TC_UART1
#else /* TARGET_FLAVOUR_FPGA */
#define PLAT_ARM_BOOT_UART_BASE		TC_UART0
#endif /* TARGET_FLAVOUR_FPGA */

#define PLAT_ARM_RUN_UART_BASE		TC_UART0
#define PLAT_ARM_CRASH_UART_BASE	PLAT_ARM_RUN_UART_BASE

#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	TC_UARTCLK
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	TC_UARTCLK

#define NCI_BASE_ADDR			UL(0x4F000000)
#if (TARGET_PLATFORM == 3) && defined(TARGET_FLAVOUR_FPGA)
#define MCN_ADDRESS_SPACE_SIZE		0x00120000
#else
#define MCN_ADDRESS_SPACE_SIZE		0x00130000
#endif	/* (TARGET_PLATFORM == 3) && defined(TARGET_FLAVOUR_FPGA) */
#if TARGET_PLATFORM == 3
#define MCN_OFFSET_IN_NCI		0x00C90000
#else	/* TARGET_PLATFORM == 4 */
#ifdef TARGET_FLAVOUR_FPGA
#define MCN_OFFSET_IN_NCI		0x00420000
#else
#define MCN_OFFSET_IN_NCI		0x00D80000
#endif	/* TARGET_FLAVOUR_FPGA */
#endif	/* TARGET_PLATFORM == 3 */
#define MCN_BASE_ADDR(n)		(NCI_BASE_ADDR + MCN_OFFSET_IN_NCI + \
								((n) * MCN_ADDRESS_SPACE_SIZE))
#define MCN_PMU_OFFSET			0x000C4000
#define MCN_MICROARCH_OFFSET		0x000E4000
#define MCN_MICROARCH_BASE_ADDR(n)		(MCN_BASE_ADDR(n) + \
										MCN_MICROARCH_OFFSET)
#define MCN_SCR_OFFSET			0x4
#define MCN_SCR_PMU_BIT			10
#if TARGET_PLATFORM == 3
#define MCN_INSTANCES			4
#else	/* TARGET_PLATFORM == 4 */
#define MCN_INSTANCES			8
#endif	/* TARGET_PLATFORM == 3 */
#define MCN_PMU_ADDR(n)			(MCN_BASE_ADDR(n) + \
								MCN_PMU_OFFSET)
#define MCN_MPAM_NS_OFFSET		0x000D0000
#define MCN_MPAM_NS_BASE_ADDR(n)		(MCN_BASE_ADDR(n) + MCN_MPAM_NS_OFFSET)
#define MCN_MPAM_S_OFFSET		0x000D4000
#define MCN_MPAM_S_BASE_ADDR(n)		(MCN_BASE_ADDR(n) + MCN_MPAM_S_OFFSET)
#define MPAM_SLCCFG_CTL_OFFSET		0x00003018
#define SLC_RDALLOCMODE_SHIFT		8
#define SLC_RDALLOCMODE_MASK		(3 << SLC_RDALLOCMODE_SHIFT)
#define SLC_WRALLOCMODE_SHIFT		12
#define SLC_WRALLOCMODE_MASK		(3 << SLC_WRALLOCMODE_SHIFT)

#define SLC_DONT_ALLOC			0
#define SLC_ALWAYS_ALLOC		1
#define SLC_ALLOC_BUS_SIGNAL_ATTR	2

#define MCN_CONFIG_OFFSET		0x204
#define MCN_CONFIG_ADDR(n)			(MCN_BASE_ADDR(n) + MCN_CONFIG_OFFSET)
#define MCN_CONFIG_SLC_PRESENT_BIT	3

#define MCN_MTU_OFFSET			0x44000
#define MCN_MTU_BASE_ADDR(n)		(MCN_BASE_ADDR(n) + MCN_MTU_OFFSET)
#define MTU_TAG_ADDR_BASE_OFFSET	0x0

#define	MCN_CRP_OFFSET			0x24000
#define	MCN_CRP_BASE_ADDR(n)		(MCN_BASE_ADDR(n) + MCN_CRP_OFFSET)
#define	MCN_CRP_ARCH_STATE_REQ_OFFSET	0
#define	MCN_CRP_ARCH_STATE_CUR_OFFSET	8

#define	MCN_CONFIG_STATE		0
#define	MCN_RUN_STATE			1

/*
 * TC3 CPUs have the same definitions for:
 *   CORTEX_{A520|A725|X925}_CPUECTLR_EL1
 *   CORTEX_{A520|A725|X925}_CPUECTLR_EL1_EXTLLC_BIT
 * Define the common macros for easier using.
 */
#define CPUECTLR_EL1			CORTEX_A520_CPUECTLR_EL1
#define CPUECTLR_EL1_EXTLLC_BIT		CORTEX_A520_CPUECTLR_EL1_EXTLLC_BIT

#define CPUACTLR_CLUSTERPMUEN		(ULL(1) << 12)

#endif /* PLATFORM_DEF_H */
