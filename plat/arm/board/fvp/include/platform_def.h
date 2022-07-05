/*
 * Copyright (c) 2014-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <drivers/arm/tzc400.h>
#include <lib/utils_def.h>
#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/arm_def.h>
#include <plat/arm/common/arm_spm_def.h>
#include <plat/common/common_def.h>

#include "../fvp_def.h"

/* Required platform porting definitions */
#define PLATFORM_CORE_COUNT  (U(FVP_CLUSTER_COUNT) * \
			      U(FVP_MAX_CPUS_PER_CLUSTER) * \
			      U(FVP_MAX_PE_PER_CPU))

#define PLAT_NUM_PWR_DOMAINS (U(FVP_CLUSTER_COUNT) + \
			      PLATFORM_CORE_COUNT + U(1))

#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL2

/*
 * Other platform porting definitions are provided by included headers
 */

/*
 * Required ARM standard platform porting definitions
 */
#define PLAT_ARM_CLUSTER_COUNT		U(FVP_CLUSTER_COUNT)

#define PLAT_ARM_TRUSTED_SRAM_SIZE	UL(0x00040000)	/* 256 KB */

#define PLAT_ARM_TRUSTED_ROM_BASE	UL(0x00000000)
#define PLAT_ARM_TRUSTED_ROM_SIZE	UL(0x04000000)	/* 64 MB */

#define PLAT_ARM_TRUSTED_DRAM_BASE	UL(0x06000000)
#define PLAT_ARM_TRUSTED_DRAM_SIZE	UL(0x02000000)	/* 32 MB */

#if ENABLE_RME
#define PLAT_ARM_RMM_BASE		(RMM_BASE)
#define PLAT_ARM_RMM_SIZE		(RMM_LIMIT - RMM_BASE)
#endif

/*
 * Max size of SPMC is 2MB for fvp. With SPMD enabled this value corresponds to
 * max size of BL32 image.
 */
#if defined(SPD_spmd)
#define PLAT_ARM_SPMC_BASE		PLAT_ARM_TRUSTED_DRAM_BASE
#define PLAT_ARM_SPMC_SIZE		UL(0x200000)  /* 2 MB */
#endif

/* virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xc0000000)

/* No SCP in FVP */
#define PLAT_ARM_SCP_TZC_DRAM1_SIZE	UL(0x0)

#define PLAT_ARM_DRAM2_BASE	ULL(0x880000000) /* 36-bit range */
#define PLAT_ARM_DRAM2_SIZE	ULL(0x780000000) /* 30 GB */

#define FVP_DRAM3_BASE	ULL(0x8800000000) /* 40-bit range */
#define FVP_DRAM3_SIZE	ULL(0x7800000000) /* 480 GB */
#define FVP_DRAM3_END	(FVP_DRAM3_BASE + FVP_DRAM3_SIZE - 1U)

#define FVP_DRAM4_BASE	ULL(0x88000000000) /* 44-bit range */
#define FVP_DRAM4_SIZE	ULL(0x78000000000) /* 7.5 TB */
#define FVP_DRAM4_END	(FVP_DRAM4_BASE + FVP_DRAM4_SIZE - 1U)

#define FVP_DRAM5_BASE	ULL(0x880000000000) /* 48-bit range */
#define FVP_DRAM5_SIZE	ULL(0x780000000000) /* 120 TB */
#define FVP_DRAM5_END	(FVP_DRAM5_BASE + FVP_DRAM5_SIZE - 1U)

#define FVP_DRAM6_BASE	ULL(0x8800000000000) /* 52-bit range */
#define FVP_DRAM6_SIZE	ULL(0x7800000000000) /* 1920 TB */
#define FVP_DRAM6_END	(FVP_DRAM6_BASE + FVP_DRAM6_SIZE - 1U)

/* Range of kernel DTB load address */
#define FVP_DTB_DRAM_MAP_START		ULL(0x82000000)
#define FVP_DTB_DRAM_MAP_SIZE		ULL(0x02000000)	/* 32 MB */

