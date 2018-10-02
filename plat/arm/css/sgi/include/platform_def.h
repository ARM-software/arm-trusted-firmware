/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arm_def.h>
#include <arm_spm_def.h>
#include <board_arm_def.h>
#include <board_css_def.h>
#include <common_def.h>
#include <css_def.h>
#include <soc_css_def.h>
#include <utils_def.h>
#include <xlat_tables_defs.h>

#define CSS_SGI_MAX_CPUS_PER_CLUSTER	4

/* CPU topology */
#define PLAT_ARM_CLUSTER_COUNT		2
#define CSS_SGI_MAX_PE_PER_CPU		1
#define PLATFORM_CORE_COUNT		(PLAT_ARM_CLUSTER_COUNT *	\
					CSS_SGI_MAX_CPUS_PER_CLUSTER * \
					CSS_SGI_MAX_PE_PER_CPU)

#if ARM_BOARD_OPTIMISE_MEM

#if defined(IMAGE_BL31) || defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		6
# define MAX_XLAT_TABLES		4
#else
# define PLAT_ARM_MMAP_ENTRIES		10
# define MAX_XLAT_TABLES		5
#endif

#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL1_RW_SIZE	0xA000
#else
# define PLAT_ARM_MAX_BL1_RW_SIZE	0x6000
#endif

#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL2_SIZE		0x1D000
#else
# define PLAT_ARM_MAX_BL2_SIZE		0xC000
#endif

#endif /* ARM_BOARD_OPTIMISE_MEM */

#define PLAT_ARM_NSTIMER_FRAME_ID	0

#define PLAT_CSS_MHU_BASE		0x45000000

#define PLAT_ARM_TRUSTED_ROM_BASE	0x0
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x00080000	/* 512KB */

#define PLAT_ARM_NSRAM_BASE		0x06000000
#define PLAT_ARM_NSRAM_SIZE		0x00080000	/* 512KB */

#define PLAT_MAX_PWR_LVL		U(1)

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#define CSS_SGI_DEVICE_BASE	(0x20000000)
#define CSS_SGI_DEVICE_SIZE	(0x20000000)
#define CSS_SGI_MAP_DEVICE	MAP_REGION_FLAT(		\
					CSS_SGI_DEVICE_BASE,	\
					CSS_SGI_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE		0x30000000
#define PLAT_ARM_GICC_BASE		0x2C000000
#define PLAT_ARM_GICR_BASE		0x300C0000

/* Map the secure region for access from S-EL0 */
#define PLAT_ARM_SECURE_MAP_DEVICE	MAP_REGION_FLAT(	\
					SOC_CSS_DEVICE_BASE,	\
					SOC_CSS_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE | MT_USER)

#if RAS_EXTENSION
/* Allocate 128KB for CPER buffers */
#define PLAT_SP_BUF_BASE			ULL(0x20000)

#define PLAT_ARM_SP_IMAGE_STACK_BASE		(ARM_SP_IMAGE_NS_BUF_BASE + \
						ARM_SP_IMAGE_NS_BUF_SIZE + \
						PLAT_SP_BUF_BASE)

/* Platform specific SMC FID's used for RAS */
#define SP_DMC_ERROR_INJECT_EVENT_AARCH64	0xC4000042
#define SP_DMC_ERROR_INJECT_EVENT_AARCH32	0x84000042

#define SP_DMC_ERROR_OVERFLOW_EVENT_AARCH64	0xC4000043
#define SP_DMC_ERROR_OVERFLOW_EVENT_AARCH32	0x84000043

#define SP_DMC_ERROR_ECC_EVENT_AARCH64		0xC4000044
#define SP_DMC_ERROR_ECC_EVENT_AARCH32		0x84000044

/* ARM SDEI dynamic shared event numbers */
#define SGI_SDEI_DS_EVENT_0			804
#define SGI_SDEI_DS_EVENT_1			805

#define PLAT_ARM_PRIVATE_SDEI_EVENTS	\
	SDEI_DEFINE_EVENT_0(ARM_SDEI_SGI), \
	SDEI_EXPLICIT_EVENT(SGI_SDEI_DS_EVENT_0, SDEI_MAPF_CRITICAL), \
	SDEI_EXPLICIT_EVENT(SGI_SDEI_DS_EVENT_1, SDEI_MAPF_CRITICAL),
#define PLAT_ARM_SHARED_SDEI_EVENTS

#define ARM_SP_CPER_BUF_BASE			(ARM_SP_IMAGE_NS_BUF_BASE + \
						ARM_SP_IMAGE_NS_BUF_SIZE)
#define ARM_SP_CPER_BUF_SIZE			ULL(0x20000)
#define ARM_SP_CPER_BUF_MMAP			MAP_REGION2(		\
						ARM_SP_CPER_BUF_BASE,	\
						ARM_SP_CPER_BUF_BASE,	\
						ARM_SP_CPER_BUF_SIZE,	\
						MT_RW_DATA | MT_NS | MT_USER, \
						PAGE_SIZE)

#else
#define PLAT_ARM_SP_IMAGE_STACK_BASE	(ARM_SP_IMAGE_NS_BUF_BASE +	\
					 ARM_SP_IMAGE_NS_BUF_SIZE)
#endif /* RAS_EXTENSION */

/* Platform ID address */
#define SSC_VERSION                     (SSC_REG_BASE + SSC_VERSION_OFFSET)
#ifndef __ASSEMBLY__
/* SSC_VERSION related accessors */
/* Returns the part number of the platform */
#define GET_SGI_PART_NUM                                       \
		GET_SSC_VERSION_PART_NUM(mmio_read_32(SSC_VERSION))
/* Returns the configuration number of the platform */
#define GET_SGI_CONFIG_NUM                                     \
		GET_SSC_VERSION_CONFIG(mmio_read_32(SSC_VERSION))
#endif /* __ASSEMBLY__ */

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


#endif /* PLATFORM_DEF_H */
