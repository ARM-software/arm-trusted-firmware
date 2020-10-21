/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
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

#define PLATFORM_CORE_COUNT		4

#define PLAT_ARM_TRUSTED_SRAM_SIZE	0x00080000	/* 512 KB */

/*
 * The top 16MB of ARM_DRAM1 is configured as secure access only using the TZC,
 * its base is ARM_AP_TZC_DRAM1_BASE.
 *
 * Reserve 32MB below ARM_AP_TZC_DRAM1_BASE for:
 *   - BL32_BASE when SPD_spmd is enabled
 *   - Region to load Trusted OS
 */
#define TC0_TZC_DRAM1_BASE		(ARM_AP_TZC_DRAM1_BASE -	\
					 TC0_TZC_DRAM1_SIZE)
#define TC0_TZC_DRAM1_SIZE		UL(0x02000000)	/* 32 MB */
#define TC0_TZC_DRAM1_END		(TC0_TZC_DRAM1_BASE +		\
					 TC0_TZC_DRAM1_SIZE - 1)

#define TC0_NS_DRAM1_BASE		ARM_DRAM1_BASE
#define TC0_NS_DRAM1_SIZE		(ARM_DRAM1_SIZE -		\
					 ARM_TZC_DRAM1_SIZE -		\
					 TC0_TZC_DRAM1_SIZE)
#define TC0_NS_DRAM1_END		(TC0_NS_DRAM1_BASE +		\
					 TC0_NS_DRAM1_SIZE - 1)

/*
 * Mappings for TC0 DRAM1 (non-secure) and TC0 TZC DRAM1 (secure)
 */
#define TC0_MAP_NS_DRAM1		MAP_REGION_FLAT(		\
						TC0_NS_DRAM1_BASE,	\
						TC0_NS_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_NS)


#define TC0_MAP_TZC_DRAM1		MAP_REGION_FLAT(		\
						TC0_TZC_DRAM1_BASE,	\
						TC0_TZC_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)
/*
 * Max size of SPMC is 2MB for tc0. With SPMD enabled this value corresponds to
 * max size of BL32 image.
 */
#if defined(SPD_spmd)
#define PLAT_ARM_SPMC_BASE		TC0_TZC_DRAM1_BASE
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
#define PLAT_ARM_MAX_BL1_RW_SIZE	0xC000

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
 * little space for growth.
 */
#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL2_SIZE		0x1E000
#else
# define PLAT_ARM_MAX_BL2_SIZE		0x14000
#endif

/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE is
 * calculated using the current BL31 PROGBITS debug size plus the sizes of
 * BL2 and BL1-RW
 */
#define PLAT_ARM_MAX_BL31_SIZE		0x3B000

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
#  define PLATFORM_STACK_SIZE		0x400
# endif
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE		0x440
#endif


#define TC0_DEVICE_BASE			0x21000000
#define TC0_DEVICE_SIZE			0x5f000000

// TC0_MAP_DEVICE covers different peripherals
// available to the platform
#define TC0_MAP_DEVICE	MAP_REGION_FLAT(		\
					TC0_DEVICE_BASE,	\
					TC0_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)


#define TC0_FLASH0_RO	MAP_REGION_FLAT(V2M_FLASH0_BASE,\
						V2M_FLASH0_SIZE,	\
						MT_DEVICE | MT_RO | MT_SECURE)

#define PLAT_ARM_NSTIMER_FRAME_ID	0

#define PLAT_ARM_TRUSTED_ROM_BASE	0x0
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x00080000	/* 512KB */

#define PLAT_ARM_NSRAM_BASE		0x06000000
#define PLAT_ARM_NSRAM_SIZE		0x00080000	/* 512KB */

#define PLAT_ARM_DRAM2_BASE		ULL(0x8080000000)
#define PLAT_ARM_DRAM2_SIZE		ULL(0x180000000)

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +	\
					 PLAT_SP_IMAGE_NS_BUF_SIZE)

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

/*Secure Watchdog Constants */
#define SBSA_SECURE_WDOG_BASE		UL(0x2A480000)
#define SBSA_SECURE_WDOG_TIMEOUT	UL(100)

#define PLAT_ARM_SCMI_CHANNEL_COUNT	1

#define PLAT_ARM_CLUSTER_COUNT		U(1)
#define PLAT_MAX_CPUS_PER_CLUSTER	U(4)
#define PLAT_MAX_PE_PER_CPU		U(1)

#define PLAT_CSS_MHU_BASE		UL(0x45400000)
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

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
#define PLAT_ARM_GICR_BASE		UL(0x30140000)

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
 * The first region below, TC0_TZC_DRAM1_BASE (0xfd000000) to
 * ARM_SCP_TZC_DRAM1_END (0xffffffff) will mark the last 48 MB of DRAM as
 * secure. The second region gives non secure access to rest of DRAM.
 */
#define TC0_TZC_REGIONS_DEF						\
	{TC0_TZC_DRAM1_BASE, ARM_SCP_TZC_DRAM1_END,			\
		TZC_REGION_S_RDWR, PLAT_ARM_TZC_NS_DEV_ACCESS},		\
	{TC0_NS_DRAM1_BASE, TC0_NS_DRAM1_END, ARM_TZC_NS_DRAM_S_ACCESS, \
		PLAT_ARM_TZC_NS_DEV_ACCESS}

/* virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xc0000000)

#endif /* PLATFORM_DEF_H */
