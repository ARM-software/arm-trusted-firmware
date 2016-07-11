/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#ifndef __ARM_DEF_H__
#define __ARM_DEF_H__

#include <arch.h>
#include <common_def.h>
#include <platform_def.h>
#include <tbbr_img_def.h>
#include <xlat_tables.h>


/******************************************************************************
 * Definitions common to all ARM standard platforms
 *****************************************************************************/

/* Special value used to verify platform parameters from BL2 to BL31 */
#define ARM_BL31_PLAT_PARAM_VAL		0x0f1e2d3c4b5a6978ULL

#define ARM_SYSTEM_COUNT		1

#define ARM_CACHE_WRITEBACK_SHIFT	6

/*
 * Macros mapping the MPIDR Affinity levels to ARM Platform Power levels. The
 * power levels have a 1:1 mapping with the MPIDR affinity levels.
 */
#define ARM_PWR_LVL0		MPIDR_AFFLVL0
#define ARM_PWR_LVL1		MPIDR_AFFLVL1
#define ARM_PWR_LVL2		MPIDR_AFFLVL2

/*
 *  Macros for local power states in ARM platforms encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define ARM_LOCAL_STATE_RUN	0
/* Local power state for retention. Valid only for CPU power domains */
#define ARM_LOCAL_STATE_RET	1
/* Local power state for OFF/power-down. Valid for CPU and cluster power
   domains */
#define ARM_LOCAL_STATE_OFF	2

/* Memory location options for TSP */
#define ARM_TRUSTED_SRAM_ID		0
#define ARM_TRUSTED_DRAM_ID		1
#define ARM_DRAM_ID			2

/* The first 4KB of Trusted SRAM are used as shared memory */
#define ARM_TRUSTED_SRAM_BASE		0x04000000
#define ARM_SHARED_RAM_BASE		ARM_TRUSTED_SRAM_BASE
#define ARM_SHARED_RAM_SIZE		0x00001000	/* 4 KB */

/* The remaining Trusted SRAM is used to load the BL images */
#define ARM_BL_RAM_BASE			(ARM_SHARED_RAM_BASE +	\
					 ARM_SHARED_RAM_SIZE)
#define ARM_BL_RAM_SIZE			(PLAT_ARM_TRUSTED_SRAM_SIZE -	\
					 ARM_SHARED_RAM_SIZE)

/*
 * The top 16MB of DRAM1 is configured as secure access only using the TZC
 *   - SCP TZC DRAM: If present, DRAM reserved for SCP use
 *   - AP TZC DRAM: The remaining TZC secured DRAM reserved for AP use
 */
#define ARM_TZC_DRAM1_SIZE		MAKE_ULL(0x01000000)

#define ARM_SCP_TZC_DRAM1_BASE		(ARM_DRAM1_BASE +		\
					 ARM_DRAM1_SIZE -		\
					 ARM_SCP_TZC_DRAM1_SIZE)
#define ARM_SCP_TZC_DRAM1_SIZE		PLAT_ARM_SCP_TZC_DRAM1_SIZE
#define ARM_SCP_TZC_DRAM1_END		(ARM_SCP_TZC_DRAM1_BASE +	\
					 ARM_SCP_TZC_DRAM1_SIZE - 1)

#define ARM_AP_TZC_DRAM1_BASE		(ARM_DRAM1_BASE +		\
					 ARM_DRAM1_SIZE -		\
					 ARM_TZC_DRAM1_SIZE)
#define ARM_AP_TZC_DRAM1_SIZE		(ARM_TZC_DRAM1_SIZE -		\
					 ARM_SCP_TZC_DRAM1_SIZE)
#define ARM_AP_TZC_DRAM1_END		(ARM_AP_TZC_DRAM1_BASE +	\
					 ARM_AP_TZC_DRAM1_SIZE - 1)


#define ARM_NS_DRAM1_BASE		ARM_DRAM1_BASE
#define ARM_NS_DRAM1_SIZE		(ARM_DRAM1_SIZE -		\
					 ARM_TZC_DRAM1_SIZE)
#define ARM_NS_DRAM1_END		(ARM_NS_DRAM1_BASE +		\
					 ARM_NS_DRAM1_SIZE - 1)

#define ARM_DRAM1_BASE			MAKE_ULL(0x80000000)
#define ARM_DRAM1_SIZE			MAKE_ULL(0x80000000)
#define ARM_DRAM1_END			(ARM_DRAM1_BASE +		\
					 ARM_DRAM1_SIZE - 1)

