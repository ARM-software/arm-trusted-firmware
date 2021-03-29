/*
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI_BASE_PLATFORM_DEF_H
#define SGI_BASE_PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/arm/common/arm_def.h>
#include <plat/arm/common/arm_spm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <plat/common/common_def.h>

#define PLATFORM_CORE_COUNT		(CSS_SGI_CHIP_COUNT *		\
					PLAT_ARM_CLUSTER_COUNT *	\
					CSS_SGI_MAX_CPUS_PER_CLUSTER *	\
					CSS_SGI_MAX_PE_PER_CPU)

#define PLAT_ARM_TRUSTED_SRAM_SIZE	0x00040000	/* 256 KB */

/* Remote chip address offset (4TB per chip) */
#define CSS_SGI_REMOTE_CHIP_MEM_OFFSET(n)	((ULL(1) << 42) * (n))

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage. In addition to that, on
 * multi-chip platforms, address regions on each of the remote chips are
 * also mapped. In BL31, for instance, three address regions on the remote
 * chips are accessed - secure ram, css device and soc device regions.
 */
#if defined(IMAGE_BL31)
# if SPM_MM
#  define PLAT_ARM_MMAP_ENTRIES		(9  + ((CSS_SGI_CHIP_COUNT - 1) * 3))
#  define MAX_XLAT_TABLES		(7  + ((CSS_SGI_CHIP_COUNT - 1) * 3))
#  define PLAT_SP_IMAGE_MMAP_REGIONS	9
#  define PLAT_SP_IMAGE_MAX_XLAT_TABLES	11
# else
#  define PLAT_ARM_MMAP_ENTRIES		(5 + ((CSS_SGI_CHIP_COUNT - 1) * 3))
#  define MAX_XLAT_TABLES		(6 + ((CSS_SGI_CHIP_COUNT - 1) * 3))
# endif
#elif defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		8
# define MAX_XLAT_TABLES		5
#elif defined(IMAGE_BL2)
# define PLAT_ARM_MMAP_ENTRIES		(11 + (CSS_SGI_CHIP_COUNT - 1))

/*
 * MAX_XLAT_TABLES entries need to be doubled because when the address width
 * exceeds 40 bits an additional level of translation is required. In case of
 * multichip platforms peripherals also fall into address space with width
 * > 40 bits
 *
 */
# define MAX_XLAT_TABLES		(7  + ((CSS_SGI_CHIP_COUNT - 1) * 2))
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
 * little space for growth. Additional 8KiB space is added per chip in
 * order to accommodate the additional level of translation required for "TZC"
 * peripheral access which lies in >4TB address space.
 *
 */
#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL2_SIZE		(0x1D000 + ((CSS_SGI_CHIP_COUNT - 1) * \
							0x2000))
#else
# define PLAT_ARM_MAX_BL2_SIZE		(0x14000 + ((CSS_SGI_CHIP_COUNT - 1) * \
							0x2000))
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
# if SPM_MM
#  define PLATFORM_STACK_SIZE 0x500
# else
#  define PLATFORM_STACK_SIZE 0x400
# endif
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE 0x440
#endif


#define PLAT_ARM_NSTIMER_FRAME_ID	0

#define PLAT_ARM_TRUSTED_ROM_BASE	0x0
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x00080000	/* 512KB */

#define PLAT_ARM_NSRAM_BASE		0x06000000
#define PLAT_ARM_NSRAM_SIZE		0x00080000	/* 512KB */

#define PLAT_ARM_DRAM2_BASE		ULL(0x8080000000)
#define PLAT_ARM_DRAM2_SIZE		ULL(0x180000000)

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#define CSS_SGI_DEVICE_BASE	(0x20000000)
#define CSS_SGI_DEVICE_SIZE	(0x20000000)
#define CSS_SGI_MAP_DEVICE	MAP_REGION_FLAT(		\
					CSS_SGI_DEVICE_BASE,	\
					CSS_SGI_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_MAP_SHARED_RAM_REMOTE_CHIP(n)					\
			MAP_REGION_FLAT(					\
				CSS_SGI_REMOTE_CHIP_MEM_OFFSET(n) +		\
				ARM_SHARED_RAM_BASE,				\
				ARM_SHARED_RAM_SIZE,				\
				MT_NON_CACHEABLE | MT_RW | MT_SECURE		\
			)

