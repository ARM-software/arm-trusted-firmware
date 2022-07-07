/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_DEF_H
#define ARM_DEF_H

#include <arch.h>
#include <common/interrupt_props.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/gic_common.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/arm/common/smccc_def.h>
#include <plat/common/common_def.h>

/******************************************************************************
 * Definitions common to all ARM standard platforms
 *****************************************************************************/

/*
 * Root of trust key hash lengths
 */
#define ARM_ROTPK_HEADER_LEN		19
#define ARM_ROTPK_HASH_LEN		32

/* Special value used to verify platform parameters from BL2 to BL31 */
#define ARM_BL31_PLAT_PARAM_VAL		ULL(0x0f1e2d3c4b5a6978)

#define ARM_SYSTEM_COUNT		U(1)

#define ARM_CACHE_WRITEBACK_SHIFT	6

/*
 * Macros mapping the MPIDR Affinity levels to ARM Platform Power levels. The
 * power levels have a 1:1 mapping with the MPIDR affinity levels.
 */
#define ARM_PWR_LVL0		MPIDR_AFFLVL0
#define ARM_PWR_LVL1		MPIDR_AFFLVL1
#define ARM_PWR_LVL2		MPIDR_AFFLVL2
#define ARM_PWR_LVL3		MPIDR_AFFLVL3

/*
 *  Macros for local power states in ARM platforms encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define ARM_LOCAL_STATE_RUN	U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define ARM_LOCAL_STATE_RET	U(1)
/* Local power state for OFF/power-down. Valid for CPU and cluster power
   domains */
#define ARM_LOCAL_STATE_OFF	U(2)

/* Memory location options for TSP */
#define ARM_TRUSTED_SRAM_ID		0
#define ARM_TRUSTED_DRAM_ID		1
#define ARM_DRAM_ID			2

#ifdef PLAT_ARM_TRUSTED_SRAM_BASE
#define ARM_TRUSTED_SRAM_BASE		PLAT_ARM_TRUSTED_SRAM_BASE
#else
#define ARM_TRUSTED_SRAM_BASE		UL(0x04000000)
#endif /* PLAT_ARM_TRUSTED_SRAM_BASE */

#define ARM_SHARED_RAM_BASE		ARM_TRUSTED_SRAM_BASE
#define ARM_SHARED_RAM_SIZE		UL(0x00001000)	/* 4 KB */

/* The remaining Trusted SRAM is used to load the BL images */
#define ARM_BL_RAM_BASE			(ARM_SHARED_RAM_BASE +	\
					 ARM_SHARED_RAM_SIZE)
#define ARM_BL_RAM_SIZE			(PLAT_ARM_TRUSTED_SRAM_SIZE -	\
					 ARM_SHARED_RAM_SIZE)

/*
 * The top 16MB (or 64MB if RME is enabled) of DRAM1 is configured as
 * follows:
 *   - SCP TZC DRAM: If present, DRAM reserved for SCP use
 *   - L1 GPT DRAM: Reserved for L1 GPT if RME is enabled
 *   - REALM DRAM: Reserved for Realm world if RME is enabled
 *   - TF-A <-> RMM SHARED: Area shared for communication between TF-A and RMM
 *   - AP TZC DRAM: The remaining TZC secured DRAM reserved for AP use
 *
 *              RME enabled(64MB)                RME not enabled(16MB)
 *              --------------------             -------------------
 *              |                  |             |                 |
 *              |  AP TZC (~28MB)  |             |  AP TZC (~14MB) |
 *              --------------------             -------------------
 *              |                  |             |                 |
 *              |   REALM (RMM)    |             |  EL3 TZC (2MB)  |
 *              |   (32MB - 4KB)   |             -------------------
 *              --------------------             |                 |
 *              |                  |             |    SCP TZC      |
 *              |   TF-A <-> RMM   |  0xFFFF_FFFF-------------------
 *              |   SHARED (4KB)   |
 *              --------------------
 *              |                  |
 *              |  EL3 TZC (3MB)   |
 *              --------------------
 *              | L1 GPT + SCP TZC |
 *              |       (~1MB)     |
 *  0xFFFF_FFFF --------------------
 */
#if ENABLE_RME
#define ARM_TZC_DRAM1_SIZE              UL(0x04000000) /* 64MB */
/*
 * Define a region within the TZC secured DRAM for use by EL3 runtime
 * firmware. This region is meant to be NOLOAD and will not be zero
 * initialized. Data sections with the attribute `arm_el3_tzc_dram` will be
 * placed here. 3MB region is reserved if RME is enabled, 2MB otherwise.
 */
