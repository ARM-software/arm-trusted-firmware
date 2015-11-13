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

#ifndef __PLAT_DEF_H__
#define __PLAT_DEF_H__

#define MT8173_PRIMARY_CPU	0x0

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define MT_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define IO_PHYS			(0x10000000)
#define INFRACFG_AO_BASE	(IO_PHYS + 0x1000)
#define GPIO_BASE		(IO_PHYS + 0x5000)
#define SPM_BASE		(IO_PHYS + 0x6000)
#define RGU_BASE		(IO_PHYS + 0x7000)
#define PMIC_WRAP_BASE		(IO_PHYS + 0xD000)
#define MCUCFG_BASE		(IO_PHYS + 0x200000)
#define TRNG_base		(IO_PHYS + 0x20F000)
#define MT_GIC_BASE		(IO_PHYS + 0x220000)
#define PLAT_MT_CCI_BASE	(IO_PHYS + 0x390000)

/* Aggregate of all devices in the first GB */
#define MTK_DEV_RNG0_BASE	IO_PHYS
#define MTK_DEV_RNG0_SIZE	0x400000
#define MTK_DEV_RNG1_BASE	(IO_PHYS + 0x1000000)
#define MTK_DEV_RNG1_SIZE	0x4000000

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define MT8173_UART0_BASE	(IO_PHYS + 0x01002000)
#define MT8173_UART1_BASE	(IO_PHYS + 0x01003000)
#define MT8173_UART2_BASE	(IO_PHYS + 0x01004000)
#define MT8173_UART3_BASE	(IO_PHYS + 0x01005000)

#define MT8173_BAUDRATE		(115200)
#define MT8173_UART_CLOCK	(26000000)

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	13000000
#define SYS_COUNTER_FREQ_IN_MHZ		13

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base MTK_platform compatible GIC memory map */
#define BASE_GICD_BASE		(MT_GIC_BASE + 0x1000)
#define BASE_GICC_BASE		(MT_GIC_BASE + 0x2000)
#define BASE_GICR_BASE		0	/* no GICR in GIC-400 */
#define BASE_GICH_BASE		(MT_GIC_BASE + 0x4000)
#define BASE_GICV_BASE		(MT_GIC_BASE + 0x6000)
#define INT_POL_CTL0		0x10200620

#define GIC_PRIVATE_SIGNALS	(32)

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_MT_CCI_CLUSTER0_SL_IFACE_IX	4
#define PLAT_MT_CCI_CLUSTER1_SL_IFACE_IX	3

/*******************************************************************************
 * WDT related constants
 ******************************************************************************/
#define MTK_WDT_BASE		(RGU_BASE + 0)
#define MTK_WDT_SWRST		(MTK_WDT_BASE + 0x0014)

#define MTK_WDT_MODE_DUAL_MODE	0x0040
#define MTK_WDT_MODE_IRQ	0x0008
#define MTK_WDT_MODE_KEY	0x22000000
#define MTK_WDT_MODE_EXTEN	0x0004
#define MTK_WDT_SWRST_KEY	0x1209

/* FIQ platform related define */
#define MT_IRQ_SEC_SGI_0	8
#define MT_IRQ_SEC_SGI_1	9
#define MT_IRQ_SEC_SGI_2	10
#define MT_IRQ_SEC_SGI_3	11
#define MT_IRQ_SEC_SGI_4	12
#define MT_IRQ_SEC_SGI_5	13
#define MT_IRQ_SEC_SGI_6	14
#define MT_IRQ_SEC_SGI_7	15

#endif /* __PLAT_DEF_H__ */
