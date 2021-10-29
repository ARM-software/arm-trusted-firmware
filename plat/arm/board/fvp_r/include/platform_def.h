/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_R_PLATFORM_DEF_H
#define FVP_R_PLATFORM_DEF_H

#define PLAT_V2M_OFFSET			0x80000000

#define BL33_IMAGE_DESC {				\
	.image_id = BL33_IMAGE_ID,			\
	SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,	\
		VERSION_2, image_info_t, 0),		\
	.image_info.image_base = PLAT_ARM_DRAM1_BASE + 0x1000,		\
	.image_info.image_max_size = UL(0x3ffff000), \
	SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,	\
		VERSION_2, entry_point_info_t, SECURE | EXECUTABLE),\
	.ep_info.pc = PLAT_ARM_DRAM1_BASE + 0x1000,				\
	.ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS),	\
}

#include "../fvp_r_def.h"
#include <drivers/arm/tzc400.h>
#include <lib/utils_def.h>
#include <plat/arm/board/common/v2m_def.h>

/* These are referenced by arm_def.h #included next, so #define first. */
#define PLAT_ARM_TRUSTED_ROM_BASE	UL(0x80000000)
#define PLAT_ARM_TRUSTED_SRAM_BASE	UL(0x84000000)
#define PLAT_ARM_TRUSTED_DRAM_BASE	UL(0x86000000)
#define PLAT_ARM_DRAM1_BASE		ULL(0x0)
#define PLAT_ARM_DRAM2_BASE		ULL(0x080000000)

#define PLAT_HW_CONFIG_DTB_BASE		ULL(0x12000000)
#define PLAT_ARM_SYS_CNTCTL_BASE	UL(0xaa430000)
#define PLAT_ARM_SYS_CNTREAD_BASE	UL(0xaa800000)
#define PLAT_ARM_SYS_TIMCTL_BASE	UL(0xaa810000)
#define PLAT_ARM_SYS_CNT_BASE_S		UL(0xaa820000)
#define PLAT_ARM_SYS_CNT_BASE_NS	UL(0xaa830000)
#define PLAT_ARM_SP805_TWDG_BASE	UL(0xaa490000)

#include <plat/arm/common/arm_def.h>
#include <plat/common/common_def.h>


/* Required to create plat_regions: */
#define MIN_LVL_BLOCK_DESC	U(1)

/* Required platform porting definitions */
#define PLATFORM_CORE_COUNT  (U(FVP_R_CLUSTER_COUNT) * \
			      U(FVP_R_MAX_CPUS_PER_CLUSTER) * \
			      U(FVP_R_MAX_PE_PER_CPU))

#define PLAT_NUM_PWR_DOMAINS (U(FVP_R_CLUSTER_COUNT) + \
			      PLATFORM_CORE_COUNT + U(1))

#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL2

/*
 * Other platform porting definitions are provided by included headers
 */

/*
 * Required ARM standard platform porting definitions
 */
#define PLAT_ARM_CLUSTER_COUNT		U(FVP_R_CLUSTER_COUNT)
#define PLAT_ARM_DRAM1_SIZE		ULL(0x7fffffff)
#define PLAT_ARM_TRUSTED_SRAM_SIZE	UL(0x00040000)	/* 256 KB */
#define PLAT_ARM_TRUSTED_ROM_SIZE	UL(0x04000000)	/* 64 MB */
#define PLAT_ARM_TRUSTED_DRAM_SIZE	UL(0x02000000)	/* 32 MB */

/* These two are defined thus in arm_def.h, but doesn't seem to see it... */
#define PLAT_BL1_RO_LIMIT               (BL1_RO_BASE \
					+ PLAT_ARM_TRUSTED_ROM_SIZE)

#define PLAT_ARM_SYS_CNTCTL_BASE	UL(0xaa430000)
#define PLAT_ARM_SYS_CNTREAD_BASE	UL(0xaa800000)
#define PLAT_ARM_SYS_TIMCTL_BASE	UL(0xaa810000)
#define PLAT_ARM_SYS_CNT_BASE_S		UL(0xaa820000)
#define PLAT_ARM_SYS_CNT_BASE_NS	UL(0xaa830000)
#define PLAT_ARM_SP805_TWDG_BASE	UL(0xaa490000)

/* virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xc0000000)

/* No SCP in FVP_R */
#define PLAT_ARM_SCP_TZC_DRAM1_SIZE	UL(0x0)

#define PLAT_ARM_DRAM2_SIZE		UL(0x80000000)

#define PLAT_HW_CONFIG_DTB_SIZE		ULL(0x8000)

#define ARM_DTB_DRAM_NS			MAP_REGION_FLAT(		\
					PLAT_HW_CONFIG_DTB_BASE,	\
					PLAT_HW_CONFIG_DTB_SIZE,	\
					MT_MEMORY | MT_RO | MT_NS)