#define ARM_EL3_TZC_DRAM1_SIZE		UL(0x00300000) /* 3MB */
#define ARM_L1_GPT_SIZE			UL(0x00100000) /* 1MB */

/* 32MB - ARM_EL3_RMM_SHARED_SIZE */
#define ARM_REALM_SIZE			(UL(0x02000000) -		\
						ARM_EL3_RMM_SHARED_SIZE)
#define ARM_EL3_RMM_SHARED_SIZE		(PAGE_SIZE)    /* 4KB */
#else
#define ARM_TZC_DRAM1_SIZE		UL(0x01000000) /* 16MB */
#define ARM_EL3_TZC_DRAM1_SIZE		UL(0x00200000) /* 2MB */
#define ARM_L1_GPT_SIZE			UL(0)
#define ARM_REALM_SIZE			UL(0)
#define ARM_EL3_RMM_SHARED_SIZE		UL(0)
#endif /* ENABLE_RME */

#define ARM_SCP_TZC_DRAM1_BASE		(ARM_DRAM1_BASE +		\
					ARM_DRAM1_SIZE -		\
					(ARM_SCP_TZC_DRAM1_SIZE +	\
					ARM_L1_GPT_SIZE))
#define ARM_SCP_TZC_DRAM1_SIZE		PLAT_ARM_SCP_TZC_DRAM1_SIZE
#define ARM_SCP_TZC_DRAM1_END		(ARM_SCP_TZC_DRAM1_BASE +	\
					ARM_SCP_TZC_DRAM1_SIZE - 1U)
#if ENABLE_RME
#define ARM_L1_GPT_ADDR_BASE		(ARM_DRAM1_BASE +		\
					ARM_DRAM1_SIZE -		\
					ARM_L1_GPT_SIZE)
#define ARM_L1_GPT_END			(ARM_L1_GPT_ADDR_BASE +		\
					ARM_L1_GPT_SIZE - 1U)

#define ARM_REALM_BASE			(ARM_EL3_RMM_SHARED_BASE -	\
					 ARM_REALM_SIZE)

#define ARM_REALM_END                   (ARM_REALM_BASE + ARM_REALM_SIZE - 1U)

#define ARM_EL3_RMM_SHARED_BASE		(ARM_DRAM1_BASE +		\
					 ARM_DRAM1_SIZE -		\
					(ARM_SCP_TZC_DRAM1_SIZE +	\
					ARM_L1_GPT_SIZE +		\
					ARM_EL3_RMM_SHARED_SIZE +	\
					ARM_EL3_TZC_DRAM1_SIZE))

#define ARM_EL3_RMM_SHARED_END		(ARM_EL3_RMM_SHARED_BASE +	\
					 ARM_EL3_RMM_SHARED_SIZE - 1U)
#endif /* ENABLE_RME */

#define ARM_EL3_TZC_DRAM1_BASE		(ARM_SCP_TZC_DRAM1_BASE -	\
					ARM_EL3_TZC_DRAM1_SIZE)
#define ARM_EL3_TZC_DRAM1_END		(ARM_EL3_TZC_DRAM1_BASE +	\
					ARM_EL3_TZC_DRAM1_SIZE - 1U)

#define ARM_AP_TZC_DRAM1_BASE		(ARM_DRAM1_BASE +		\
					ARM_DRAM1_SIZE -		\
					ARM_TZC_DRAM1_SIZE)
#define ARM_AP_TZC_DRAM1_SIZE		(ARM_TZC_DRAM1_SIZE -		\
					(ARM_SCP_TZC_DRAM1_SIZE +	\
					ARM_EL3_TZC_DRAM1_SIZE +	\
					ARM_EL3_RMM_SHARED_SIZE +	\
					ARM_REALM_SIZE +		\
					ARM_L1_GPT_SIZE))
#define ARM_AP_TZC_DRAM1_END		(ARM_AP_TZC_DRAM1_BASE +	\
					ARM_AP_TZC_DRAM1_SIZE - 1U)

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
#define ARM_OPTEE_PAGEABLE_LOAD_SIZE	UL(0x400000)
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
					 ARM_NS_DRAM1_SIZE - 1U)
