/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM_BASE_PLATFORM_DEF_H
#define SGM_BASE_PLATFORM_DEF_H

#include <drivers/arm/tzc400.h>
#include <drivers/arm/tzc_common.h>
#include <plat/arm/board/common/board_css_def.h>
#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/arm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <plat/arm/soc/common/soc_css_def.h>
#include <plat/common/common_def.h>

/* CPU topology */
#define PLAT_ARM_CLUSTER_COUNT		1
#define PLAT_ARM_CLUSTER_CORE_COUNT	8
#define PLATFORM_CORE_COUNT		PLAT_ARM_CLUSTER_CORE_COUNT

#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL2
#define PLAT_NUM_PWR_DOMAINS		(ARM_SYSTEM_COUNT + \
					PLAT_ARM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
			CSS_G1S_IRQ_PROPS(grp), \
			ARM_G1S_IRQ_PROPS(grp)

#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE		0x30000000
#define PLAT_ARM_GICR_BASE		0x300C0000
#define PLAT_ARM_GICC_BASE		0x2c000000

#define CSS_GIC_SIZE			0x00200000

#define CSS_MAP_GIC_DEVICE		MAP_REGION_FLAT(	\
					PLAT_ARM_GICD_BASE,	\
					CSS_GIC_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

/* Platform ID address */
#define SSC_VERSION                     (SSC_REG_BASE + SSC_VERSION_OFFSET)
#ifndef __ASSEMBLY__
/* SSC_VERSION related accessors */
/* Returns the part number of the platform */
#define GET_PLAT_PART_NUM                                       \
		GET_SSC_VERSION_PART_NUM(mmio_read_32(SSC_VERSION))
/* Returns the configuration number of the platform */
#define GET_PLAT_CONFIG_NUM                                     \
		GET_SSC_VERSION_CONFIG(mmio_read_32(SSC_VERSION))
#endif /* __ASSEMBLY__ */


/*************************************************************************
 * Definitions common to all SGM CSS based platforms
 *************************************************************************/

/* TZC-400 related constants */
#define PLAT_ARM_TZC_BASE		0x2a500000
#define TZC_NSAID_ALL_AP		0  /* Note: Same as default NSAID!! */
#define TZC_NSAID_HDLCD0		2
#define TZC_NSAID_HDLCD1		3
#define TZC_NSAID_GPU			9
#define TZC_NSAID_VIDEO			10
#define TZC_NSAID_DISP0			11
#define TZC_NSAID_DISP1			12


/*************************************************************************
 * Required platform porting definitions common to all SGM CSS based
 * platforms
 *************************************************************************/

#define PLAT_ARM_TRUSTED_SRAM_SIZE	0x00040000	/* 256 KB */

/* MHU related constants */
#define PLAT_CSS_MHU_BASE		0x2b1f0000
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

#define PLAT_ARM_TRUSTED_ROM_BASE	0x00000000
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x00080000

#define PLAT_ARM_CCI_BASE		0x2a000000

/* Cluster to CCI slave mapping */
#define PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX	6
#define PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX	5

/* System timer related constants */
#define PLAT_ARM_NSTIMER_FRAME_ID	0

/* TZC related constants */
#define PLAT_ARM_TZC_NS_DEV_ACCESS	(				\
		TZC_REGION_ACCESS_RDWR(TZC_NSAID_ALL_AP)	|	\
		TZC_REGION_ACCESS_RDWR(TZC_NSAID_HDLCD0)	|	\
		TZC_REGION_ACCESS_RDWR(TZC_NSAID_HDLCD1)	|	\
		TZC_REGION_ACCESS_RDWR(TZC_NSAID_GPU)	|	\
		TZC_REGION_ACCESS_RDWR(TZC_NSAID_VIDEO)	|	\
		TZC_REGION_ACCESS_RDWR(TZC_NSAID_DISP0)	|	\
		TZC_REGION_ACCESS_RDWR(TZC_NSAID_DISP1))

/* Display Processor register definitions to setup the NSAIDs */
#define MALI_DP_BASE           0x2cc00000
#define DP_NPROT_NSAID_OFFSET     0x1000c
#define W_NPROT_NSAID_SHIFT            24
#define LS_NPORT_NSAID_SHIFT           12

/*
 * Base address of the first memory region used for communication between AP
 * and SCP. Used by the BootOverMHU and SCPI protocols.
 */
#if !CSS_USE_SCMI_SDS_DRIVER
/*
 * Note that this is located at the same address as SCP_BOOT_CFG_ADDR, which
 * means the SCP/AP configuration data gets overwritten when the AP initiates
 * communication with the SCP. The configuration data is expected to be a
 * 32-bit word on all CSS platforms. Part of this configuration is
 * which CPU is the primary, according to the shift and mask definitions below.
 */
#define PLAT_CSS_SCP_COM_SHARED_MEM_BASE	(ARM_TRUSTED_SRAM_BASE + 0x80)
#define PLAT_CSS_PRIMARY_CPU_SHIFT		8
#define PLAT_CSS_PRIMARY_CPU_BIT_WIDTH		4
#endif

/*
 * tspd support is conditional so enable this for CSS sgm platforms.
 */
#define SPD_tspd

/*
 * PLAT_CSS_MAX_SCP_BL2_SIZE is calculated using the current
 * SCP_BL2 size plus a little space for growth.
 */
#define PLAT_CSS_MAX_SCP_BL2_SIZE	0x15000

/*
 * PLAT_CSS_MAX_SCP_BL2U_SIZE is calculated using the current
 * SCP_BL2U size plus a little space for growth.
 */
#define PLAT_CSS_MAX_SCP_BL2U_SIZE	0x15000

/*
 * Most platform porting definitions provided by included headers
 */

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#if defined(IMAGE_BL31)
# define PLAT_ARM_MMAP_ENTRIES		8
# define MAX_XLAT_TABLES		5
#elif defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		8
# define MAX_XLAT_TABLES		5
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
#define PLAT_ARM_MAX_BL1_RW_SIZE	0xB000

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
# define PLAT_ARM_MAX_BL2_SIZE		0x1D000
#else
# define PLAT_ARM_MAX_BL2_SIZE		0x11000
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
#  define PLATFORM_STACK_SIZE 0x1000
# else
#  define PLATFORM_STACK_SIZE 0x440
# endif
#elif defined(IMAGE_BL2)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE 0x1000
# else
#  define PLATFORM_STACK_SIZE 0x400
# endif
#elif defined(IMAGE_BL2U)
# define PLATFORM_STACK_SIZE 0x400
#elif defined(IMAGE_BL31)
# define PLATFORM_STACK_SIZE 0x400
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE 0x440
#endif

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

/* System power domain level */
#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2

#endif /* SGM_BASE_PLATFORM_DEF_H */
