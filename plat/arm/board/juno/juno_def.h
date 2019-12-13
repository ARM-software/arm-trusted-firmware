/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_DEF_H
#define JUNO_DEF_H

#include <lib/utils_def.h>

/*******************************************************************************
 * Juno memory map related constants
 ******************************************************************************/

/* Board revisions */
#define REV_JUNO_R0			U(0x1)	/* Rev B */
#define REV_JUNO_R1			U(0x2)	/* Rev C */
#define REV_JUNO_R2			U(0x3)	/* Rev D */

/* Bypass offset from start of NOR flash */
#define BL1_ROM_BYPASS_OFFSET		UL(0x03EC0000)

#define EMMC_BASE			UL(0x0c000000)
#define EMMC_SIZE			UL(0x04000000)

#define PSRAM_BASE			UL(0x14000000)
#define PSRAM_SIZE			UL(0x02000000)

#define JUNO_SSC_VER_PART_NUM		U(0x030)

/*******************************************************************************
 * Juno topology related constants
 ******************************************************************************/
#define JUNO_CLUSTER_COUNT		U(2)
#define JUNO_CLUSTER0_CORE_COUNT	U(2)
#define JUNO_CLUSTER1_CORE_COUNT	U(4)

/*******************************************************************************
 * TZC-400 related constants
 ******************************************************************************/
#define TZC400_NSAID_CCI400		0  /* Note: Same as default NSAID!! */
#define TZC400_NSAID_PCIE		1
#define TZC400_NSAID_HDLCD0		2
#define TZC400_NSAID_HDLCD1		3
#define TZC400_NSAID_USB		4
#define TZC400_NSAID_DMA330		5
#define TZC400_NSAID_THINLINKS		6
#define TZC400_NSAID_AP			9
#define TZC400_NSAID_GPU		10
#define TZC400_NSAID_SCP		11
#define TZC400_NSAID_CORESIGHT		12

/*******************************************************************************
 * TRNG related constants
 ******************************************************************************/
#define TRNG_BASE	UL(0x7FE60000)
#define TRNG_NOUTPUTS	4
#define TRNG_STATUS	UL(0x10)
#define TRNG_INTMASK	UL(0x14)
#define TRNG_CONFIG	UL(0x18)
#define TRNG_CONTROL	UL(0x1C)
#define TRNG_NBYTES	16	/* Number of bytes generated per round. */

/*******************************************************************************
 * MMU-401 related constants
 ******************************************************************************/
#define MMU401_SSD_OFFSET		UL(0x4000)
#define MMU401_DMA330_BASE		UL(0x7fb00000)

/*******************************************************************************
 * Interrupt handling constants
 ******************************************************************************/
#define JUNO_IRQ_DMA_SMMU		126
#define JUNO_IRQ_HDLCD0_SMMU		128
#define JUNO_IRQ_HDLCD1_SMMU		130
#define JUNO_IRQ_USB_SMMU		132
#define JUNO_IRQ_THIN_LINKS_SMMU	134
#define JUNO_IRQ_SEC_I2C		137
#define JUNO_IRQ_GPU_SMMU_1		73
#define JUNO_IRQ_ETR_SMMU		75

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

#endif /* JUNO_DEF_H */
