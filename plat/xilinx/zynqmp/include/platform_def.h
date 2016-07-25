/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include "../zynqmp_def.h"

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE 0x440

#define PLATFORM_CORE_COUNT		4
#define PLAT_NUM_POWER_DOMAINS		5
#define PLAT_MAX_PWR_LVL		1
#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		2

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL31 debug size plus a
 * little space for growth.
 */
#ifndef ZYNQMP_ATF_MEM_BASE
# define BL31_BASE			0xfffea000
# define BL31_LIMIT			0xffffffff
#else
# define BL31_BASE			(ZYNQMP_ATF_MEM_BASE)
# define BL31_LIMIT			(ZYNQMP_ATF_MEM_BASE + ZYNQMP_ATF_MEM_SIZE - 1)
# ifdef ZYNQMP_ATF_MEM_PROGBITS_SIZE
#  define BL31_PROGBITS_LIMIT		(ZYNQMP_ATF_MEM_BASE + ZYNQMP_ATF_MEM_PROGBITS_SIZE - 1)
# endif
#endif

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
#ifndef ZYNQMP_BL32_MEM_BASE
# define BL32_BASE			0x60000000
# define BL32_LIMIT			0x7fffffff
#else
# define BL32_BASE			(ZYNQMP_BL32_MEM_BASE)
# define BL32_LIMIT			(ZYNQMP_BL32_MEM_BASE + ZYNQMP_BL32_MEM_SIZE - 1)
#endif

/*******************************************************************************
 * BL33 specific defines.
 ******************************************************************************/
#ifndef PRELOADED_BL33_BASE
# define PLAT_ARM_NS_IMAGE_OFFSET	0x8000000
#else
# define PLAT_ARM_NS_IMAGE_OFFSET	PRELOADED_BL33_BASE
#endif

/*******************************************************************************
 * TSP  specific defines.
 ******************************************************************************/
#define TSP_SEC_MEM_BASE		BL32_BASE
#define TSP_SEC_MEM_SIZE		(BL32_LIMIT - BL32_BASE + 1)

/* ID of the secure physical generic timer interrupt used by the TSP */
#define TSP_IRQ_SEC_PHY_TIMER		ARM_IRQ_SEC_PHY_TIMER

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define ADDR_SPACE_SIZE			(1ull << 32)
#define MAX_MMAP_REGIONS		7
#define MAX_XLAT_TABLES			5

#define CACHE_WRITEBACK_SHIFT   6
#define CACHE_WRITEBACK_GRANULE (1 << CACHE_WRITEBACK_SHIFT)

#define PLAT_ARM_GICD_BASE	BASE_GICD_BASE
#define PLAT_ARM_GICC_BASE	BASE_GICC_BASE
/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_ARM_G1S_IRQS	ARM_IRQ_SEC_PHY_TIMER,	\
				IRQ_SEC_IPI_APU,	\
				ARM_IRQ_SEC_SGI_0,	\
				ARM_IRQ_SEC_SGI_1,	\
				ARM_IRQ_SEC_SGI_2,	\
				ARM_IRQ_SEC_SGI_3,	\
				ARM_IRQ_SEC_SGI_4,	\
				ARM_IRQ_SEC_SGI_5,	\
				ARM_IRQ_SEC_SGI_6,	\
				ARM_IRQ_SEC_SGI_7

#define PLAT_ARM_G0_IRQS

#endif /* __PLATFORM_DEF_H__ */
