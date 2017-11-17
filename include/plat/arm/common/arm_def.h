/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __ARM_DEF_H__
#define __ARM_DEF_H__

#include <arch.h>
#include <common_def.h>
#include <gic_common.h>
#include <interrupt_props.h>
#include <platform_def.h>
#include <tbbr_img_def.h>
#include <utils_def.h>
#include <xlat_tables_defs.h>


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
#define ARM_TZC_DRAM1_SIZE		ULL(0x01000000)

#define ARM_SCP_TZC_DRAM1_BASE		(ARM_DRAM1_BASE +		\
					 ARM_DRAM1_SIZE -		\
					 ARM_SCP_TZC_DRAM1_SIZE)
#define ARM_SCP_TZC_DRAM1_SIZE		PLAT_ARM_SCP_TZC_DRAM1_SIZE
#define ARM_SCP_TZC_DRAM1_END		(ARM_SCP_TZC_DRAM1_BASE +	\
					 ARM_SCP_TZC_DRAM1_SIZE - 1)

/*
 * Define a 2MB region within the TZC secured DRAM for use by EL3 runtime
 * firmware. This region is meant to be NOLOAD and will not be zero
 * initialized. Data sections with the attribute `arm_el3_tzc_dram` will be
 * placed here.
 */
#define ARM_EL3_TZC_DRAM1_BASE		(ARM_SCP_TZC_DRAM1_BASE - ARM_EL3_TZC_DRAM1_SIZE)
#define ARM_EL3_TZC_DRAM1_SIZE		ULL(0x00200000) /* 2 MB */
#define ARM_EL3_TZC_DRAM1_END		(ARM_EL3_TZC_DRAM1_BASE +	\
					ARM_EL3_TZC_DRAM1_SIZE - 1)

#define ARM_AP_TZC_DRAM1_BASE		(ARM_DRAM1_BASE +		\
					 ARM_DRAM1_SIZE -		\
					 ARM_TZC_DRAM1_SIZE)
#define ARM_AP_TZC_DRAM1_SIZE		(ARM_TZC_DRAM1_SIZE -		\
					 (ARM_SCP_TZC_DRAM1_SIZE +	\
					 ARM_EL3_TZC_DRAM1_SIZE))
#define ARM_AP_TZC_DRAM1_END		(ARM_AP_TZC_DRAM1_BASE +	\
					 ARM_AP_TZC_DRAM1_SIZE - 1)

/* Define the Access permissions for Secure peripherals to NS_DRAM */
#if ARM_CRYPTOCELL_INTEG
/*
 * Allow Secure peripheral to read NS DRAM when integrated with CryptoCell.
 * This is required by CryptoCell to authenticate BL33 which is loaded
 * into the Non Secure DDR.
 */
#define ARM_TZC_NS_DRAM_S_ACCESS	TZC_REGION_S_RD
#else
#define ARM_TZC_NS_DRAM_S_ACCESS	TZC_REGION_S_NONE
#endif

#ifdef SPD_opteed
/*
 * BL2 needs to map 4MB at the end of TZC_DRAM1 in order to
 * load/authenticate the trusted os extra image. The first 512KB of
 * TZC_DRAM1 are reserved for trusted os (OPTEE). The extra image loading
 * for OPTEE is paged image which only include the paging part using
 * virtual memory but without "init" data. OPTEE will copy the "init" data
 * (from pager image) to the first 512KB of TZC_DRAM, and then copy the
 * extra image behind the "init" data.
 */
#define ARM_OPTEE_PAGEABLE_LOAD_BASE	(ARM_AP_TZC_DRAM1_BASE + \
					 ARM_AP_TZC_DRAM1_SIZE - \
					 ARM_OPTEE_PAGEABLE_LOAD_SIZE)
#define ARM_OPTEE_PAGEABLE_LOAD_SIZE	0x400000
#define ARM_OPTEE_PAGEABLE_LOAD_MEM	MAP_REGION_FLAT(		\
					ARM_OPTEE_PAGEABLE_LOAD_BASE,	\
					ARM_OPTEE_PAGEABLE_LOAD_SIZE,	\
					MT_MEMORY | MT_RW | MT_SECURE)

/*
 * Map the memory for the OP-TEE core (also known as OP-TEE pager when paging
 * support is enabled).
 */
#define ARM_MAP_OPTEE_CORE_MEM		MAP_REGION_FLAT(		\
						BL32_BASE,		\
						BL32_LIMIT - BL32_BASE,	\
						MT_MEMORY | MT_RW | MT_SECURE)
#endif /* SPD_opteed */

#define ARM_NS_DRAM1_BASE		ARM_DRAM1_BASE
#define ARM_NS_DRAM1_SIZE		(ARM_DRAM1_SIZE -		\
					 ARM_TZC_DRAM1_SIZE)
#define ARM_NS_DRAM1_END		(ARM_NS_DRAM1_BASE +		\
					 ARM_NS_DRAM1_SIZE - 1)