#define ARM_DRAM2_BASE			MAKE_ULL(0x880000000)
#define ARM_DRAM2_SIZE			PLAT_ARM_DRAM2_SIZE
#define ARM_DRAM2_END			(ARM_DRAM2_BASE +		\
					 ARM_DRAM2_SIZE - 1)

#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define ARM_G1S_IRQS			ARM_IRQ_SEC_PHY_TIMER,		\
					ARM_IRQ_SEC_SGI_1,		\
					ARM_IRQ_SEC_SGI_2,		\
					ARM_IRQ_SEC_SGI_3,		\
					ARM_IRQ_SEC_SGI_4,		\
					ARM_IRQ_SEC_SGI_5,		\
					ARM_IRQ_SEC_SGI_7

#define ARM_G0_IRQS			ARM_IRQ_SEC_SGI_0,		\
					ARM_IRQ_SEC_SGI_6

#define ARM_MAP_SHARED_RAM		MAP_REGION_FLAT(		\
						ARM_SHARED_RAM_BASE,	\
						ARM_SHARED_RAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_MAP_NS_DRAM1		MAP_REGION_FLAT(		\
						ARM_NS_DRAM1_BASE,	\
						ARM_NS_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_NS)

#define ARM_MAP_TSP_SEC_MEM		MAP_REGION_FLAT(		\
						TSP_SEC_MEM_BASE,	\
						TSP_SEC_MEM_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)

#if ARM_BL31_IN_DRAM
#define ARM_MAP_BL31_SEC_DRAM		MAP_REGION_FLAT(		\
						BL31_BASE,		\
						PLAT_ARM_MAX_BL31_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)
#endif

/*
 * The number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#if USE_COHERENT_MEM
#define ARM_BL_REGIONS			3
#else
#define ARM_BL_REGIONS			2
#endif

#define MAX_MMAP_REGIONS		(PLAT_ARM_MMAP_ENTRIES +	\
					 ARM_BL_REGIONS)

/* Memory mapped Generic timer interfaces  */
#define ARM_SYS_CNTCTL_BASE		0x2a430000
#define ARM_SYS_CNTREAD_BASE		0x2a800000
#define ARM_SYS_TIMCTL_BASE		0x2a810000

#define ARM_CONSOLE_BAUDRATE		115200

/* Trusted Watchdog constants */
#define ARM_SP805_TWDG_BASE		0x2a490000
#define ARM_SP805_TWDG_CLK_HZ		32768
/* The TBBR document specifies a watchdog timeout of 256 seconds. SP805
 * asserts reset after two consecutive countdowns (2 x 128 = 256 sec) */
#define ARM_TWDG_TIMEOUT_SEC		128
#define ARM_TWDG_LOAD_VAL		(ARM_SP805_TWDG_CLK_HZ * 	\
					 ARM_TWDG_TIMEOUT_SEC)

/******************************************************************************
 * Required platform porting definitions common to all ARM standard platforms
 *****************************************************************************/

#define ADDR_SPACE_SIZE			(1ull << 32)

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		ARM_LOCAL_STATE_RET

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		ARM_LOCAL_STATE_OFF

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE		(1 << ARM_CACHE_WRITEBACK_SHIFT)


/*******************************************************************************
 * BL1 specific defines.
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 ******************************************************************************/
#define BL1_RO_BASE			PLAT_ARM_TRUSTED_ROM_BASE
#define BL1_RO_LIMIT			(PLAT_ARM_TRUSTED_ROM_BASE	\
					 + PLAT_ARM_TRUSTED_ROM_SIZE)
/*
 * Put BL1 RW at the top of the Trusted SRAM.
 */
#define BL1_RW_BASE			(ARM_BL_RAM_BASE +		\
						ARM_BL_RAM_SIZE -	\
						PLAT_ARM_MAX_BL1_RW_SIZE)
#define BL1_RW_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)

/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/
#if ARM_BL31_IN_DRAM
/*
 * BL31 is loaded in the DRAM.
 * Put BL2 just below BL1.
 */
#define BL2_BASE			(BL1_RW_BASE - PLAT_ARM_MAX_BL2_SIZE)
#define BL2_LIMIT			BL1_RW_BASE
#else
/*
 * Put BL2 just below BL31.
 */