#ifdef PLAT_ARM_DRAM1_BASE
#define ARM_DRAM1_BASE			PLAT_ARM_DRAM1_BASE
#else
#define ARM_DRAM1_BASE			ULL(0x80000000)
#endif /* PLAT_ARM_DRAM1_BASE */

#define ARM_DRAM1_SIZE			ULL(0x80000000)
#define ARM_DRAM1_END			(ARM_DRAM1_BASE +		\
					 ARM_DRAM1_SIZE - 1U)

#define ARM_DRAM2_BASE			PLAT_ARM_DRAM2_BASE
#define ARM_DRAM2_SIZE			PLAT_ARM_DRAM2_SIZE
#define ARM_DRAM2_END			(ARM_DRAM2_BASE +		\
					 ARM_DRAM2_SIZE - 1U)

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
 * Define a list of Group 1 Secure and Group 0 interrupt properties as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE)

#define ARM_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE)

#define ARM_MAP_SHARED_RAM	MAP_REGION_FLAT(			\
					ARM_SHARED_RAM_BASE,		\
					ARM_SHARED_RAM_SIZE,		\
					MT_DEVICE | MT_RW | EL3_PAS)

#define ARM_MAP_NS_DRAM1	MAP_REGION_FLAT(			\
					ARM_NS_DRAM1_BASE,		\
					ARM_NS_DRAM1_SIZE,		\
					MT_MEMORY | MT_RW | MT_NS)

#define ARM_MAP_DRAM2		MAP_REGION_FLAT(			\
					ARM_DRAM2_BASE,			\
					ARM_DRAM2_SIZE,			\
					MT_MEMORY | MT_RW | MT_NS)

#define ARM_MAP_TSP_SEC_MEM	MAP_REGION_FLAT(			\
					TSP_SEC_MEM_BASE,		\
					TSP_SEC_MEM_SIZE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

#if ARM_BL31_IN_DRAM
#define ARM_MAP_BL31_SEC_DRAM	MAP_REGION_FLAT(			\
					BL31_BASE,			\
					PLAT_ARM_MAX_BL31_SIZE,		\
					MT_MEMORY | MT_RW | MT_SECURE)
#endif

#define ARM_MAP_EL3_TZC_DRAM	MAP_REGION_FLAT(			\
					ARM_EL3_TZC_DRAM1_BASE,		\
					ARM_EL3_TZC_DRAM1_SIZE,		\
					MT_MEMORY | MT_RW | EL3_PAS)

#define ARM_MAP_TRUSTED_DRAM	MAP_REGION_FLAT(			\
					PLAT_ARM_TRUSTED_DRAM_BASE,	\
					PLAT_ARM_TRUSTED_DRAM_SIZE,	\
					MT_MEMORY | MT_RW | MT_SECURE)

#if ENABLE_RME
/*
 * We add the EL3_RMM_SHARED size to RMM mapping to map the region as a block.
 * Else we end up requiring more pagetables in BL2 for ROMLIB build.
 */
#define ARM_MAP_RMM_DRAM	MAP_REGION_FLAT(			\
					PLAT_ARM_RMM_BASE,		\
					(PLAT_ARM_RMM_SIZE + 		\
					ARM_EL3_RMM_SHARED_SIZE),	\
					MT_MEMORY | MT_RW | MT_REALM)


#define ARM_MAP_GPT_L1_DRAM	MAP_REGION_FLAT(			\
					ARM_L1_GPT_ADDR_BASE,		\
					ARM_L1_GPT_SIZE,		\
					MT_MEMORY | MT_RW | EL3_PAS)

#define ARM_MAP_EL3_RMM_SHARED_MEM					\
				MAP_REGION_FLAT(			\
					ARM_EL3_RMM_SHARED_BASE,	\
					ARM_EL3_RMM_SHARED_SIZE,	\
					MT_MEMORY | MT_RW | MT_REALM)

#endif /* ENABLE_RME */

/*
 * Mapping for the BL1 RW region. This mapping is needed by BL2 in order to
 * share the Mbed TLS heap. Since the heap is allocated inside BL1, it resides
 * in the BL1 RW region. Hence, BL2 needs access to the BL1 RW region in order
 * to be able to access the heap.
 */
#define ARM_MAP_BL1_RW		MAP_REGION_FLAT(	\
					BL1_RW_BASE,	\
					BL1_RW_LIMIT - BL1_RW_BASE, \
					MT_MEMORY | MT_RW | EL3_PAS)

