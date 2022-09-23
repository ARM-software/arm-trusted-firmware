/*
 * Copyright (c) 2014-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <drivers/arm/tzc400.h>
#if TRUSTED_BOARD_BOOT
#include MBEDTLS_CONFIG_FILE
#endif
#include <plat/arm/board/common/board_css_def.h>
#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/arm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <plat/arm/soc/common/soc_css_def.h>
#include <plat/common/common_def.h>

#include "../juno_def.h"

/* Required platform porting definitions */
/* Juno supports system power domain */
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL2
#define PLAT_NUM_PWR_DOMAINS		(ARM_SYSTEM_COUNT + \
					JUNO_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)
#define PLATFORM_CORE_COUNT		(JUNO_CLUSTER0_CORE_COUNT + \
					JUNO_CLUSTER1_CORE_COUNT)

/* Cryptocell HW Base address */
#define PLAT_CRYPTOCELL_BASE		UL(0x60050000)

/*
 * Other platform porting definitions are provided by included headers
 */

/*
 * Required ARM standard platform porting definitions
 */
#define PLAT_ARM_CLUSTER_COUNT		JUNO_CLUSTER_COUNT

#define PLAT_ARM_TRUSTED_SRAM_SIZE	UL(0x00040000)	/* 256 KB */

/* Use the bypass address */
#define PLAT_ARM_TRUSTED_ROM_BASE	(V2M_FLASH0_BASE + \
					BL1_ROM_BYPASS_OFFSET)

#define NSRAM_BASE			UL(0x2e000000)
#define NSRAM_SIZE			UL(0x00008000)	/* 32KB */

#define PLAT_ARM_DRAM2_BASE		ULL(0x880000000)
#define PLAT_ARM_DRAM2_SIZE		ULL(0x180000000)

/* Range of kernel DTB load address */
#define JUNO_DTB_DRAM_MAP_START		ULL(0x82000000)
#define JUNO_DTB_DRAM_MAP_SIZE		ULL(0x00008000) /* 32KB */

#define ARM_DTB_DRAM_NS			MAP_REGION_FLAT(		\
					JUNO_DTB_DRAM_MAP_START,	\
					JUNO_DTB_DRAM_MAP_SIZE,		\
					MT_MEMORY | MT_RO | MT_NS)

/* virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xc0000000)

/*
 * PLAT_ARM_MAX_ROMLIB_RW_SIZE is define to use a full page
 */

#if USE_ROMLIB
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0x1000)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0xe000)
#define JUNO_BL2_ROMLIB_OPTIMIZATION UL(0x8000)
#else
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0)
#define JUNO_BL2_ROMLIB_OPTIMIZATION UL(0)
#endif

/*
 * Actual ROM size on Juno is 64 KB, but TBB currently requires at least 80 KB
 * in debug mode. We can test TBB on Juno bypassing the ROM and using 128 KB of
 * flash
 */

#if TRUSTED_BOARD_BOOT
#define PLAT_ARM_TRUSTED_ROM_SIZE	UL(0x00020000)
#else
#define PLAT_ARM_TRUSTED_ROM_SIZE	UL(0x00010000)
#endif /* TRUSTED_BOARD_BOOT */

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#ifdef IMAGE_BL1
# define PLAT_ARM_MMAP_ENTRIES		7
# define MAX_XLAT_TABLES		4
#endif

#ifdef IMAGE_BL2
#ifdef SPD_opteed
# define PLAT_ARM_MMAP_ENTRIES		11
# define MAX_XLAT_TABLES		5
#else
# define PLAT_ARM_MMAP_ENTRIES		10
# define MAX_XLAT_TABLES		4
#endif
#endif

#ifdef IMAGE_BL2U
# define PLAT_ARM_MMAP_ENTRIES		5
# define MAX_XLAT_TABLES		3
#endif

#ifdef IMAGE_BL31
#  define PLAT_ARM_MMAP_ENTRIES		7
#  define MAX_XLAT_TABLES		5
#endif

#ifdef IMAGE_BL32
# define PLAT_ARM_MMAP_ENTRIES		6
# define MAX_XLAT_TABLES		4
#endif

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL1_RW_SIZE	UL(0xB000)
#else
# define PLAT_ARM_MAX_BL1_RW_SIZE	UL(0x6000)
#endif

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth.
 */
#if TRUSTED_BOARD_BOOT
#if TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x1F000) - JUNO_BL2_ROMLIB_OPTIMIZATION)
#elif TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x1D000) - JUNO_BL2_ROMLIB_OPTIMIZATION)
#else
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x1D000) - JUNO_BL2_ROMLIB_OPTIMIZATION)
#endif
#else
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x13000) - JUNO_BL2_ROMLIB_OPTIMIZATION)
#endif

/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE is
 * calculated using the current BL31 PROGBITS debug size plus the sizes of
 * BL2 and BL1-RW.  SCP_BL2 image is loaded into the space BL31 -> BL2_BASE.
 * Hence the BL31 PROGBITS size should be >= PLAT_CSS_MAX_SCP_BL2_SIZE.
 */
#define PLAT_ARM_MAX_BL31_SIZE		UL(0x3D000)