#define ARM_DRAM1_BASE			ULL(0x80000000)
#define ARM_DRAM1_SIZE			ULL(0x80000000)
#define ARM_DRAM1_END			(ARM_DRAM1_BASE +		\
					 ARM_DRAM1_SIZE - 1)

#define ARM_DRAM2_BASE			ULL(0x880000000)
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
 * List of secure interrupts are deprecated, but are retained only to support
 * legacy configurations.
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

/*
 * Define a list of Group 1 Secure and Group 0 interrupt properties as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

#define ARM_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

#define ARM_MAP_SHARED_RAM		MAP_REGION_FLAT(		\
						ARM_SHARED_RAM_BASE,	\
						ARM_SHARED_RAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_MAP_NS_DRAM1		MAP_REGION_FLAT(		\
						ARM_NS_DRAM1_BASE,	\
						ARM_NS_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_NS)

#define ARM_MAP_DRAM2			MAP_REGION_FLAT(		\
						ARM_DRAM2_BASE,		\
						ARM_DRAM2_SIZE,		\
						MT_MEMORY | MT_RW | MT_NS)
#ifdef SPD_tspd

#define ARM_MAP_TSP_SEC_MEM		MAP_REGION_FLAT(		\
						TSP_SEC_MEM_BASE,	\
						TSP_SEC_MEM_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)
#endif

#if ARM_BL31_IN_DRAM
#define ARM_MAP_BL31_SEC_DRAM		MAP_REGION_FLAT(		\
						BL31_BASE,		\
						PLAT_ARM_MAX_BL31_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)
#endif

#define ARM_MAP_EL3_TZC_DRAM		MAP_REGION_FLAT(			\
						ARM_EL3_TZC_DRAM1_BASE,	\
						ARM_EL3_TZC_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)

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

/*
 * We need to access DRAM2 from BL2 for PSCI_MEM_PROTECT for
 * AArch64 builds
 */
#ifdef AARCH64
#define PLAT_PHY_ADDR_SPACE_SIZE			(1ull << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE			(1ull << 36)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE			(1ull << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE			(1ull << 32)
#endif


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
 * For AArch64 BL31 is loaded in the DRAM.
 * Put BL2 just below BL1.
 */
#define BL2_BASE			(BL1_RW_BASE - PLAT_ARM_MAX_BL2_SIZE)
#define BL2_LIMIT			BL1_RW_BASE

#elif BL2_AT_EL3

#define BL2_BASE			ARM_BL_RAM_BASE
#define BL2_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)

#elif defined(AARCH32) || JUNO_AARCH32_EL3_RUNTIME
/*
 * Put BL2 just below BL32.
 */
#define BL2_BASE			(BL32_BASE - PLAT_ARM_MAX_BL2_SIZE)
#define BL2_LIMIT			BL32_BASE

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
#elif (RESET_TO_BL31)
/*
 * Put BL31_BASE in the middle of the Trusted SRAM.
 */
#define BL31_BASE			(ARM_TRUSTED_SRAM_BASE + \
						(PLAT_ARM_TRUSTED_SRAM_SIZE >> 1))
#define BL31_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)
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

#if defined(AARCH32) || JUNO_AARCH32_EL3_RUNTIME
/*******************************************************************************
 * BL32 specific defines for EL3 runtime in AArch32 mode
 ******************************************************************************/
# if RESET_TO_SP_MIN && !JUNO_AARCH32_EL3_RUNTIME
/* SP_MIN is the only BL image in SRAM. Allocate the whole of SRAM to BL32 */
#  define BL32_BASE			ARM_BL_RAM_BASE
#  define BL32_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)
# else
/* Put BL32 at the top of the Trusted SRAM.*/
#  define BL32_BASE			(ARM_BL_RAM_BASE +		\
						ARM_BL_RAM_SIZE -	\
						PLAT_ARM_MAX_BL32_SIZE)
#  define BL32_PROGBITS_LIMIT		BL1_RW_BASE
#  define BL32_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)
# endif /* RESET_TO_SP_MIN && !JUNO_AARCH32_EL3_RUNTIME */

#else
/*******************************************************************************
 * BL32 specific defines for EL3 runtime in AArch64 mode
 ******************************************************************************/
/*
 * On ARM standard platforms, the TSP can execute from Trusted SRAM,
 * Trusted DRAM (if available) or the DRAM region secured by the TrustZone
 * controller.
 */
# if ENABLE_SPM
#  define TSP_SEC_MEM_BASE		(ARM_AP_TZC_DRAM1_BASE + ULL(0x200000))
#  define TSP_SEC_MEM_SIZE		(ARM_AP_TZC_DRAM1_SIZE - ULL(0x200000))
#  define BL32_BASE			(ARM_AP_TZC_DRAM1_BASE + ULL(0x200000))
#  define BL32_LIMIT			(ARM_AP_TZC_DRAM1_BASE +	\
						ARM_AP_TZC_DRAM1_SIZE)