#define BL2_BASE			(BL31_BASE - PLAT_ARM_MAX_BL2_SIZE)
#define BL2_LIMIT			BL31_BASE
#endif

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
#if ARM_BL31_IN_DRAM
/*
 * Put BL31 at the bottom of TZC secured DRAM
 */
#define BL31_BASE			ARM_AP_TZC_DRAM1_BASE
#define BL31_LIMIT			(ARM_AP_TZC_DRAM1_BASE +	\
						PLAT_ARM_MAX_BL31_SIZE)
#else
/*
 * Put BL31 at the top of the Trusted SRAM.
 */
#define BL31_BASE			(ARM_BL_RAM_BASE +		\
						ARM_BL_RAM_SIZE -	\
						PLAT_ARM_MAX_BL31_SIZE)
#define BL31_PROGBITS_LIMIT		BL1_RW_BASE
#define BL31_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)
#endif

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
/*
 * On ARM standard platforms, the TSP can execute from Trusted SRAM,
 * Trusted DRAM (if available) or the DRAM region secured by the TrustZone
 * controller.
 */
#if ARM_BL31_IN_DRAM
# define TSP_SEC_MEM_BASE		(ARM_AP_TZC_DRAM1_BASE +	\
						PLAT_ARM_MAX_BL31_SIZE)
# define TSP_SEC_MEM_SIZE		(ARM_AP_TZC_DRAM1_SIZE -	\
						PLAT_ARM_MAX_BL31_SIZE)
# define BL32_BASE			(ARM_AP_TZC_DRAM1_BASE +	\
						PLAT_ARM_MAX_BL31_SIZE)
# define BL32_LIMIT			(ARM_AP_TZC_DRAM1_BASE +	\
						ARM_AP_TZC_DRAM1_SIZE)
#elif ARM_TSP_RAM_LOCATION_ID == ARM_TRUSTED_SRAM_ID
# define TSP_SEC_MEM_BASE		ARM_BL_RAM_BASE
# define TSP_SEC_MEM_SIZE		ARM_BL_RAM_SIZE
# define TSP_PROGBITS_LIMIT		BL2_BASE
# define BL32_BASE			ARM_BL_RAM_BASE
# define BL32_LIMIT			BL31_BASE
#elif ARM_TSP_RAM_LOCATION_ID == ARM_TRUSTED_DRAM_ID
# define TSP_SEC_MEM_BASE		PLAT_ARM_TRUSTED_DRAM_BASE
# define TSP_SEC_MEM_SIZE		PLAT_ARM_TRUSTED_DRAM_SIZE
# define BL32_BASE			PLAT_ARM_TRUSTED_DRAM_BASE
# define BL32_LIMIT			(PLAT_ARM_TRUSTED_DRAM_BASE	\
						+ (1 << 21))
#elif ARM_TSP_RAM_LOCATION_ID == ARM_DRAM_ID
# define TSP_SEC_MEM_BASE		ARM_AP_TZC_DRAM1_BASE
# define TSP_SEC_MEM_SIZE		ARM_AP_TZC_DRAM1_SIZE
# define BL32_BASE			ARM_AP_TZC_DRAM1_BASE
# define BL32_LIMIT			(ARM_AP_TZC_DRAM1_BASE +	\
						ARM_AP_TZC_DRAM1_SIZE)
#else
# error "Unsupported ARM_TSP_RAM_LOCATION_ID value"
#endif

/* BL32 is mandatory in AArch32 */
#ifndef AARCH32
#ifdef SPD_none
#undef BL32_BASE
#endif /* SPD_none */
#endif

/*******************************************************************************
 * FWU Images: NS_BL1U, BL2U & NS_BL2U defines.
 ******************************************************************************/
#define BL2U_BASE			BL2_BASE
#if ARM_BL31_IN_DRAM
#define BL2U_LIMIT			BL1_RW_BASE
#else
#define BL2U_LIMIT			BL31_BASE
#endif
#define NS_BL2U_BASE			ARM_NS_DRAM1_BASE
#define NS_BL1U_BASE			(PLAT_ARM_NVM_BASE + 0x03EB8000)

/*
 * ID of the secure physical generic timer interrupt used by the TSP.
 */
#define TSP_IRQ_SEC_PHY_TIMER		ARM_IRQ_SEC_PHY_TIMER


/*
 * One cache line needed for bakery locks on ARM platforms
 */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE		(1 * CACHE_WRITEBACK_GRANULE)


#endif /* __ARM_DEF_H__ */
