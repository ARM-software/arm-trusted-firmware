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
#define MHU_PAYLOAD_CACHED		0

#define NSROM_BASE			0x1f000000
#define NSROM_SIZE			0x00001000

/* Following covers CSS Peripherals excluding NSROM and NSRAM  */
#define CSS_DEVICE_BASE			0x20000000
#define CSS_DEVICE_SIZE			0x0e000000
#define MHU_BASE			0x2b1f0000

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
 * the start of the Trusted SRAM. Part of this configuration is which CPU is the
 * primary, according to the shift and mask definitions below.
 *
 * Note that the value stored at this address is only valid at boot time, before
 * the SCP_BL2 image is transferred to SCP.
 */
#define SCP_BOOT_CFG_ADDR		(ARM_TRUSTED_SRAM_BASE + 0x80)
#define PRIMARY_CPU_SHIFT		8
#define PRIMARY_CPU_BIT_WIDTH		4

/*
 * Base address of the first memory region used for communication between AP
 * and SCP. Used by the BOM and SCPI protocols.
 *
 * Note that this is located at the same address as SCP_BOOT_CFG_ADDR, which
 * means the SCP/AP configuration data gets overwritten when the AP initiates
 * communication with the SCP.
 */
#define SCP_COM_SHARED_MEM_BASE		(ARM_TRUSTED_SRAM_BASE + 0x80)

#define CSS_MAP_DEVICE			MAP_REGION_FLAT(		\
						CSS_DEVICE_BASE,	\
						CSS_DEVICE_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)


/*************************************************************************
 * Required platform porting definitions common to all
 * ARM Compute SubSystems (CSS)
 ************************************************************************/

/*
 * Load address of SCP_BL2 in CSS platform ports
 * SCP_BL2 is loaded to the same place as BL31.  Once SCP_BL2 is transferred to the
 * SCP, it is discarded and BL31 is loaded over the top.
 */
#define SCP_BL2_BASE			BL31_BASE

#define SCP_BL2U_BASE			BL31_BASE

#define PLAT_ARM_SHARED_RAM_CACHED	MHU_PAYLOAD_CACHED

/* Load address of Non-Secure Image for CSS platform ports */
#define PLAT_ARM_NS_IMAGE_OFFSET	0xE0000000

/* TZC related constants */
#define PLAT_ARM_TZC_FILTERS		REG_ATTR_FILTER_BIT_ALL
#define PLAT_ARM_TZC_BASE		0x2a4a0000

/* System timer related constants */
#define PLAT_ARM_NSTIMER_FRAME_ID	1

/* Trusted mailbox base address common to all CSS */
#define PLAT_ARM_TRUSTED_MAILBOX_BASE	ARM_TRUSTED_SRAM_BASE


#endif /* __CSS_DEF_H__ */
