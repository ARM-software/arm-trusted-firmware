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

#ifndef __CSS_DEF_H__
#define __CSS_DEF_H__

#include <arm_def.h>
#include <tzc400.h>

/*************************************************************************
 * Definitions common to all ARM Compute SubSystems (CSS)
 *************************************************************************/
#define NSROM_BASE			0x1f000000
#define NSROM_SIZE			0x00001000

/* Following covers CSS Peripherals excluding NSROM and NSRAM  */
#define CSS_DEVICE_BASE			0x20000000
#define CSS_DEVICE_SIZE			0x0e000000

#define NSRAM_BASE			0x2e000000
#define NSRAM_SIZE			0x00008000

/* System Security Control Registers */
#define SSC_REG_BASE			0x2a420000
#define SSC_GPRETN			(SSC_REG_BASE + 0x030)

/* The slave_bootsecure controls access to GPU, DMC and CS. */
#define CSS_NIC400_SLAVE_BOOTSECURE	8

/* Interrupt handling constants */
#define CSS_IRQ_MHU			69
#define CSS_IRQ_GPU_SMMU_0		71
#define CSS_IRQ_TZC			80
#define CSS_IRQ_TZ_WDOG			86
#define CSS_IRQ_SEC_SYS_TIMER		91

/*
 * Define a list of Group 1 Secure interrupts as per GICv3 terminology. On a
 * GICv2 system or mode, the interrupts will be treated as Group 0 interrupts.
 */
#define CSS_G1S_IRQS			CSS_IRQ_MHU,		\
					CSS_IRQ_GPU_SMMU_0,	\
					CSS_IRQ_TZC,		\
					CSS_IRQ_TZ_WDOG,	\
					CSS_IRQ_SEC_SYS_TIMER

/*
 * SCP <=> AP boot configuration
 *
 * The SCP/AP boot configuration is a 32-bit word located at a known offset from
 * the start of the Trusted SRAM.
 *
 * Note that the value stored at this address is only valid at boot time, before
 * the SCP_BL2 image is transferred to SCP.
 */
#define SCP_BOOT_CFG_ADDR		PLAT_CSS_SCP_COM_SHARED_MEM_BASE

#define CSS_MAP_DEVICE			MAP_REGION_FLAT(		\
						CSS_DEVICE_BASE,	\
						CSS_DEVICE_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

/* Platform ID address */
#define SSC_VERSION_OFFSET			0x040

#define SSC_VERSION_CONFIG_SHIFT		28
#define SSC_VERSION_MAJOR_REV_SHIFT		24
#define SSC_VERSION_MINOR_REV_SHIFT		20
#define SSC_VERSION_DESIGNER_ID_SHIFT		12
#define SSC_VERSION_PART_NUM_SHIFT		0x0
#define SSC_VERSION_CONFIG_MASK			0xf
#define SSC_VERSION_MAJOR_REV_MASK		0xf
#define SSC_VERSION_MINOR_REV_MASK		0xf
#define SSC_VERSION_DESIGNER_ID_MASK		0xff
#define SSC_VERSION_PART_NUM_MASK		0xfff

#ifndef __ASSEMBLY__

/* SSC_VERSION related accessors */

/* Returns the part number of the platform */
#define GET_SSC_VERSION_PART_NUM(val)				\
		(((val) >> SSC_VERSION_PART_NUM_SHIFT) &	\
		SSC_VERSION_PART_NUM_MASK)

/* Returns the configuration number of the platform */
#define GET_SSC_VERSION_CONFIG(val)				\
		(((val) >> SSC_VERSION_CONFIG_SHIFT) &		\
		SSC_VERSION_CONFIG_MASK)

#endif /* __ASSEMBLY__ */

/*************************************************************************
 * Required platform porting definitions common to all
 * ARM Compute SubSystems (CSS)
 ************************************************************************/

/*
 * The loading of SCP images(SCP_BL2 or SCP_BL2U) is done if there
 * respective base addresses are defined (i.e SCP_BL2_BASE, SCP_BL2U_BASE).
 * Hence, `CSS_LOAD_SCP_IMAGES` needs to be set to 1 if BL2 needs to load
 * an SCP_BL2/SCP_BL2U image.
 */
#if CSS_LOAD_SCP_IMAGES
/*
 * Load address of SCP_BL2 in CSS platform ports
 * SCP_BL2 is loaded to the same place as BL31.  Once SCP_BL2 is transferred to the
 * SCP, it is discarded and BL31 is loaded over the top.
 */
#define SCP_BL2_BASE			BL31_BASE

#define SCP_BL2U_BASE			BL31_BASE
#endif /* CSS_LOAD_SCP_IMAGES */

/* Load address of Non-Secure Image for CSS platform ports */
#define PLAT_ARM_NS_IMAGE_OFFSET	0xE0000000

/* TZC related constants */
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT_ALL

/* Trusted mailbox base address common to all CSS */
#define PLAT_ARM_TRUSTED_MAILBOX_BASE	ARM_TRUSTED_SRAM_BASE

/*
 * Parsing of CPU and Cluster states, as returned by 'Get CSS Power State' SCP
 * command
 */
#define CSS_CLUSTER_PWR_STATE_ON	0
#define CSS_CLUSTER_PWR_STATE_OFF	3

#define CSS_CPU_PWR_STATE_ON		1
#define CSS_CPU_PWR_STATE_OFF		0
#define CSS_CPU_PWR_STATE(state, n)	(((state) >> (n)) & 1)

#endif /* __CSS_DEF_H__ */
