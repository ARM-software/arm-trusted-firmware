/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#define MHU_SECURE_BASE			ARM_SHARED_RAM_BASE
#define MHU_SECURE_SIZE			ARM_SHARED_RAM_SIZE
#define MHU_PAYLOAD_CACHED		0

#define TRUSTED_MAILBOXES_BASE		MHU_SECURE_BASE
#define TRUSTED_MAILBOX_SHIFT		4

#define NSROM_BASE			0x1f000000
#define NSROM_SIZE			0x00001000

/* Following covers CSS Peripherals excluding NSROM and NSRAM  */
#define CSS_DEVICE_BASE			0x20000000
#define CSS_DEVICE_SIZE			0x0e000000
#define MHU_BASE			0x2b1f0000

#define NSRAM_BASE			0x2e000000
#define NSRAM_SIZE			0x00008000

/* The slave_bootsecure controls access to GPU, DMC and CS. */
#define CSS_NIC400_SLAVE_BOOTSECURE	8

/* Interrupt handling constants */
#define CSS_IRQ_MHU			69
#define CSS_IRQ_GPU_SMMU_0		71
#define CSS_IRQ_GPU_SMMU_1		73
#define CSS_IRQ_ETR_SMMU		75
#define CSS_IRQ_TZC			80
#define CSS_IRQ_TZ_WDOG			86

/* SCP <=> AP boot configuration */
#define SCP_BOOT_CFG_ADDR		0x04000080
#define PRIMARY_CPU_SHIFT		8
#define PRIMARY_CPU_MASK		0xf


#define CSS_MAP_DEVICE			MAP_REGION_FLAT(		\
						CSS_DEVICE_BASE,	\
						CSS_DEVICE_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)


/*************************************************************************
 * Required platform porting definitions common to all
 * ARM Compute SubSystems (CSS)
 ************************************************************************/

/*
 * Load address of BL3-0 in CSS platform ports
 * BL3-0 is loaded to the same place as BL3-1.  Once BL3-0 is transferred to the
 * SCP, it is discarded and BL3-1 is loaded over the top.
 */
#define BL30_BASE			BL31_BASE

#define PLAT_ARM_SHARED_RAM_CACHED	MHU_PAYLOAD_CACHED

/* Load address of Non-Secure Image for CSS platform ports */
#define PLAT_ARM_NS_IMAGE_OFFSET	0xE0000000

/* TZC related constants */
#define PLAT_ARM_TZC_FILTERS		REG_ATTR_FILTER_BIT_ALL


#endif /* __CSS_DEF_H__ */