/*
 * If SEPARATE_CODE_AND_RODATA=1 we define a region for each section
 * otherwise one region is defined containing both.
 */
#if SEPARATE_CODE_AND_RODATA
#define ARM_MAP_BL_RO			MAP_REGION_FLAT(			\
						BL_CODE_BASE,			\
						BL_CODE_END - BL_CODE_BASE,	\
						MT_CODE | EL3_PAS),		\
					MAP_REGION_FLAT(			\
						BL_RO_DATA_BASE,		\
						BL_RO_DATA_END			\
							- BL_RO_DATA_BASE,	\
						MT_RO_DATA | EL3_PAS)
#else
#define ARM_MAP_BL_RO			MAP_REGION_FLAT(			\
						BL_CODE_BASE,			\
						BL_CODE_END - BL_CODE_BASE,	\
						MT_CODE | EL3_PAS)
#endif
#if USE_COHERENT_MEM
#define ARM_MAP_BL_COHERENT_RAM		MAP_REGION_FLAT(			\
						BL_COHERENT_RAM_BASE,		\
						BL_COHERENT_RAM_END		\
							- BL_COHERENT_RAM_BASE, \
						MT_DEVICE | MT_RW | EL3_PAS)
#endif
#if USE_ROMLIB
#define ARM_MAP_ROMLIB_CODE		MAP_REGION_FLAT(			\
						ROMLIB_RO_BASE,			\
						ROMLIB_RO_LIMIT	- ROMLIB_RO_BASE,\
						MT_CODE | EL3_PAS)

#define ARM_MAP_ROMLIB_DATA		MAP_REGION_FLAT(			\
						ROMLIB_RW_BASE,			\
						ROMLIB_RW_END	- ROMLIB_RW_BASE,\
						MT_MEMORY | MT_RW | EL3_PAS)
#endif

/*
 * Map mem_protect flash region with read and write permissions
 */
#define ARM_V2M_MAP_MEM_PROTECT		MAP_REGION_FLAT(PLAT_ARM_MEM_PROT_ADDR,	\
						V2M_FLASH_BLOCK_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)
/*
 * Map the region for device tree configuration with read and write permissions
 */
#define ARM_MAP_BL_CONFIG_REGION	MAP_REGION_FLAT(ARM_BL_RAM_BASE,	\
						(ARM_FW_CONFIGS_LIMIT		\
							- ARM_BL_RAM_BASE),	\
						MT_MEMORY | MT_RW | EL3_PAS)
/*
 * Map L0_GPT with read and write permissions
 */
#if ENABLE_RME
#define ARM_MAP_L0_GPT_REGION		MAP_REGION_FLAT(ARM_L0_GPT_ADDR_BASE,	\
						ARM_L0_GPT_SIZE,		\
						MT_MEMORY | MT_RW | MT_ROOT)
#endif

/*
 * The max number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#define ARM_BL_REGIONS			7

#define MAX_MMAP_REGIONS		(PLAT_ARM_MMAP_ENTRIES +	\
					 ARM_BL_REGIONS)

/* Memory mapped Generic timer interfaces  */
#ifdef PLAT_ARM_SYS_CNTCTL_BASE
#define ARM_SYS_CNTCTL_BASE		PLAT_ARM_SYS_CNTCTL_BASE
#else
#define ARM_SYS_CNTCTL_BASE		UL(0x2a430000)
#endif

#ifdef PLAT_ARM_SYS_CNTREAD_BASE
#define ARM_SYS_CNTREAD_BASE		PLAT_ARM_SYS_CNTREAD_BASE
#else
#define ARM_SYS_CNTREAD_BASE		UL(0x2a800000)
#endif

#ifdef PLAT_ARM_SYS_TIMCTL_BASE
#define ARM_SYS_TIMCTL_BASE		PLAT_ARM_SYS_TIMCTL_BASE
#else
#define ARM_SYS_TIMCTL_BASE		UL(0x2a810000)
#endif

#ifdef PLAT_ARM_SYS_CNT_BASE_S
#define ARM_SYS_CNT_BASE_S		PLAT_ARM_SYS_CNT_BASE_S
#else
#define ARM_SYS_CNT_BASE_S		UL(0x2a820000)
#endif

#ifdef PLAT_ARM_SYS_CNT_BASE_NS
#define ARM_SYS_CNT_BASE_NS		PLAT_ARM_SYS_CNT_BASE_NS
#else
#define ARM_SYS_CNT_BASE_NS		UL(0x2a830000)
#endif

