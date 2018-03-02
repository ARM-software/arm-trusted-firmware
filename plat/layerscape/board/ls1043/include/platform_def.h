/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <ls_def.h>
#include <common_def.h>
#include <tzc400.h>
#include <utils.h>
#include "../ls1043_def.h"

#define FIRMWARE_WELCOME_STR_LS1043	"Welcome to LS1043 BL1 Phase\n"
#define FIRMWARE_WELCOME_STR_LS1043_BL2	"Welcome to LS1043 BL2 Phase\n"
#define FIRMWARE_WELCOME_STR_LS1043_BL31 "Welcome to LS1043 BL31 Phase\n"
#define FIRMWARE_WELCOME_STR_LS1043_BL32 "Welcome to LS1043 BL32 Phase, TSP\n"

#define HH_DEBUG
#define BL31_DEBUG

/* Required platform porting definitions */
#define PLAT_PRIMARY_CPU  0x0
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1
#define PLATFORM_CORE_COUNT		4
#define COUNTER_FREQUENCY              25000000        /* 25MHz */

/*
 * Required LS standard platform porting definitions
 */
#define PLAT_LS_CLUSTER_COUNT		1
#define PLAT_LS1043_CCI_CLUSTER0_SL_IFACE_IX 4
#define LS1043_CLUSTER_COUNT 1
#define LS1043_MAX_CPUS_PER_CLUSTER 4

#define LS_DRAM1_BASE		0x80000000
#define LS_DRAM2_BASE		0x880000000
#define LS_DRAM2_SIZE		0x780000000	//30G
#define LS_DRAM1_SIZE		0x80000000	//2G
#define LS_NS_DRAM_BASE		LS_DRAM1_BASE
/* 64M Secure Memory, in fact there a 2M non-secure hole on top of it */
#define LS_SECURE_DRAM_SIZE	(64 * 1024 * 1024)
#define LS_SECURE_DRAM_BASE	(LS_NS_DRAM_BASE + LS_DRAM1_SIZE - \
					LS_SECURE_DRAM_SIZE)
#define LS_NS_DRAM_SIZE		(LS_DRAM1_SIZE - LS_SECURE_DRAM_SIZE)

/*
 * By default, BL2 is in DDR memory.
 * If LS_BL2_IN_OCRAM is defined, BL2 will in OCRAM
 */
//#define LS_BL2_IN_OCRAM

#ifndef LS_BL2_IN_OCRAM
/*
 * on top of SECURE memory is 2M non-secure hole for OPTee,
 *  1M secure memory below this hole will be used for BL2.
 */
#define LS_BL2_DDR_BASE		(LS_SECURE_DRAM_BASE + LS_SECURE_DRAM_SIZE \
					- 3 * 1024 * 1024)
#endif

#define PLAT_LS_CCSR_BASE 0x1000000
#define PLAT_LS_CCSR_SIZE 0xF000000

/* Flash base address, currently ROM is not used for ATF images on LS platforms */
#define PLAT_LS_TRUSTED_ROM_BASE	0x60100000
#define PLAT_LS_TRUSTED_ROM_SIZE	0x20000000	//Flash size
#define PLAT_LS_FLASH_SIZE			0x20000000
#define PLAT_LS_FLASH_BASE			0x60000000

#define LS_SRAM_BASE	0x10000000
#define LS_SRAM_LIMIT	0x10020000   //128K
#define LS_SRAM_SHARED_SIZE	0x1000 //4K
#define LS_SRAM_SIZE    (LS_SRAM_LIMIT - LS_SRAM_BASE)
#define LS_BL_RAM_BASE    (LS_SRAM_BASE + LS_SRAM_SHARED_SIZE)

#define PLAT_LS_FIP_MAX_SIZE 0x4000000

/* Memory Layout */

#define BL1_RO_BASE			PLAT_LS_TRUSTED_ROM_BASE
#define BL1_RO_LIMIT			(PLAT_LS_TRUSTED_ROM_BASE	\
					 + PLAT_LS_TRUSTED_ROM_SIZE)
#define PLAT_LS_FIP_BASE		0x60120000