#define ARM_DTB_DRAM_NS			MAP_REGION_FLAT(		\
					FVP_DTB_DRAM_MAP_START,		\
					FVP_DTB_DRAM_MAP_SIZE,		\
					MT_MEMORY | MT_RO | MT_NS)

#if SPMC_AT_EL3
/*
 * Number of Secure Partitions supported.
 * SPMC at EL3, uses this count to configure the maximum number of supported
 * secure partitions.
 */
#define SECURE_PARTITION_COUNT		1

/*
 * Number of Normal World Partitions supported.
 * SPMC at EL3, uses this count to configure the maximum number of supported
 * NWd partitions.
 */
#define NS_PARTITION_COUNT		1

/*
 * Number of Logical Partitions supported.
 * SPMC at EL3, uses this count to configure the maximum number of supported
 * logical partitions.
 */
#define MAX_EL3_LP_DESCS_COUNT		1

#endif /* SPMC_AT_EL3 */

/*
 * Load address of BL33 for this platform port
 */
#define PLAT_ARM_NS_IMAGE_BASE		(ARM_DRAM1_BASE + UL(0x8000000))

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#if defined(IMAGE_BL31)
# if SPM_MM
#  define PLAT_ARM_MMAP_ENTRIES		10
#  define MAX_XLAT_TABLES		9
#  define PLAT_SP_IMAGE_MMAP_REGIONS	30
#  define PLAT_SP_IMAGE_MAX_XLAT_TABLES	10
# elif SPMC_AT_EL3
#  define PLAT_ARM_MMAP_ENTRIES		13
#  define MAX_XLAT_TABLES		11
# else
#  define PLAT_ARM_MMAP_ENTRIES		9
#  if USE_DEBUGFS
#   if ENABLE_RME
#    define MAX_XLAT_TABLES		9
#   else
#    define MAX_XLAT_TABLES		8
#   endif
#  else
#   if ENABLE_RME
#    define MAX_XLAT_TABLES		8
#   else
#    define MAX_XLAT_TABLES		7
#   endif
#  endif
# endif
#elif defined(IMAGE_BL32)
# if SPMC_AT_EL3
#  define PLAT_ARM_MMAP_ENTRIES		270
#  define MAX_XLAT_TABLES		10
# else
#  define PLAT_ARM_MMAP_ENTRIES		9
#  define MAX_XLAT_TABLES		6
# endif
#elif !USE_ROMLIB
# define PLAT_ARM_MMAP_ENTRIES		11
# define MAX_XLAT_TABLES		5
#else
# define PLAT_ARM_MMAP_ENTRIES		12
# define MAX_XLAT_TABLES		6
#endif

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#define PLAT_ARM_MAX_BL1_RW_SIZE	UL(0xB000)

/*
 * PLAT_ARM_MAX_ROMLIB_RW_SIZE is define to use a full page
 */

#if USE_ROMLIB
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0x1000)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0xe000)
#define FVP_BL2_ROMLIB_OPTIMIZATION	UL(0x5000)
#else
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0)
#define FVP_BL2_ROMLIB_OPTIMIZATION UL(0)
#endif

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth.
 */
#if TRUSTED_BOARD_BOOT && COT_DESC_IN_DTB
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x1E000) - FVP_BL2_ROMLIB_OPTIMIZATION)
#elif CRYPTO_SUPPORT
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x1D000) - FVP_BL2_ROMLIB_OPTIMIZATION)
#elif ARM_BL31_IN_DRAM
/* When ARM_BL31_IN_DRAM is set, BL2 can use almost all of Trusted SRAM. */
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x1F000) - FVP_BL2_ROMLIB_OPTIMIZATION)
#else
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x13000) - FVP_BL2_ROMLIB_OPTIMIZATION)
#endif

