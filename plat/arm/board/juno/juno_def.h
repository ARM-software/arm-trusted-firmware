/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __JUNO_DEF_H__
#define __JUNO_DEF_H__


/*******************************************************************************
 * Juno memory map related constants
 ******************************************************************************/

/* Board revisions */
#define REV_JUNO_R0			0x1	/* Rev B */
#define REV_JUNO_R1			0x2	/* Rev C */
#define REV_JUNO_R2			0x3	/* Rev D */

/* Bypass offset from start of NOR flash */
#define BL1_ROM_BYPASS_OFFSET		0x03EC0000

#define EMMC_BASE			0x0c000000
#define EMMC_SIZE			0x04000000

#define PSRAM_BASE			0x14000000
#define PSRAM_SIZE			0x02000000

#define JUNO_SSC_VER_PART_NUM		0x030

/*******************************************************************************
 * Juno topology related constants
 ******************************************************************************/
#define JUNO_CLUSTER_COUNT		2
#define JUNO_CLUSTER0_CORE_COUNT	2
#define JUNO_CLUSTER1_CORE_COUNT	4

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
#define TRNG_BASE	0x7FE60000ULL
#define TRNG_NOUTPUTS	4
#define TRNG_STATUS	0x10
#define TRNG_INTMASK	0x14
#define TRNG_CONFIG	0x18
#define TRNG_CONTROL	0x1C
#define TRNG_NBYTES	16	/* Number of bytes generated per round. */

/*******************************************************************************
 * MMU-401 related constants
 ******************************************************************************/
#define MMU401_SSD_OFFSET		0x4000
#define MMU401_DMA330_BASE		0x7fb00000

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

#endif /* __JUNO_DEF_H__ */