#define V2M_FVP_R_SYSREGS_BASE		UL(0x9c010000)

/*
 * Load address of BL33 for this platform port,
 * U-Boot specifically must be loaded at a 4K aligned address.
 */
#define PLAT_ARM_NS_IMAGE_BASE		(PLAT_ARM_DRAM1_BASE + 0x1000)

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#if !USE_ROMLIB
# define PLAT_ARM_MMAP_ENTRIES		11
# define MAX_XLAT_TABLES		5
#else
# define PLAT_ARM_MMAP_ENTRIES		12
# define MAX_XLAT_TABLES		6
#endif
# define N_MPU_REGIONS			16  /* number of MPU regions */
# define ALL_MPU_EL2_REGIONS_USED	0xffffffff
	/* this is the PRENR_EL2 value if all MPU regions are in use */

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
#define FVP_R_BL2_ROMLIB_OPTIMIZATION UL(0x6000)
#else
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0)
#define FVP_R_BL2_ROMLIB_OPTIMIZATION UL(0)
#endif

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth.
 */
#if TRUSTED_BOARD_BOOT
#if COT_DESC_IN_DTB
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x1E000) - FVP_R_BL2_ROMLIB_OPTIMIZATION)
#else
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x1D000) - FVP_R_BL2_ROMLIB_OPTIMIZATION)
#endif
#else
# define PLAT_ARM_MAX_BL2_SIZE	(UL(0x13000) - FVP_R_BL2_ROMLIB_OPTIMIZATION)
#endif

/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE is
 * calculated using the current BL31 PROGBITS debug size plus the sizes of
 * BL2 and BL1-RW
 */
#define PLAT_ARM_MAX_BL31_SIZE		UL(0x3D000)

/*
 * Size of cacheable stacks
 */
#if defined(IMAGE_BL1)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		UL(0x1000)
# else
#  define PLATFORM_STACK_SIZE		UL(0x500)
# endif
#endif

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

/*
 * These nominally reserve the last block of flash for PSCI MEM PROTECT flag,
 * but no PSCI in FVP_R platform, so reserve nothing:
 */
#define PLAT_ARM_FLASH_IMAGE_BASE	(PLAT_ARM_DRAM1_BASE + UL(0x40000000))
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(PLAT_ARM_DRAM1_SIZE - UL(0x40000000))

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

/* CCI related constants */
#define PLAT_FVP_R_CCI400_BASE		UL(0xac090000)
#define PLAT_FVP_R_CCI400_CLUS0_SL_PORT	3
#define PLAT_FVP_R_CCI400_CLUS1_SL_PORT	4

/* CCI-500/CCI-550 on Base platform */
#define PLAT_FVP_R_CCI5XX_BASE		UL(0xaa000000)
#define PLAT_FVP_R_CCI5XX_CLUS0_SL_PORT	5
#define PLAT_FVP_R_CCI5XX_CLUS1_SL_PORT	6

/* System timer related constants */
#define PLAT_ARM_NSTIMER_FRAME_ID	U(1)

/* Mailbox base address */
#define PLAT_ARM_TRUSTED_MAILBOX_BASE	ARM_TRUSTED_SRAM_BASE


/* TrustZone controller related constants
 *
 * Currently only filters 0 and 2 are connected on Base FVP_R.
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
#define PLAT_ARM_TZC_BASE		UL(0xaa4a0000)
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT(0)

#define PLAT_ARM_TZC_NS_DEV_ACCESS	(				\
		TZC_REGION_ACCESS_RDWR(FVP_R_NSAID_DEFAULT)	|	\
		TZC_REGION_ACCESS_RDWR(FVP_R_NSAID_PCI)		|	\
		TZC_REGION_ACCESS_RDWR(FVP_R_NSAID_AP)		|	\
		TZC_REGION_ACCESS_RDWR(FVP_R_NSAID_VIRTIO)	|	\
		TZC_REGION_ACCESS_RDWR(FVP_R_NSAID_VIRTIO_OLD))

/*
 * GIC related constants to cater for both GICv2 and GICv3 instances of an
 * FVP_R. They could be overridden at runtime in case the FVP_R implements the
 * legacy VE memory map.
 */
#define PLAT_ARM_GICD_BASE		BASE_GICD_BASE
#define PLAT_ARM_GICR_BASE		BASE_GICR_BASE
#define PLAT_ARM_GICC_BASE		BASE_GICC_BASE

#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +	\
					 PLAT_SP_IMAGE_NS_BUF_SIZE)

#define PLAT_SP_PRI			PLAT_RAS_PRI

/*
 * Physical and virtual address space limits for MPU in AARCH64 & AARCH32 modes
 */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 36)

#define ARM_SOC_CONTINUATION_SHIFT	U(24)
#define ARM_SOC_IDENTIFICATION_SHIFT	U(16)

#endif /* FVP_R_PLATFORM_DEF_H */