#if RESET_TO_BL31
/* Size of Trusted SRAM - the first 4KB of shared memory - GPT L0 Tables */
#define PLAT_ARM_MAX_BL31_SIZE		(PLAT_ARM_TRUSTED_SRAM_SIZE - \
					 ARM_SHARED_RAM_SIZE - \
					 ARM_L0_GPT_SIZE)
#else
/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE is
 * calculated using the current BL31 PROGBITS debug size plus the sizes of
 * BL2 and BL1-RW
 */
#define PLAT_ARM_MAX_BL31_SIZE		(UL(0x3D000) - ARM_L0_GPT_SIZE)
#endif /* RESET_TO_BL31 */

#ifndef __aarch64__
#if RESET_TO_SP_MIN
/* Size of Trusted SRAM - the first 4KB of shared memory */
#define PLAT_ARM_MAX_BL32_SIZE		(PLAT_ARM_TRUSTED_SRAM_SIZE - \
					 ARM_SHARED_RAM_SIZE)
#else
/*
 * Since BL32 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL32_SIZE is
 * calculated using the current SP_MIN PROGBITS debug size plus the sizes of
 * BL2 and BL1-RW
 */
# define PLAT_ARM_MAX_BL32_SIZE		UL(0x3B000)
#endif /* RESET_TO_SP_MIN */
#endif

/*
 * Size of cacheable stacks
 */
#if defined(IMAGE_BL1)
# if CRYPTO_SUPPORT
#  define PLATFORM_STACK_SIZE		UL(0x1000)
# else
#  define PLATFORM_STACK_SIZE		UL(0x500)
# endif /* CRYPTO_SUPPORT */
#elif defined(IMAGE_BL2)
# if CRYPTO_SUPPORT
#  define PLATFORM_STACK_SIZE		UL(0x1000)
# else
#  define PLATFORM_STACK_SIZE		UL(0x600)
# endif /* CRYPTO_SUPPORT */
#elif defined(IMAGE_BL2U)
# define PLATFORM_STACK_SIZE		UL(0x400)
#elif defined(IMAGE_BL31)
#  define PLATFORM_STACK_SIZE		UL(0x800)
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE		UL(0x440)
#elif defined(IMAGE_RMM)
# define PLATFORM_STACK_SIZE		UL(0x440)
#endif

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

/* Reserve the last block of flash for PSCI MEM PROTECT flag */
#define PLAT_ARM_FLASH_IMAGE_BASE	V2M_FLASH0_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#if ARM_GPT_SUPPORT
/*
 * Offset of the FIP in the GPT image. BL1 component uses this option
 * as it does not load the partition table to get the FIP base
 * address. At sector 34 by default (i.e. after reserved sectors 0-33)
 * Offset = 34 * 512(sector size) = 17408 i.e. 0x4400
 */
#define PLAT_ARM_FIP_OFFSET_IN_GPT	0x4400
#endif /* ARM_GPT_SUPPORT */

#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

/*
 * PL011 related constants
 */
#define PLAT_ARM_BOOT_UART_BASE		V2M_IOFPGA_UART0_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	V2M_IOFPGA_UART0_CLK_IN_HZ

#define PLAT_ARM_RUN_UART_BASE		V2M_IOFPGA_UART1_BASE
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	V2M_IOFPGA_UART1_CLK_IN_HZ

#define PLAT_ARM_CRASH_UART_BASE	PLAT_ARM_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	PLAT_ARM_RUN_UART_CLK_IN_HZ

#define PLAT_ARM_TSP_UART_BASE		V2M_IOFPGA_UART2_BASE
#define PLAT_ARM_TSP_UART_CLK_IN_HZ	V2M_IOFPGA_UART2_CLK_IN_HZ

#define PLAT_ARM_TRP_UART_BASE		V2M_IOFPGA_UART3_BASE
#define PLAT_ARM_TRP_UART_CLK_IN_HZ	V2M_IOFPGA_UART3_CLK_IN_HZ