#define ARM_CONSOLE_BAUDRATE		115200

/* Trusted Watchdog constants */
#ifdef PLAT_ARM_SP805_TWDG_BASE
#define ARM_SP805_TWDG_BASE		PLAT_ARM_SP805_TWDG_BASE
#else
#define ARM_SP805_TWDG_BASE		UL(0x2a490000)
#endif
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
#define CACHE_WRITEBACK_GRANULE		(U(1) << ARM_CACHE_WRITEBACK_SHIFT)

/*
 * To enable FW_CONFIG to be loaded by BL1, define the corresponding base
 * and limit. Leave enough space of BL2 meminfo.
 */
#define ARM_FW_CONFIG_BASE		(ARM_BL_RAM_BASE + sizeof(meminfo_t))
#define ARM_FW_CONFIG_LIMIT		((ARM_BL_RAM_BASE + PAGE_SIZE) \
					+ (PAGE_SIZE / 2U))

/*
 * Boot parameters passed from BL2 to BL31/BL32 are stored here
 */
#define ARM_BL2_MEM_DESC_BASE		(ARM_FW_CONFIG_LIMIT)
#define ARM_BL2_MEM_DESC_LIMIT		(ARM_BL2_MEM_DESC_BASE \
					+ (PAGE_SIZE / 2U))

/*
 * Define limit of firmware configuration memory:
 * ARM_FW_CONFIG + ARM_BL2_MEM_DESC memory
 */
#define ARM_FW_CONFIGS_LIMIT		(ARM_BL_RAM_BASE + (PAGE_SIZE * 2))

#if ENABLE_RME
/*
 * Store the L0 GPT on Trusted SRAM next to firmware
 * configuration memory, 4KB aligned.
 */
#define ARM_L0_GPT_SIZE			(PAGE_SIZE)
#define ARM_L0_GPT_ADDR_BASE		(ARM_FW_CONFIGS_LIMIT)
#define ARM_L0_GPT_LIMIT		(ARM_L0_GPT_ADDR_BASE + ARM_L0_GPT_SIZE)
#else
#define ARM_L0_GPT_SIZE			U(0)
#endif

/*******************************************************************************
 * BL1 specific defines.
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 ******************************************************************************/
#define BL1_RO_BASE			PLAT_ARM_TRUSTED_ROM_BASE
#ifdef PLAT_BL1_RO_LIMIT
#define BL1_RO_LIMIT			PLAT_BL1_RO_LIMIT
#else
#define BL1_RO_LIMIT			(PLAT_ARM_TRUSTED_ROM_BASE	\
					 + (PLAT_ARM_TRUSTED_ROM_SIZE - \
					    PLAT_ARM_MAX_ROMLIB_RO_SIZE))
#endif

/*
 * Put BL1 RW at the top of the Trusted SRAM.
 */
#define BL1_RW_BASE			(ARM_BL_RAM_BASE +		\
						ARM_BL_RAM_SIZE -	\
						(PLAT_ARM_MAX_BL1_RW_SIZE +\
						 PLAT_ARM_MAX_ROMLIB_RW_SIZE))
#define BL1_RW_LIMIT			(ARM_BL_RAM_BASE + 		\
					    (ARM_BL_RAM_SIZE - PLAT_ARM_MAX_ROMLIB_RW_SIZE))

#define ROMLIB_RO_BASE			BL1_RO_LIMIT
#define ROMLIB_RO_LIMIT			(PLAT_ARM_TRUSTED_ROM_BASE + PLAT_ARM_TRUSTED_ROM_SIZE)

#define ROMLIB_RW_BASE			(BL1_RW_BASE + PLAT_ARM_MAX_BL1_RW_SIZE)
#define ROMLIB_RW_END			(ROMLIB_RW_BASE + PLAT_ARM_MAX_ROMLIB_RW_SIZE)

/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/
#if BL2_AT_EL3
#if ENABLE_PIE
/*
 * As the BL31 image size appears to be increased when built with the ENABLE_PIE
 * option, set BL2 base address to have enough space for BL31 in Trusted SRAM.
 */
#define BL2_BASE			(ARM_TRUSTED_SRAM_BASE + \
					(PLAT_ARM_TRUSTED_SRAM_SIZE >> 1) + \
					0x3000)