#ifdef LS_BL2_IN_OCRAM
/* BL2 is in OCRAM */
#define PLAT_LS_MAX_BL1_RW_SIZE		(52 * 1024)		/* 52K */
#define PLAT_LS_MAX_BL31_SIZE		(64 * 1024)		/* 64K */
#define PLAT_LS_MAX_BL2_SIZE		(44 * 1024)		/* 44K */
/* Reserve memory in OCRAM for BL31 Text and ROData segment */
#define BL31_TEXT_RODATA_SIZE		(32 * 1024)		/* 32K */
#else /* LS_BL2_IN_OCRAM */
/* BL2 in DDR */
#define PLAT_LS_MAX_BL1_RW_SIZE		(64 * 1024)		/* 64K */
#define PLAT_LS_MAX_BL31_SIZE		(64 * 1024)		/* 64K */
#define PLAT_LS_MAX_BL2_SIZE		(1 * 1024 * 1024)	/* 1M */
#endif /* LS_BL2_IN_OCRAM */
/*
 * Put BL31 at the start of OCRAM.
 */
#define BL31_BASE			LS_SRAM_BASE
#define BL31_LIMIT			(LS_SRAM_BASE + PLAT_LS_MAX_BL31_SIZE)

#ifdef LS_BL2_IN_OCRAM
/*
 * BL2 follow BL31 Text and ROData region.
 */
#define BL2_BASE			(BL31_BASE + BL31_TEXT_RODATA_SIZE)
#define BL2_LIMIT			(BL2_BASE + PLAT_LS_MAX_BL2_SIZE)

#else
/*
 * BL2 in DDR memory.
 */
#define BL2_BASE			LS_BL2_DDR_BASE
#define BL2_LIMIT			(BL2_BASE + PLAT_LS_MAX_BL2_SIZE)

#endif

/*
 * Put BL1 RW at the top of the Trusted SRAM.
 */
#ifdef LS_BL2_IN_OCRAM
#define BL1_RW_BASE			BL2_LIMIT
#else
#define BL1_RW_BASE			BL31_LIMIT
#endif
#define BL1_RW_LIMIT			LS_SRAM_LIMIT

/* Put BL32 in secure memory */
#define BL32_BASE		LS_SECURE_DRAM_BASE
#define BL32_LIMIT		(LS_SECURE_DRAM_BASE + LS_SECURE_DRAM_SIZE)
/* BL33 memory region */
#define BL33_BASE		0x82000000
#define BL33_LIMIT		(LS_NS_DRAM_BASE + LS_NS_DRAM_SIZE)

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
/*
 * On ARM standard platforms, the TSP can execute from Trusted SRAM,
 * Trusted DRAM (if available) or the DRAM region secured by the TrustZone
 * controller.
 */

#define TSP_SEC_MEM_BASE  BL32_BASE
#define TSP_SEC_MEM_SIZE  (BL32_LIMIT - BL32_BASE)

/*
 * ID of the secure physical generic timer interrupt used by the TSP.
 */
#define TSP_IRQ_SEC_PHY_TIMER 29


/*
 * GIC related constants
 */
#define PLAT_LS1043_CCI_BASE 0x01180000
#define GICD_BASE       0x01401000
#define GICC_BASE       0x01402000
#define GICD_BASE_64K           0x01410000
#define GICC_BASE_64K           0x01420000

#define DCFG_CCSR_SVR           0x1ee00a4
#define REV1_0                  0x10
#define REV1_1                  0x11
#define GIC_ADDR_BIT            31
#define SCFG_GIC400_ALIGN       0x1570188

