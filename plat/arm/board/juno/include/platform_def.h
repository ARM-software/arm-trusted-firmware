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

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arm_def.h>
#include <board_arm_def.h>
#include <board_css_def.h>
#include <common_def.h>
#include <css_def.h>
#include <soc_css_def.h>
#include <tzc400.h>
#include <v2m_def.h>
#include "../juno_def.h"

/* Juno supports system power domain */
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL2
#define PLAT_NUM_PWR_DOMAINS		(ARM_SYSTEM_COUNT + \
					ARM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)
/*
 * Other platform porting definitions are provided by included headers
 */

/*
 * Required ARM standard platform porting definitions
 */
#define PLAT_ARM_CLUSTER0_CORE_COUNT	2
#define PLAT_ARM_CLUSTER1_CORE_COUNT	4

/* Use the bypass address */
#define PLAT_ARM_TRUSTED_ROM_BASE	V2M_FLASH0_BASE + BL1_ROM_BYPASS_OFFSET

/*
 * Actual ROM size on Juno is 64 KB, but TBB currently requires at least 80 KB
 * in debug mode. We can test TBB on Juno bypassing the ROM and using 128 KB of
 * flash
 */
#if TRUSTED_BOARD_BOOT
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x00020000
#else
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x00010000
#endif /* TRUSTED_BOARD_BOOT */


/* CCI related constants */
#define PLAT_ARM_CCI_BASE		0x2c090000
#define PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX	4
#define PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX	3

/* TZC related constants */
#define PLAT_ARM_TZC_NS_DEV_ACCESS	(				\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CCI400)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_PCIE)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD0)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD1)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_USB)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_DMA330)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_THINLINKS)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_AP)		|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_GPU)	|	\
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CORESIGHT))

/*
 * Required ARM CSS based platform porting definitions
 */

/* GIC related constants (no GICR in GIC-400) */
#define PLAT_ARM_GICD_BASE		0x2c010000
#define PLAT_ARM_GICC_BASE		0x2c02f000
#define PLAT_ARM_GICH_BASE		0x2c04f000
#define PLAT_ARM_GICV_BASE		0x2c06f000

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_ARM_G1S_IRQS		CSS_G1S_IRQS,			\
					ARM_G1S_IRQS,			\
					JUNO_IRQ_DMA_SMMU,		\
					JUNO_IRQ_HDLCD0_SMMU,		\
					JUNO_IRQ_HDLCD1_SMMU,		\
					JUNO_IRQ_USB_SMMU,		\
					JUNO_IRQ_THIN_LINKS_SMMU,	\
					JUNO_IRQ_SEC_I2C,		\
					JUNO_IRQ_GPU_SMMU_1,		\
					JUNO_IRQ_ETR_SMMU

#define PLAT_ARM_G0_IRQS		ARM_G0_IRQS

/*
 * Required ARM CSS SoC based platform porting definitions
 */

/* CSS SoC NIC-400 Global Programmers View (GPV) */
#define PLAT_SOC_CSS_NIC400_BASE	0x2a000000


#endif /* __PLATFORM_DEF_H__ */