#else
/* Put BL2 towards the middle of the Trusted SRAM */
#define BL2_BASE			(ARM_TRUSTED_SRAM_BASE + \
					(PLAT_ARM_TRUSTED_SRAM_SIZE >> 1) + \
					0x2000)
#endif /* ENABLE_PIE */
#define BL2_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)

#else
/*
 * Put BL2 just below BL1.
 */
#define BL2_BASE			(BL1_RW_BASE - PLAT_ARM_MAX_BL2_SIZE)
#define BL2_LIMIT			BL1_RW_BASE
#endif

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
#if ARM_BL31_IN_DRAM || SEPARATE_NOBITS_REGION
/*
 * Put BL31 at the bottom of TZC secured DRAM
 */
#define BL31_BASE			ARM_AP_TZC_DRAM1_BASE
#define BL31_LIMIT			(ARM_AP_TZC_DRAM1_BASE +	\
						PLAT_ARM_MAX_BL31_SIZE)
/*
 * For SEPARATE_NOBITS_REGION, BL31 PROGBITS are loaded in TZC secured DRAM.
 * And BL31 NOBITS are loaded in Trusted SRAM such that BL2 is overwritten.
 */
#if SEPARATE_NOBITS_REGION
#define BL31_NOBITS_BASE		BL2_BASE
#define BL31_NOBITS_LIMIT		BL2_LIMIT
#endif /* SEPARATE_NOBITS_REGION */
#elif (RESET_TO_BL31)
/* Ensure Position Independent support (PIE) is enabled for this config.*/
# if !ENABLE_PIE
#  error "BL31 must be a PIE if RESET_TO_BL31=1."
#endif
/*
 * Since this is PIE, we can define BL31_BASE to 0x0 since this macro is solely
 * used for building BL31 and not used for loading BL31.
 */
#  define BL31_BASE			0x0
#  define BL31_LIMIT			PLAT_ARM_MAX_BL31_SIZE
#else
/* Put BL31 below BL2 in the Trusted SRAM.*/
#define BL31_BASE			((ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)\
						- PLAT_ARM_MAX_BL31_SIZE)
#define BL31_PROGBITS_LIMIT		BL2_BASE
/*
 * For BL2_AT_EL3 make sure the BL31 can grow up until BL2_BASE. This is
 * because in the BL2_AT_EL3 configuration, BL2 is always resident.
 */
#if BL2_AT_EL3
#define BL31_LIMIT			BL2_BASE
#else
#define BL31_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)
#endif
#endif

/******************************************************************************
 * RMM specific defines
 *****************************************************************************/
#if ENABLE_RME
#define RMM_BASE			(ARM_REALM_BASE)
#define RMM_LIMIT			(RMM_BASE + ARM_REALM_SIZE)
#define RMM_SHARED_BASE			(ARM_EL3_RMM_SHARED_BASE)
#define RMM_SHARED_SIZE			(ARM_EL3_RMM_SHARED_SIZE)
#endif

#if !defined(__aarch64__) || JUNO_AARCH32_EL3_RUNTIME
/*******************************************************************************
 * BL32 specific defines for EL3 runtime in AArch32 mode
 ******************************************************************************/
# if RESET_TO_SP_MIN && !JUNO_AARCH32_EL3_RUNTIME
/* Ensure Position Independent support (PIE) is enabled for this config.*/
# if !ENABLE_PIE
#  error "BL32 must be a PIE if RESET_TO_SP_MIN=1."
#endif
/*
 * Since this is PIE, we can define BL32_BASE to 0x0 since this macro is solely
 * used for building BL32 and not used for loading BL32.
 */
#  define BL32_BASE			0x0
#  define BL32_LIMIT			PLAT_ARM_MAX_BL32_SIZE
# else
/* Put BL32 below BL2 in the Trusted SRAM.*/
#  define BL32_BASE			((ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)\
						- PLAT_ARM_MAX_BL32_SIZE)
#  define BL32_PROGBITS_LIMIT		BL2_BASE
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
# if SPM_MM || SPMC_AT_EL3
#  define TSP_SEC_MEM_BASE		(ARM_AP_TZC_DRAM1_BASE + ULL(0x200000))
#  define TSP_SEC_MEM_SIZE		(ARM_AP_TZC_DRAM1_SIZE - ULL(0x200000))
#  define BL32_BASE			(ARM_AP_TZC_DRAM1_BASE + ULL(0x200000))
#  define BL32_LIMIT			(ARM_AP_TZC_DRAM1_BASE +	\
						ARM_AP_TZC_DRAM1_SIZE)