#if JUNO_AARCH32_EL3_RUNTIME
/*
 * Since BL32 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL32_SIZE is
 * calculated using the current BL32 PROGBITS debug size plus the sizes of
 * BL2 and BL1-RW.  SCP_BL2 image is loaded into the space BL32 -> BL2_BASE.
 * Hence the BL32 PROGBITS size should be >= PLAT_CSS_MAX_SCP_BL2_SIZE.
 */
#define PLAT_ARM_MAX_BL32_SIZE		UL(0x3D000)
#endif

/*
 * Size of cacheable stacks
 */
#if defined(IMAGE_BL1)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		UL(0x1000)
# else
#  define PLATFORM_STACK_SIZE		UL(0x440)
# endif
#elif defined(IMAGE_BL2)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		UL(0x1000)
# else
#  define PLATFORM_STACK_SIZE		UL(0x400)
# endif
#elif defined(IMAGE_BL2U)
# define PLATFORM_STACK_SIZE		UL(0x400)
#elif defined(IMAGE_BL31)
# if PLAT_XLAT_TABLES_DYNAMIC
#  define PLATFORM_STACK_SIZE		UL(0x800)
# else
#  define PLATFORM_STACK_SIZE		UL(0x400)
# endif
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE		UL(0x440)
#endif

/* CCI related constants */
#define PLAT_ARM_CCI_BASE		UL(0x2c090000)
#define PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX	4
#define PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX	3

/* System timer related constants */
#define PLAT_ARM_NSTIMER_FRAME_ID		U(1)

/* TZC related constants */
#define PLAT_ARM_TZC_BASE		UL(0x2a4a0000)
#define PLAT_ARM_TZC_NS_DEV_ACCESS	(				\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CCI400)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_PCIE)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD0)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD1)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_USB)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_DMA330)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_THINLINKS)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_AP)		|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_GPU)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CORESIGHT))

/* TZC related constants */
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT_ALL

/*
 * Required ARM CSS based platform porting definitions
 */

/* GIC related constants (no GICR in GIC-400) */
#define PLAT_ARM_GICD_BASE		UL(0x2c010000)
#define PLAT_ARM_GICC_BASE		UL(0x2c02f000)
#define PLAT_ARM_GICH_BASE		UL(0x2c04f000)
#define PLAT_ARM_GICV_BASE		UL(0x2c06f000)

/* MHU related constants */
#define PLAT_CSS_MHU_BASE		UL(0x2b1f0000)

/*
 * Base address of the first memory region used for communication between AP
 * and SCP. Used by the BOM and SCPI protocols.
 */
#if !CSS_USE_SCMI_SDS_DRIVER
/*
 * Note that this is located at the same address as SCP_BOOT_CFG_ADDR, which
 * means the SCP/AP configuration data gets overwritten when the AP initiates
 * communication with the SCP. The configuration data is expected to be a
 * 32-bit word on all CSS platforms. On Juno, part of this configuration is
 * which CPU is the primary, according to the shift and mask definitions below.
 */
#define PLAT_CSS_SCP_COM_SHARED_MEM_BASE	(ARM_TRUSTED_SRAM_BASE + UL(0x80))
#define PLAT_CSS_PRIMARY_CPU_SHIFT		8
#define PLAT_CSS_PRIMARY_CPU_BIT_WIDTH		4
#endif

/*
 * SCP_BL2 uses up whatever remaining space is available as it is loaded before
 * anything else in this memory region and is handed over to the SCP before
 * BL31 is loaded over the top.
 */
#define PLAT_CSS_MAX_SCP_BL2_SIZE \
	((SCP_BL2_LIMIT - ARM_FW_CONFIG_LIMIT) & ~PAGE_SIZE_MASK)

#define PLAT_CSS_MAX_SCP_BL2U_SIZE	PLAT_CSS_MAX_SCP_BL2_SIZE

#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	CSS_G1S_IRQ_PROPS(grp), \
	ARM_G1S_IRQ_PROPS(grp), \
	INTR_PROP_DESC(JUNO_IRQ_DMA_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		(grp), GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_HDLCD0_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		(grp), GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_HDLCD1_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		(grp), GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_USB_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		(grp), GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_THIN_LINKS_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		(grp), GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_SEC_I2C, GIC_HIGHEST_SEC_PRIORITY, \
		(grp), GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_GPU_SMMU_1, GIC_HIGHEST_SEC_PRIORITY, \
		(grp), GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_ETR_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		(grp), GIC_INTR_CFG_LEVEL)

#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

/*
 * Required ARM CSS SoC based platform porting definitions
 */

/* CSS SoC NIC-400 Global Programmers View (GPV) */
#define PLAT_SOC_CSS_NIC400_BASE	UL(0x2a000000)

#define PLAT_ARM_PRIVATE_SDEI_EVENTS	ARM_SDEI_PRIVATE_EVENTS
#define PLAT_ARM_SHARED_SDEI_EVENTS	ARM_SDEI_SHARED_EVENTS

/* System power domain level */
#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2

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

/* Number of SCMI channels on the platform */
#define PLAT_ARM_SCMI_CHANNEL_COUNT	U(1)

#endif /* PLATFORM_DEF_H */
