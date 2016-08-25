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

#ifndef __PLAT_DEF_H__
#define __PLAT_DEF_H__

#define RK3399_PRIMARY_CPU	0x0

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define RK_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define SIZE_K(n)	((n) * 1024)
#define SIZE_M(n)	((n) * 1024 * 1024)

/* Register base address and size */
#define MMIO_BASE		0xfe000000

#define GIC500_BASE		(MMIO_BASE + 0xe00000)
#define GIC500_SIZE		SIZE_M(2)

#define PMU_BASE		(MMIO_BASE + 0x1310000)
#define PMU_SIZE		SIZE_K(64)

#define PMUGRF_BASE		(MMIO_BASE + 0x1320000)
#define PMUGRF_SIZE		SIZE_K(64)

#define SGRF_BASE		(MMIO_BASE + 0x1330000)
#define SGRF_SIZE		SIZE_K(64)

#define PMUSRAM_BASE		(MMIO_BASE + 0x13b0000)
#define PMUSRAM_SIZE		SIZE_K(64)
#define PMUSRAM_RSIZE		SIZE_K(8)

#define PWM_BASE		(MMIO_BASE + 0x1420000)
#define PWM_SIZE		SIZE_K(64)

#define CIC_BASE		(MMIO_BASE + 0x1620000)
#define CIC_SIZE		SIZE_K(4)

#define DCF_BASE		(MMIO_BASE + 0x16a0000)
#define DCF_SIZE		SIZE_K(4)

#define GPIO0_BASE		(MMIO_BASE + 0x1720000)
#define GPIO0_SIZE		SIZE_K(64)

#define GPIO1_BASE		(MMIO_BASE + 0x1730000)
#define GPIO1_SIZE		SIZE_K(64)

#define CRUS_BASE		(MMIO_BASE + 0x1750000)
#define CRUS_SIZE		SIZE_K(128)

#define GRF_BASE		(MMIO_BASE + 0x1770000)
#define GRF_SIZE		SIZE_K(64)

#define GPIO2_BASE		(MMIO_BASE + 0x1780000)
#define GPIO2_SIZE		SIZE_K(32)

#define GPIO3_BASE		(MMIO_BASE + 0x1788000)
#define GPIO3_SIZE		SIZE_K(32)

#define GPIO4_BASE		(MMIO_BASE + 0x1790000)
#define GPIO4_SIZE		SIZE_K(32)

#define STIME_BASE		(MMIO_BASE + 0x1860000)
#define STIME_SIZE		SIZE_K(64)

#define SRAM_BASE		(MMIO_BASE + 0x18c0000)
#define SRAM_SIZE		SIZE_K(192)

#define SERVICE_NOC_0_BASE	(MMIO_BASE + 0x1a50000)
#define NOC_0_SIZE		SIZE_K(192)

#define DDRC0_BASE		(MMIO_BASE + 0x1a80000)
#define DDRC0_SIZE		SIZE_K(32)

#define SERVICE_NOC_1_BASE	(MMIO_BASE + 0x1a84000)
#define NOC_1_SIZE		SIZE_K(16)

#define DDRC1_BASE		(MMIO_BASE + 0x1a88000)
#define DDRC1_SIZE		SIZE_K(32)

#define SERVICE_NOC_2_BASE	(MMIO_BASE + 0x1a8c000)
#define NOC_2_SIZE		SIZE_K(16)

#define SERVICE_NOC_3_BASE	(MMIO_BASE + 0x1a90000)
#define NOC_3_SIZE		SIZE_K(448)

#define CCI500_BASE		(MMIO_BASE + 0x1b00000)
#define CCI500_SIZE		SIZE_M(1)

#define DDR_PI_OFFSET		0x800
#define DDR_PHY_OFFSET		0x2000

#define DDRC0_PI_BASE		(DDRC0_BASE + DDR_PI_OFFSET)
#define DDRC0_PHY_BASE		(DDRC0_BASE + DDR_PHY_OFFSET)
#define DDRC1_PI_BASE		(DDRC1_BASE + DDR_PI_OFFSET)
#define DDRC1_PHY_BASE		(DDRC1_BASE + DDR_PHY_OFFSET)

/* Aggregate of all devices in the first GB */
#define RK3399_DEV_RNG0_BASE	MMIO_BASE
#define RK3399_DEV_RNG0_SIZE	0x1d00000

/*
 * include i2c pmu/audio, pwm0-3 rkpwm0-3 uart_dbg,mailbox scr
 * 0xff650000 -0xff6c0000
 */
#define PD_BUS0_BASE		(MMIO_BASE + 0x1650000)
#define PD_BUS0_SIZE		SIZE_K(448)

#define PMUCRU_BASE		(MMIO_BASE + 0x1750000)
#define CRU_BASE		(MMIO_BASE + 0x1760000)

#define COLD_BOOT_BASE		(MMIO_BASE + 0x1ff0000)

/**************************************************************************
 * UART related constants
 **************************************************************************/
#define RK3399_UART2_BASE	(0xff1a0000)
#define RK3399_UART2_SIZE	SIZE_K(64)

#define RK3399_BAUDRATE		(115200)
#define RK3399_UART_CLOCK	(24000000)

/******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	24000000

/* Base rockchip_platform compatible GIC memory map */
#define BASE_GICD_BASE		(GIC500_BASE)
#define BASE_GICR_BASE		(GIC500_BASE + SIZE_M(1))

/*****************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_RK_CCI_CLUSTER0_SL_IFACE_IX	0
#define PLAT_RK_CCI_CLUSTER1_SL_IFACE_IX	1

/******************************************************************************
 * sgi, ppi
 ******************************************************************************/
#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15
/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define RK3399_G1S_IRQS			ARM_IRQ_SEC_PHY_TIMER
#define RK3399_G0_IRQS			ARM_IRQ_SEC_SGI_6

#endif /* __PLAT_DEF_H__ */