#define CSS_SGI_MAP_DEVICE_REMOTE_CHIP(n)					\
			MAP_REGION_FLAT(					\
				CSS_SGI_REMOTE_CHIP_MEM_OFFSET(n) +		\
				CSS_SGI_DEVICE_BASE,				\
				CSS_SGI_DEVICE_SIZE,				\
				MT_DEVICE | MT_RW | MT_SECURE			\
			)

#define SOC_CSS_MAP_DEVICE_REMOTE_CHIP(n)					\
			MAP_REGION_FLAT(					\
				CSS_SGI_REMOTE_CHIP_MEM_OFFSET(n) +		\
				SOC_CSS_DEVICE_BASE,				\
				SOC_CSS_DEVICE_SIZE,				\
				MT_DEVICE | MT_RW | MT_SECURE			\
			)

/* Map the secure region for access from S-EL0 */
#define PLAT_ARM_SECURE_MAP_DEVICE	MAP_REGION_FLAT(	\
					SOC_CSS_DEVICE_BASE,	\
					SOC_CSS_DEVICE_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE | MT_USER)

#define PLAT_SP_PRI				PLAT_RAS_PRI

#if SPM_MM && RAS_EXTENSION
/*
 * CPER buffer memory of 128KB is reserved and it is placed adjacent to the
 * memory shared between EL3 and S-EL0.
 */
#define CSS_SGI_SP_CPER_BUF_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE + \
					 PLAT_SP_IMAGE_NS_BUF_SIZE)
#define CSS_SGI_SP_CPER_BUF_SIZE	ULL(0x20000)
#define CSS_SGI_SP_CPER_BUF_MMAP	MAP_REGION2(			       \
						CSS_SGI_SP_CPER_BUF_BASE,      \
						CSS_SGI_SP_CPER_BUF_BASE,      \
						CSS_SGI_SP_CPER_BUF_SIZE,      \
						MT_RW_DATA | MT_NS | MT_USER,  \
						PAGE_SIZE)

/*
 * Secure partition stack follows right after the memory space reserved for
 * CPER buffer memory.
 */
#define PLAT_ARM_SP_IMAGE_STACK_BASE		(PLAT_SP_IMAGE_NS_BUF_BASE +   \
						 PLAT_SP_IMAGE_NS_BUF_SIZE +   \
						 CSS_SGI_SP_CPER_BUF_SIZE)
#elif SPM_MM
/*
 * Secure partition stack follows right after the memory region that is shared
 * between EL3 and S-EL0.
 */
#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +	\
					 PLAT_SP_IMAGE_NS_BUF_SIZE)
#endif /* SPM_MM && RAS_EXTENSION */

/* Platform ID address */
#define SSC_VERSION                     (SSC_REG_BASE + SSC_VERSION_OFFSET)
#ifndef __ASSEMBLER__
/* SSC_VERSION related accessors */
/* Returns the part number of the platform */
#define GET_SGI_PART_NUM                                       \
		GET_SSC_VERSION_PART_NUM(mmio_read_32(SSC_VERSION))
/* Returns the configuration number of the platform */
#define GET_SGI_CONFIG_NUM                                     \
		GET_SSC_VERSION_CONFIG(mmio_read_32(SSC_VERSION))
#endif /* __ASSEMBLER__ */

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

/* Number of SCMI channels on the platform */
#define PLAT_ARM_SCMI_CHANNEL_COUNT	CSS_SGI_CHIP_COUNT

/*
 * Mapping definition of the TrustZone Controller for ARM SGI/RD platforms
 * where both the DRAM regions are marked for non-secure access. This applies
 * to multi-chip platforms.
 */
#define SGI_PLAT_TZC_NS_REMOTE_REGIONS_DEF(n)				\
	{CSS_SGI_REMOTE_CHIP_MEM_OFFSET(n) + ARM_DRAM1_BASE,		\
		CSS_SGI_REMOTE_CHIP_MEM_OFFSET(n) + ARM_DRAM1_END,	\
		ARM_TZC_NS_DRAM_S_ACCESS, PLAT_ARM_TZC_NS_DEV_ACCESS},	\
	{CSS_SGI_REMOTE_CHIP_MEM_OFFSET(n) + ARM_DRAM2_BASE,		\
		CSS_SGI_REMOTE_CHIP_MEM_OFFSET(n) + ARM_DRAM2_END,	\
		ARM_TZC_NS_DRAM_S_ACCESS, PLAT_ARM_TZC_NS_DEV_ACCESS}

#endif /* SGI_BASE_PLATFORM_DEF_H */
