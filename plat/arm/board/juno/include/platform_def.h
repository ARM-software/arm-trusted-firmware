/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arm_def.h>
#include <board_arm_def.h>
#include <board_css_def.h>
#include <common_def.h>
#include <css_def.h>
#if TRUSTED_BOARD_BOOT
#include <mbedtls_config.h>
#endif
#include <soc_css_def.h>
#include <tzc400.h>
#include <v2m_def.h>
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
#define PLAT_CRYPTOCELL_BASE		0x60050000

/*
 * Other platform porting definitions are provided by included headers
 */

/*
 * Required ARM standard platform porting definitions
 */
#define PLAT_ARM_CLUSTER_COUNT		JUNO_CLUSTER_COUNT

/* Use the bypass address */
#define PLAT_ARM_TRUSTED_ROM_BASE	V2M_FLASH0_BASE + BL1_ROM_BYPASS_OFFSET

/*
 * Actual ROM size on Juno is 64 KB, but TBB currently requires at least 80 KB
 * in debug mode. We can test TBB on Juno bypassing the ROM and using 128 KB of
 * flash
 */
#if TRUSTED_BOARD_BOOT
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x00020000
#else
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x00010000
#endif /* TRUSTED_BOARD_BOOT */

/*
 * If ARM_BOARD_OPTIMISE_MEM=0 then Juno uses the default, unoptimised values
 * defined for ARM development platforms.
 */
#if ARM_BOARD_OPTIMISE_MEM
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
# define PLAT_ARM_MMAP_ENTRIES		4
# define MAX_XLAT_TABLES		3
#endif

#ifdef IMAGE_BL31
#  define PLAT_ARM_MMAP_ENTRIES		7
#  define MAX_XLAT_TABLES		3
#endif

#ifdef IMAGE_BL32
# define PLAT_ARM_MMAP_ENTRIES		5
# define MAX_XLAT_TABLES		4
#endif

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL1_RW_SIZE	0xA000
#else
# define PLAT_ARM_MAX_BL1_RW_SIZE	0x6000
#endif

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth.
 */
#if TRUSTED_BOARD_BOOT
#if TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA
# define PLAT_ARM_MAX_BL2_SIZE		0x1E000
#else
# define PLAT_ARM_MAX_BL2_SIZE		0x1A000
#endif
#else
# define PLAT_ARM_MAX_BL2_SIZE		0xC000
#endif

/*
 * PLAT_ARM_MAX_BL31_SIZE is calculated using the current BL31 debug size plus a
 * little space for growth.
 * SCP_BL2 image is loaded into the space BL31 -> BL1_RW_BASE.
 * For TBB use case, PLAT_ARM_MAX_BL1_RW_SIZE has been increased and therefore
 * PLAT_ARM_MAX_BL31_SIZE has been increased to ensure SCP_BL2 has the same
 * space available.
 */
#define PLAT_ARM_MAX_BL31_SIZE		0x1E000

#if JUNO_AARCH32_EL3_RUNTIME
/*
 * PLAT_ARM_MAX_BL32_SIZE is calculated for SP_MIN as the AArch32 Secure
 * Payload. We also need to take care of SCP_BL2 size as well, as the SCP_BL2
 * is loaded into the space BL32 -> BL1_RW_BASE
 */
# define PLAT_ARM_MAX_BL32_SIZE		0x1E000
#endif

/*
 * Since free SRAM space is scant, enable the ASSERTION message size
 * optimization by fixing the PLAT_LOG_LEVEL_ASSERT to LOG_LEVEL_INFO (40).
 */
#define PLAT_LOG_LEVEL_ASSERT		40

#endif /* ARM_BOARD_OPTIMISE_MEM */

/* CCI related constants */
#define PLAT_ARM_CCI_BASE		0x2c090000
#define PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX	4
#define PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX	3

/* System timer related constants */
#define PLAT_ARM_NSTIMER_FRAME_ID		1

/* TZC related constants */
#define PLAT_ARM_TZC_BASE		0x2a4a0000
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

/*
 * Required ARM CSS based platform porting definitions
 */

/* GIC related constants (no GICR in GIC-400) */
#define PLAT_ARM_GICD_BASE		0x2c010000
#define PLAT_ARM_GICC_BASE		0x2c02f000
#define PLAT_ARM_GICH_BASE		0x2c04f000
#define PLAT_ARM_GICV_BASE		0x2c06f000

/* MHU related constants */
#define PLAT_CSS_MHU_BASE		0x2b1f0000

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
#define PLAT_CSS_SCP_COM_SHARED_MEM_BASE	(ARM_TRUSTED_SRAM_BASE + 0x80)
#define PLAT_CSS_PRIMARY_CPU_SHIFT		8
#define PLAT_CSS_PRIMARY_CPU_BIT_WIDTH		4
#endif

/*
 * PLAT_CSS_MAX_SCP_BL2_SIZE is calculated using the current
 * SCP_BL2 size plus a little space for growth.
 */
#define PLAT_CSS_MAX_SCP_BL2_SIZE	0x14000

/*
 * PLAT_CSS_MAX_SCP_BL2U_SIZE is calculated using the current
 * SCP_BL2U size plus a little space for growth.
 */
#define PLAT_CSS_MAX_SCP_BL2U_SIZE	0x14000

#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	CSS_G1S_IRQ_PROPS(grp), \
	ARM_G1S_IRQ_PROPS(grp), \
	INTR_PROP_DESC(JUNO_IRQ_DMA_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_HDLCD0_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_HDLCD1_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_USB_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_THIN_LINKS_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_SEC_I2C, GIC_HIGHEST_SEC_PRIORITY, \
		grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_GPU_SMMU_1, GIC_HIGHEST_SEC_PRIORITY, \
		grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(JUNO_IRQ_ETR_SMMU, GIC_HIGHEST_SEC_PRIORITY, \
		grp, GIC_INTR_CFG_LEVEL)

#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

/*
 * Required ARM CSS SoC based platform porting definitions
 */

/* CSS SoC NIC-400 Global Programmers View (GPV) */
#define PLAT_SOC_CSS_NIC400_BASE	0x2a000000

#define PLAT_ARM_PRIVATE_SDEI_EVENTS	ARM_SDEI_PRIVATE_EVENTS
#define PLAT_ARM_SHARED_SDEI_EVENTS	ARM_SDEI_SHARED_EVENTS

#endif /* __PLATFORM_DEF_H__ */
