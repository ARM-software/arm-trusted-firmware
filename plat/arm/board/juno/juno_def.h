/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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
