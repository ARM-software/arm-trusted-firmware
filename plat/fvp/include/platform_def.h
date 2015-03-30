/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include "../fvp_def.h"


/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#if DEBUG_XLAT_TABLE
#define PLATFORM_STACK_SIZE 0x800
#elif IMAGE_BL1
#if TRUSTED_BOARD_BOOT
#define PLATFORM_STACK_SIZE 0x1000
#else
#define PLATFORM_STACK_SIZE 0x440
#endif
#elif IMAGE_BL2
#if TRUSTED_BOARD_BOOT
#define PLATFORM_STACK_SIZE 0x1000
#else
#define PLATFORM_STACK_SIZE 0x400
#endif
#elif IMAGE_BL31
#define PLATFORM_STACK_SIZE 0x400
#elif IMAGE_BL32
#define PLATFORM_STACK_SIZE 0x440
#endif

#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_NAME			"bl2.bin"

/* EL3 Runtime Firmware BL31 */
#define BL31_IMAGE_NAME			"bl31.bin"

/* Secure Payload BL32 (Trusted OS) */
#define BL32_IMAGE_NAME			"bl32.bin"

/* Non-Trusted Firmware BL33 */
#define BL33_IMAGE_NAME			"bl33.bin" /* e.g. UEFI */

#if TRUSTED_BOARD_BOOT
/* Certificates */
# define BL2_CERT_NAME			"bl2.crt"
# define TRUSTED_KEY_CERT_NAME		"trusted_key.crt"

# define BL30_KEY_CERT_NAME		"bl30_key.crt"
# define BL31_KEY_CERT_NAME		"bl31_key.crt"
# define BL32_KEY_CERT_NAME		"bl32_key.crt"
# define BL33_KEY_CERT_NAME		"bl33_key.crt"

# define BL30_CERT_NAME			"bl30.crt"
# define BL31_CERT_NAME			"bl31.crt"
# define BL32_CERT_NAME			"bl32.crt"
# define BL33_CERT_NAME			"bl33.crt"
#endif /* TRUSTED_BOARD_BOOT */

#define PLATFORM_CACHE_LINE_SIZE	64
#define PLATFORM_CLUSTER_COUNT		2ull
#define PLATFORM_CLUSTER0_CORE_COUNT	4
#define PLATFORM_CLUSTER1_CORE_COUNT	4
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER1_CORE_COUNT + \
						PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	4
#define PLATFORM_NUM_AFFS		(PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)
#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

/*******************************************************************************
 * BL1 specific defines.
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 ******************************************************************************/
#define BL1_RO_BASE			FVP_TRUSTED_ROM_BASE
#define BL1_RO_LIMIT			(FVP_TRUSTED_ROM_BASE \
					+ FVP_TRUSTED_ROM_SIZE)
/*
 * Put BL1 RW at the top of the Trusted SRAM. BL1_RW_BASE is calculated using
 * the current BL1 RW debug size plus a little space for growth.
 */
#if TRUSTED_BOARD_BOOT
#define BL1_RW_BASE			(FVP_TRUSTED_SRAM_BASE \
					+ FVP_TRUSTED_SRAM_SIZE - 0x8000)
#else
#define BL1_RW_BASE			(FVP_TRUSTED_SRAM_BASE \
					+ FVP_TRUSTED_SRAM_SIZE - 0x6000)
#endif
#define BL1_RW_LIMIT			(FVP_TRUSTED_SRAM_BASE \
					+ FVP_TRUSTED_SRAM_SIZE)

/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/
/*
 * Put BL2 just below BL3-1. BL2_BASE is calculated using the current BL2 debug
 * size plus a little space for growth.
 */
#if TRUSTED_BOARD_BOOT
#define BL2_BASE			(BL31_BASE - 0x1C000)
#else
#define BL2_BASE			(BL31_BASE - 0xC000)
#endif
#define BL2_LIMIT			BL31_BASE

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL3-1 debug size plus a little space for growth.
 */
#define BL31_BASE			(FVP_TRUSTED_SRAM_BASE \
					+ FVP_TRUSTED_SRAM_SIZE - 0x1D000)
#define BL31_PROGBITS_LIMIT		BL1_RW_BASE
#define BL31_LIMIT			(FVP_TRUSTED_SRAM_BASE \
					+ FVP_TRUSTED_SRAM_SIZE)

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
/*
 * On FVP, the TSP can execute from Trusted SRAM, Trusted DRAM or the DRAM
 * region secured by the TrustZone controller.
 */
#if FVP_TSP_RAM_LOCATION_ID == FVP_TRUSTED_SRAM_ID
# define TSP_SEC_MEM_BASE		FVP_TRUSTED_SRAM_BASE
# define TSP_SEC_MEM_SIZE		FVP_TRUSTED_SRAM_SIZE
# define TSP_PROGBITS_LIMIT		BL2_BASE
# define BL32_BASE			FVP_TRUSTED_SRAM_BASE
# define BL32_LIMIT			BL31_BASE
#elif FVP_TSP_RAM_LOCATION_ID == FVP_TRUSTED_DRAM_ID
# define TSP_SEC_MEM_BASE		FVP_TRUSTED_DRAM_BASE
# define TSP_SEC_MEM_SIZE		FVP_TRUSTED_DRAM_SIZE
# define BL32_BASE			FVP_TRUSTED_DRAM_BASE
# define BL32_LIMIT			(FVP_TRUSTED_DRAM_BASE + (1 << 21))
#elif FVP_TSP_RAM_LOCATION_ID == FVP_DRAM_ID
# define TSP_SEC_MEM_BASE		DRAM1_SEC_BASE
# define TSP_SEC_MEM_SIZE		DRAM1_SEC_SIZE
# define BL32_BASE			DRAM1_SEC_BASE
# define BL32_LIMIT			(DRAM1_SEC_BASE + DRAM1_SEC_SIZE)
#else
# error "Unsupported FVP_TSP_RAM_LOCATION_ID value"
#endif

/*
 * ID of the secure physical generic timer interrupt used by the TSP.
 */
#define TSP_IRQ_SEC_PHY_TIMER		IRQ_SEC_PHY_TIMER

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define ADDR_SPACE_SIZE			(1ull << 32)

#if IMAGE_BL1
# define MAX_XLAT_TABLES		2
#elif IMAGE_BL2
# define MAX_XLAT_TABLES		3
#elif IMAGE_BL31
# define MAX_XLAT_TABLES		2
#elif IMAGE_BL32
# if FVP_TSP_RAM_LOCATION_ID == FVP_DRAM_ID
#  define MAX_XLAT_TABLES		3
# else
#  define MAX_XLAT_TABLES		2
# endif
#endif

#define MAX_MMAP_REGIONS		16

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT   6
#define CACHE_WRITEBACK_GRANULE (1 << CACHE_WRITEBACK_SHIFT)

#if !USE_COHERENT_MEM
/*******************************************************************************
 * Size of the per-cpu data in bytes that should be reserved in the generic
 * per-cpu data structure for the FVP port.
 ******************************************************************************/
#define PLAT_PCPU_DATA_SIZE	2
#endif

/*******************************************************************************
 * SP804 timer
 ******************************************************************************/
#define SP804_BASE		0x1C110000
#define SP804_CLKDIV		1
#define SP804_CLKMULT		30

#endif /* __PLATFORM_DEF_H__ */