# elif defined(SPD_spmd)
#  define TSP_SEC_MEM_BASE		(ARM_AP_TZC_DRAM1_BASE + ULL(0x200000))
#  define TSP_SEC_MEM_SIZE		(ARM_AP_TZC_DRAM1_SIZE - ULL(0x200000))
#  define BL32_BASE			PLAT_ARM_SPMC_BASE
#  define BL32_LIMIT			(PLAT_ARM_SPMC_BASE +		\
						 PLAT_ARM_SPMC_SIZE)
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
#  define TSP_PROGBITS_LIMIT		BL31_BASE
#  define BL32_BASE			ARM_FW_CONFIGS_LIMIT
#  define BL32_LIMIT			BL31_BASE
# elif ARM_TSP_RAM_LOCATION_ID == ARM_TRUSTED_DRAM_ID
#  define TSP_SEC_MEM_BASE		PLAT_ARM_TRUSTED_DRAM_BASE
#  define TSP_SEC_MEM_SIZE		PLAT_ARM_TRUSTED_DRAM_SIZE
#  define BL32_BASE			PLAT_ARM_TRUSTED_DRAM_BASE
#  define BL32_LIMIT			(PLAT_ARM_TRUSTED_DRAM_BASE	\
						+ (UL(1) << 21))
# elif ARM_TSP_RAM_LOCATION_ID == ARM_DRAM_ID
#  define TSP_SEC_MEM_BASE		ARM_AP_TZC_DRAM1_BASE
#  define TSP_SEC_MEM_SIZE		ARM_AP_TZC_DRAM1_SIZE
#  define BL32_BASE			ARM_AP_TZC_DRAM1_BASE
#  define BL32_LIMIT			(ARM_AP_TZC_DRAM1_BASE +	\
						ARM_AP_TZC_DRAM1_SIZE)
# else
#  error "Unsupported ARM_TSP_RAM_LOCATION_ID value"
# endif
#endif /* !__aarch64__ || JUNO_AARCH32_EL3_RUNTIME */

/*
 * BL32 is mandatory in AArch32. In AArch64, undefine BL32_BASE if there is no
 * SPD and no SPM-MM and no SPMC-AT-EL3, as they are the only ones that can be
 * used as BL32.
 */
#if defined(__aarch64__) && !JUNO_AARCH32_EL3_RUNTIME
# if defined(SPD_none) && !SPM_MM && !SPMC_AT_EL3
#  undef BL32_BASE
# endif /* defined(SPD_none) && !SPM_MM || !SPMC_AT_EL3 */
#endif /* defined(__aarch64__) && !JUNO_AARCH32_EL3_RUNTIME */

/*******************************************************************************
 * FWU Images: NS_BL1U, BL2U & NS_BL2U defines.
 ******************************************************************************/
#define BL2U_BASE			BL2_BASE
#define BL2U_LIMIT			BL2_LIMIT

#define NS_BL2U_BASE			ARM_NS_DRAM1_BASE
#define NS_BL1U_BASE			(PLAT_ARM_NVM_BASE + UL(0x03EB8000))

/*
 * ID of the secure physical generic timer interrupt used by the TSP.
 */
#define TSP_IRQ_SEC_PHY_TIMER		ARM_IRQ_SEC_PHY_TIMER


/*
 * One cache line needed for bakery locks on ARM platforms
 */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE		(1 * CACHE_WRITEBACK_GRANULE)

/* Priority levels for ARM platforms */
#define PLAT_RAS_PRI			0x10
#define PLAT_SDEI_CRITICAL_PRI		0x60
#define PLAT_SDEI_NORMAL_PRI		0x70

/* ARM platforms use 3 upper bits of secure interrupt priority */
#define PLAT_PRI_BITS			3

/* SGI used for SDEI signalling */
#define ARM_SDEI_SGI			ARM_IRQ_SEC_SGI_0

#if SDEI_IN_FCONF
/* ARM SDEI dynamic private event max count */
#define ARM_SDEI_DP_EVENT_MAX_CNT	3

/* ARM SDEI dynamic shared event max count */
#define ARM_SDEI_DS_EVENT_MAX_CNT	3
#else
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
#endif /* SDEI_IN_FCONF */

#endif /* ARM_DEF_H */