/* TZASC related constants */
#define TZASC_CONFIGURATION_REG     0x000
#define TZASC_SECURITY_INV_REG      0x034
#define TZASC_SECURITY_INV_EN       0x1
#define TZASC_REGIONS_REG       0x100
/* As region address should address atleast 32KB memory. */
#define TZASC_REGION_LOWADDR_MASK   0xFFFF8000
#define TZASC_REGION_LOWADDR_OFFSET 0x0
#define TZASC_REGION_HIGHADDR_OFFSET    0x4
#define TZASC_REGION_ATTR_OFFSET    0x8
#define TZASC_REGION_ENABLED        1
#define TZASC_REGION_DISABLED       0
#define TZASC_REGION_SIZE_32KB      0xE
#define TZASC_REGION_SIZE_64KB      0xF
#define TZASC_REGION_SIZE_128KB     0x10
#define TZASC_REGION_SIZE_256KB     0x11
#define TZASC_REGION_SIZE_512KB     0x12
#define TZASC_REGION_SIZE_1MB       0x13
#define TZASC_REGION_SIZE_2MB       0x14
#define TZASC_REGION_SIZE_4MB       0x15
#define TZASC_REGION_SIZE_8MB       0x16
#define TZASC_REGION_SIZE_16MB      0x17
#define TZASC_REGION_SIZE_32MB      0x18
#define TZASC_REGION_SIZE_64MB      0x19
#define TZASC_REGION_SIZE_128MB     0x1A
#define TZASC_REGION_SIZE_256MB     0x1B
#define TZASC_REGION_SIZE_512MB     0x1C
#define TZASC_REGION_SIZE_1GB       0x1D
#define TZASC_REGION_SIZE_2GB       0x1E
#define TZASC_REGION_SIZE_4GB       0x1F
#define TZASC_REGION_SIZE_8GB       0x20
#define TZASC_REGION_SIZE_16GB      0x21
#define TZASC_REGION_SIZE_32GB      0x22
#define TZASC_REGION_SECURITY_SR    (1 << 3)
#define TZASC_REGION_SECURITY_SW    (1 << 2)
#define TZASC_REGION_SECURITY_SRW   (TZASC_REGION_SECURITY_SR| \
					TZASC_REGION_SECURITY_SW)
#define TZASC_REGION_SECURITY_NSR   (1 << 1)
#define TZASC_REGION_SECURITY_NSW   1
#define TZASC_REGION_SECURITY_NSRW  (TZASC_REGION_SECURITY_NSR| \
					TZASC_REGION_SECURITY_NSW)

#define CSU_SEC_ACCESS_REG_OFFSET       0x21C
#define TZASC_BYPASS_MUX_DISABLE        0x4
#define CCI_TERMINATE_BARRIER_TX        0x8
#define CONFIG_SYS_FSL_TZASC_ADDR       0x1500000

/* UART related definition */

#define PLAT_LS1043_DUART1_BASE	0x021c0000
#define PLAT_LS1043_DUART2_BASE	0x021d0000
#define PLAT_LS1043_DUART_SIZE	0x10000

#define PLAT_LS1043_UART_BASE  0x21c0500
#define PLAT_LS1043_UART2_BASE 0x21c0600
#define PLAT_LS1043_UART_CLOCK 400000000
#define PLAT_LS1043_UART_BAUDRATE  115200
/* Define UART to be used by ATF log */
#define LS_ATF_UART_BASE	PLAT_LS1043_UART_BASE
#define LS_ATF_UART_CLOCK	PLAT_LS1043_UART_CLOCK
#define LS_ATF_UART_BAUDRATE	PLAT_LS1043_UART_BAUDRATE

#define LS1043_SYS_CNTCTL_BASE 0x2B00000

#define CONFIG_SYS_IMMR                         0x01000000
#define CONFIG_SYS_FSL_CSU_ADDR			(CONFIG_SYS_IMMR + 0x00510000)

/* Size of cacheable stacks */
#if defined(IMAGE_BL1)
#define PLATFORM_STACK_SIZE 0x440
#define MAX_MMAP_REGIONS	6
#define MAX_XLAT_TABLES     4
#elif defined(IMAGE_BL2)
#define PLATFORM_STACK_SIZE 0x400
#define MAX_MMAP_REGIONS    8
#define MAX_XLAT_TABLES     6
#elif defined(IMAGE_BL31)
#define PLATFORM_STACK_SIZE 0x400
#define MAX_MMAP_REGIONS    8
#define MAX_XLAT_TABLES     4
#elif defined(IMAGE_BL32)
#define PLATFORM_STACK_SIZE 0x440
#define MAX_MMAP_REGIONS    8
#define MAX_XLAT_TABLES     9
#endif

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

#endif /* __PLATFORM_DEF_H__ */
