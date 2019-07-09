/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#ifndef __ASSEMBLER__
#include <stdlib.h>
#endif

#include <arch.h>

#include "rcar_def.h"

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
 #define FIRMWARE_WELCOME_STR	"Booting Rcar-gen3 Trusted Firmware\n"

/* Size of cacheable stacks */
#if IMAGE_BL1
#if TRUSTED_BOARD_BOOT
#define PLATFORM_STACK_SIZE 	U(0x1000)
#else
#define PLATFORM_STACK_SIZE 	U(0x440)
#endif
#elif IMAGE_BL2
#if TRUSTED_BOARD_BOOT
#define PLATFORM_STACK_SIZE 	U(0x1000)
#else
#define PLATFORM_STACK_SIZE 	U(0x400)
#endif
#elif IMAGE_BL31
#define PLATFORM_STACK_SIZE 	U(0x400)
#elif IMAGE_BL32
#define PLATFORM_STACK_SIZE 	U(0x440)
#endif

#define BL332_IMAGE_ID		(NS_BL2U_IMAGE_ID + 1)
#define BL333_IMAGE_ID		(NS_BL2U_IMAGE_ID + 2)
#define BL334_IMAGE_ID		(NS_BL2U_IMAGE_ID + 3)
#define BL335_IMAGE_ID		(NS_BL2U_IMAGE_ID + 4)
#define BL336_IMAGE_ID		(NS_BL2U_IMAGE_ID + 5)
#define BL337_IMAGE_ID		(NS_BL2U_IMAGE_ID + 6)
#define BL338_IMAGE_ID		(NS_BL2U_IMAGE_ID + 7)

#define BL332_KEY_CERT_ID	(NS_BL2U_IMAGE_ID + 8)
#define BL333_KEY_CERT_ID	(NS_BL2U_IMAGE_ID + 9)
#define BL334_KEY_CERT_ID	(NS_BL2U_IMAGE_ID + 10)
#define BL335_KEY_CERT_ID	(NS_BL2U_IMAGE_ID + 11)
#define BL336_KEY_CERT_ID	(NS_BL2U_IMAGE_ID + 12)
#define BL337_KEY_CERT_ID	(NS_BL2U_IMAGE_ID + 13)
#define BL338_KEY_CERT_ID	(NS_BL2U_IMAGE_ID + 14)

#define BL332_CERT_ID		(NS_BL2U_IMAGE_ID + 15)
#define BL333_CERT_ID		(NS_BL2U_IMAGE_ID + 16)
#define BL334_CERT_ID		(NS_BL2U_IMAGE_ID + 17)
#define BL335_CERT_ID		(NS_BL2U_IMAGE_ID + 18)
#define BL336_CERT_ID		(NS_BL2U_IMAGE_ID + 19)
#define BL337_CERT_ID		(NS_BL2U_IMAGE_ID + 20)
#define BL338_CERT_ID		(NS_BL2U_IMAGE_ID + 21)

/* io drivers id */
#define FLASH_DEV_ID		U(0)
#define EMMC_DEV_ID		U(1)

/*
 * R-Car H3 Cortex-A57
 * L1:I/48KB(16KBx3way) D/32KB(16KBx2way) L2:2MB(128KBx16way)
 *          Cortex-A53
 * L1:I/32KB(16KBx2way) D/32KB(8KBx4way) L2:512KB(32KBx16way)
 */
#define PLATFORM_CACHE_LINE_SIZE	64
#define PLATFORM_CLUSTER_COUNT		U(2)
#define PLATFORM_CLUSTER0_CORE_COUNT	U(4)
#define PLATFORM_CLUSTER1_CORE_COUNT	U(4)
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER1_CORE_COUNT + \
					 PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(4)

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CORE_COUNT + \
					 PLATFORM_CLUSTER_COUNT + 1)

#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

#define MAX_IO_DEVICES			U(3)
#define MAX_IO_HANDLES			U(4)

/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/
/* Put BL2 just below BL3-1. BL2_BASE is calculated using the current BL2 debug
 * size plus a little space for growth. */
#define RCAR_SYSRAM_BASE		U(0xE6300000)
#if (RCAR_LSI == RCAR_E3) || (RCAR_LSI == RCAR_D3)
#define BL2_LIMIT			U(0xE6320000)
#else
#define BL2_LIMIT			U(0xE6360000)
#endif

#if (RCAR_LSI == RCAR_E3) || (RCAR_LSI == RCAR_D3)
#define BL2_BASE			U(0xE6304000)
#define BL2_IMAGE_LIMIT			U(0xE6318000)
#elif (RCAR_LSI == RCAR_V3M)
#define BL2_BASE			U(0xE6344000)
#define BL2_IMAGE_LIMIT			U(0xE636E800)
#else
#define BL2_BASE			U(0xE6304000)
#define BL2_IMAGE_LIMIT			U(0xE632E800)
#endif
#define RCAR_SYSRAM_SIZE		(BL2_BASE - RCAR_SYSRAM_BASE)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/* Put BL3-1 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL3-1 debug size plus a little space for growth. */
#define BL31_BASE		(RCAR_TRUSTED_SRAM_BASE)
#define BL31_LIMIT		(RCAR_TRUSTED_SRAM_BASE + \
				 RCAR_TRUSTED_SRAM_SIZE)
#define	RCAR_BL31_LOG_BASE	(0x44040000)
#define	RCAR_BL31_SDRAM_BTM	(RCAR_BL31_LOG_BASE + 0x14000)
#define	RCAR_BL31_LOG_SIZE	(RCAR_BL31_SDRAM_BTM - RCAR_BL31_LOG_BASE)
#define BL31_SRAM_BASE		(DEVICE_SRAM_BASE)
#define BL31_SRAM_LIMIT		(DEVICE_SRAM_BASE + DEVICE_SRAM_SIZE)

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
#ifndef SPD_NONE
#define BL32_BASE		U(0x44100000)
#define BL32_LIMIT		(BL32_BASE + U(0x100000))
#endif

/*******************************************************************************
 * BL33
 ******************************************************************************/
#define BL33_BASE		DRAM1_NS_BASE


/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#if IMAGE_BL1
#define MAX_XLAT_TABLES		U(2)
#elif IMAGE_BL2
#define MAX_XLAT_TABLES		U(5)
#elif IMAGE_BL31
#define MAX_XLAT_TABLES		U(4)
#elif IMAGE_BL32
#define MAX_XLAT_TABLES		U(3)
#endif

#if IMAGE_BL2
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 40)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 32)
#endif

#define MAX_MMAP_REGIONS	(RCAR_MMAP_ENTRIES + RCAR_BL_REGIONS)

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT   (6)
#define CACHE_WRITEBACK_GRANULE (1 << CACHE_WRITEBACK_SHIFT)

/*******************************************************************************
 * Size of the per-cpu data in bytes that should be reserved in the generic
 * per-cpu data structure for the RCAR port.
 ******************************************************************************/
#if !USE_COHERENT_MEM
#define PLAT_PCPU_DATA_SIZE	(2)
#endif

#endif /* PLATFORM_DEF_H */