# elif ARM_BL31_IN_DRAM
#  define TSP_SEC_MEM_BASE		(ARM_AP_TZC_DRAM1_BASE +	\
						PLAT_ARM_MAX_BL31_SIZE)
#  define TSP_SEC_MEM_SIZE		(ARM_AP_TZC_DRAM1_SIZE -	\
						PLAT_ARM_MAX_BL31_SIZE)
#  define BL32_BASE			(ARM_AP_TZC_DRAM1_BASE +	\
						PLAT_ARM_MAX_BL31_SIZE)
#  define BL32_LIMIT			(ARM_AP_TZC_DRAM1_BASE +	\
						ARM_AP_TZC_DRAM1_SIZE)
# elif ARM_TSP_RAM_LOCATION_ID == ARM_TRUSTED_SRAM_ID
#  define TSP_SEC_MEM_BASE		ARM_BL_RAM_BASE
#  define TSP_SEC_MEM_SIZE		ARM_BL_RAM_SIZE
#  define TSP_PROGBITS_LIMIT		BL2_BASE
#  define BL32_BASE			ARM_BL_RAM_BASE
#  define BL32_LIMIT			BL31_BASE
# elif ARM_TSP_RAM_LOCATION_ID == ARM_TRUSTED_DRAM_ID
#  define TSP_SEC_MEM_BASE		PLAT_ARM_TRUSTED_DRAM_BASE
#  define TSP_SEC_MEM_SIZE		PLAT_ARM_TRUSTED_DRAM_SIZE
#  define BL32_BASE			PLAT_ARM_TRUSTED_DRAM_BASE
#  define BL32_LIMIT			(PLAT_ARM_TRUSTED_DRAM_BASE	\
						+ (1 << 21))
# elif ARM_TSP_RAM_LOCATION_ID == ARM_DRAM_ID
#  define TSP_SEC_MEM_BASE		ARM_AP_TZC_DRAM1_BASE
#  define TSP_SEC_MEM_SIZE		ARM_AP_TZC_DRAM1_SIZE
#  define BL32_BASE			ARM_AP_TZC_DRAM1_BASE
#  define BL32_LIMIT			(ARM_AP_TZC_DRAM1_BASE +	\
						ARM_AP_TZC_DRAM1_SIZE)
# else
#  error "Unsupported ARM_TSP_RAM_LOCATION_ID value"
# endif
#endif /* AARCH32 || JUNO_AARCH32_EL3_RUNTIME */

/*
 * BL32 is mandatory in AArch32. In AArch64, undefine BL32_BASE if there is no
 * SPD and no SPM, as they are the only ones that can be used as BL32.
 */
#if !(defined(AARCH32) || JUNO_AARCH32_EL3_RUNTIME)
# if defined(SPD_none) && !ENABLE_SPM
#  undef BL32_BASE
# endif /* defined(SPD_none) && !ENABLE_SPM */
#endif /* !(defined(AARCH32) || JUNO_AARCH32_EL3_RUNTIME) */

/*******************************************************************************
 * FWU Images: NS_BL1U, BL2U & NS_BL2U defines.
 ******************************************************************************/
#define BL2U_BASE			BL2_BASE
#define BL2U_LIMIT			BL2_LIMIT

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

/* Priority levels for ARM platforms */
#define PLAT_SDEI_CRITICAL_PRI		0x60
#define PLAT_SDEI_NORMAL_PRI		0x70

/* ARM platforms use 3 upper bits of secure interrupt priority */
#define ARM_PRI_BITS			3

/* SGI used for SDEI signalling */
#define ARM_SDEI_SGI			ARM_IRQ_SEC_SGI_0

/* ARM SDEI dynamic private event numbers */
#define ARM_SDEI_DP_EVENT_0		1000
#define ARM_SDEI_DP_EVENT_1		1001
#define ARM_SDEI_DP_EVENT_2		1002

/* ARM SDEI dynamic shared event numbers */
#define ARM_SDEI_DS_EVENT_0		2000
#define ARM_SDEI_DS_EVENT_1		2001
#define ARM_SDEI_DS_EVENT_2		2002

#define ARM_SDEI_PRIVATE_EVENTS \
	SDEI_DEFINE_EVENT_0(ARM_SDEI_SGI), \
	SDEI_PRIVATE_EVENT(ARM_SDEI_DP_EVENT_0, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC), \
	SDEI_PRIVATE_EVENT(ARM_SDEI_DP_EVENT_1, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC), \
	SDEI_PRIVATE_EVENT(ARM_SDEI_DP_EVENT_2, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC)

#define ARM_SDEI_SHARED_EVENTS \
	SDEI_SHARED_EVENT(ARM_SDEI_DS_EVENT_0, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC), \
	SDEI_SHARED_EVENT(ARM_SDEI_DS_EVENT_1, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC), \
	SDEI_SHARED_EVENT(ARM_SDEI_DS_EVENT_2, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC)

#endif /* __ARM_DEF_H__ */