#define PLAT_FVP_SMMUV3_BASE		UL(0x2b400000)
#define PLAT_ARM_SMMUV3_ROOT_REG_OFFSET UL(0x20000)

/* CCI related constants */
#define PLAT_FVP_CCI400_BASE		UL(0x2c090000)
#define PLAT_FVP_CCI400_CLUS0_SL_PORT	3
#define PLAT_FVP_CCI400_CLUS1_SL_PORT	4

/* CCI-500/CCI-550 on Base platform */
#define PLAT_FVP_CCI5XX_BASE		UL(0x2a000000)
#define PLAT_FVP_CCI5XX_CLUS0_SL_PORT	5
#define PLAT_FVP_CCI5XX_CLUS1_SL_PORT	6

/* CCN related constants. Only CCN 502 is currently supported */
#define PLAT_ARM_CCN_BASE		UL(0x2e000000)
#define PLAT_ARM_CLUSTER_TO_CCN_ID_MAP	1, 5, 7, 11

/* System timer related constants */
#define PLAT_ARM_NSTIMER_FRAME_ID		U(1)

/* Mailbox base address */
#define PLAT_ARM_TRUSTED_MAILBOX_BASE	ARM_TRUSTED_SRAM_BASE


/* TrustZone controller related constants
 *
 * Currently only filters 0 and 2 are connected on Base FVP.
 * Filter 0 : CPU clusters (no access to DRAM by default)
 * Filter 1 : not connected
 * Filter 2 : LCDs (access to VRAM allowed by default)
 * Filter 3 : not connected
 * Programming unconnected filters will have no effect at the
 * moment. These filter could, however, be connected in future.
 * So care should be taken not to configure the unused filters.
 *
 * Allow only non-secure access to all DRAM to supported devices.
 * Give access to the CPUs and Virtio. Some devices
 * would normally use the default ID so allow that too.
 */
#define PLAT_ARM_TZC_BASE		UL(0x2a4a0000)
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT(0)

#define PLAT_ARM_TZC_NS_DEV_ACCESS	(				\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_DEFAULT)	|	\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_PCI)		|	\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_AP)		|	\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_VIRTIO)	|	\
		TZC_REGION_ACCESS_RDWR(FVP_NSAID_VIRTIO_OLD))

/*
 * GIC related constants to cater for both GICv2 and GICv3 instances of an
 * FVP. They could be overridden at runtime in case the FVP implements the
 * legacy VE memory map.
 */
#define PLAT_ARM_GICD_BASE		BASE_GICD_BASE
#define PLAT_ARM_GICR_BASE		BASE_GICR_BASE
#define PLAT_ARM_GICC_BASE		BASE_GICC_BASE

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	ARM_G1S_IRQ_PROPS(grp), \
	INTR_PROP_DESC(FVP_IRQ_TZ_WDOG, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(FVP_IRQ_SEC_SYS_TIMER, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_LEVEL)

#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#if SDEI_IN_FCONF
#define PLAT_SDEI_DP_EVENT_MAX_CNT	ARM_SDEI_DP_EVENT_MAX_CNT
#define PLAT_SDEI_DS_EVENT_MAX_CNT	ARM_SDEI_DS_EVENT_MAX_CNT
#else
#define PLAT_ARM_PRIVATE_SDEI_EVENTS	ARM_SDEI_PRIVATE_EVENTS
#define PLAT_ARM_SHARED_SDEI_EVENTS	ARM_SDEI_SHARED_EVENTS
#endif

#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +	\
					 PLAT_SP_IMAGE_NS_BUF_SIZE)

#define PLAT_SP_PRI			PLAT_RAS_PRI

/*
 * Physical and virtual address space limits for MMU in AARCH64 & AARCH32 modes
 */
#ifdef __aarch64__
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 36)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#endif

/*
 * Maximum size of Event Log buffer used in Measured Boot Event Log driver
 */
#define	PLAT_ARM_EVENT_LOG_MAX_SIZE		UL(0x400)

#endif /* PLATFORM_DEF_H */
