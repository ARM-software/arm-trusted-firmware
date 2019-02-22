/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
/* Certain ARM files require defines from this file */
#include <common/tbbr/tbbr_img_def.h>
/* Soc specific defines */
#include <soc.h>
/* include the platform-level security policy */
#include <policy.h>

#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

/* Special value used to verify platform parameters from BL2 to BL31 */

#define LS_BL31_PLAT_PARAM_VAL		0x0f1e2d3c4b5a6978ULL

/******************************************************************************
 *  Board specific defines
 *****************************************************************************/

#define NXP_SYSCLK_FREQ		100000000
#define NXP_DDRCLK_FREQ		100000000

/* UART related definition */
#define NXP_CONSOLE_ADDR	NXP_UART_ADDR
#define NXP_CONSOLE_BAUDRATE	115200

#define NXP_SPD_EEPROM0		0x51

#define DDRC_NUM_DIMM		1
#define CONFIG_DDR_ECC_EN

#define PLAT_DEF_DRAM0_SIZE	0x80000000	/*  2G */

/* Board specific - size of QSPI Flash on board */
#if QSPI_BOOT
#define NXP_QSPI_FLASH_SIZE	0x20000000
#endif

#if NOR_BOOT
#define NXP_NOR_FLASH_SIZE	0x20000000
#endif


/******************************************************************************
 * Required platform porting definitions common to all ARM standard platforms
 *****************************************************************************/

/* Size of cacheable stacks */
#if defined(IMAGE_BL2)
#define PLATFORM_STACK_SIZE	0x1000
#elif defined(IMAGE_BL31)
#define PLATFORM_STACK_SIZE	0x1000
#endif

#define FIRMWARE_WELCOME_STR_LS_BL2	"Welcome to LS1043 BL2 Phase\n"
#define FIRMWARE_WELCOME_STR_LS_BL31	"Welcome to LS1043 BL31 Phase\n"

/* This is common for all platforms where
 * 64K is reserved for Secure memory
 */
/* 64M Secure Memory */
#define NXP_SECURE_DRAM_SIZE	(64 * 1024 * 1024)

/* 2M Secure EL1 Payload Shared Memory */
#define NXP_SP_SHRD_DRAM_SIZE	(2 * 1024 * 1024)

/* Check DRAM1 code at run time that it is 2 GB at run time */
/* Non secure memory */
#define NXP_NS_DRAM_SIZE	(NXP_DRAM0_SIZE - \
				(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE))

#define NXP_NS_DRAM_ADDR	NXP_DRAM0_ADDR

#ifdef TEST_BL31
#define NXP_SECURE_DRAM_ADDR 0
#else
#define NXP_SECURE_DRAM_ADDR	(NXP_NS_DRAM_ADDR + NXP_DRAM0_SIZE - \
				(NXP_SECURE_DRAM_SIZE  + NXP_SP_SHRD_DRAM_SIZE))
#endif

#define NXP_SP_SHRD_DRAM_ADDR	(NXP_NS_DRAM_ADDR + NXP_DRAM0_SIZE \
				- NXP_SP_SHRD_DRAM_SIZE)

#define BL2_BASE		NXP_OCRAM_ADDR
#define BL2_LIMIT		(NXP_OCRAM_ADDR + NXP_OCRAM_SIZE)
#define BL2_TEXT_LIMIT		(BL2_LIMIT - NXP_ROM_RSVD - CSF_HDR_SZ)

/* 2 MB reserved in secure memory for DDR */
#define BL31_BASE		NXP_SECURE_DRAM_ADDR
#define BL31_SIZE		(0x200000)
#define BL31_LIMIT		(BL31_BASE + BL31_SIZE)

/* Put BL32 in secure memory */
#define BL32_BASE		(NXP_SECURE_DRAM_ADDR + BL31_SIZE)
#define BL32_LIMIT		(NXP_SECURE_DRAM_ADDR + \
				NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE)

/* BL33 memory region */
/* Hardcoded based on current address in u-boot */
#define BL33_BASE		0x82000000
#define BL33_LIMIT		(NXP_NS_DRAM_ADDR + NXP_NS_DRAM_SIZE)

/* SD block buffer */
#define NXP_SD_BLOCK_BUF_ADDR	ULL(0x80000000)
#define NXP_SD_BLOCK_BUF_SIZE	0x00100000

/*
 * ID of the secure physical generic timer interrupt used by the BL32.
 */
#define BL32_IRQ_SEC_PHY_TIMER	29

/* IO defines as needed by IO driver framework */
#define MAX_IO_DEVICES		3
#define MAX_IO_BLOCK_DEVICES	1
#define MAX_IO_HANDLES		4

/*
 * FIP image defines - Offset at which FIP Image would be present
 * Image would include Bl31 , Bl33 and Bl32 (optional)
 */
#ifdef POLICY_FUSE_PROVISION
#define MAX_FIP_DEVICES		2
#define FUSE_BUF		ULL(0x81000000)
#define FUSE_SZ			0x80000
#endif

#define PLAT_FIP_OFFSET		0x100000
#define PLAT_FIP_MAX_SIZE	0x400000

/* Check if this size can be determined from array size */
#if defined(IMAGE_BL2)
#define MAX_MMAP_REGIONS	8
#define MAX_XLAT_TABLES		6
#elif defined(IMAGE_BL31)
#define MAX_MMAP_REGIONS	9
#define MAX_XLAT_TABLES		9
#elif defined(IMAGE_BL32)
#define MAX_MMAP_REGIONS	8
#define MAX_XLAT_TABLES		9
#endif

/******************************************************************************/
// GIC Related defines
/******************************************************************************/
/*
 * The number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#define LS_IRQ_SEC_SGI_0		8
#define LS_IRQ_SEC_SGI_1		9
#define LS_IRQ_SEC_SGI_2		10
#define LS_IRQ_SEC_SGI_3		11
#define LS_IRQ_SEC_SGI_4		12
#define LS_IRQ_SEC_SGI_5		13
#define LS_IRQ_SEC_SGI_6		14
#define LS_IRQ_SEC_SGI_7		15

/*
 * Define properties of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(BL32_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(LS_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(LS_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(LS_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(LS_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(LS_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(LS_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(LS_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(LS_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

#define PLAT_G0_IRQ_PROPS(grp)

/******************************************************************************/

#endif /* __PLATFORM_DEF_H__ */
