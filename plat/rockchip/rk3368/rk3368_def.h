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

#define RK3399_PRIMARY_CPU	0x0

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define RK_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define CCI400_BASE		0xffb90000
#define CCI400_SIZE		0x10000

#define GIC400_BASE		0xffb70000
#define GIC400_SIZE		0x10000

#define STIME_BASE		0xff830000
#define STIME_SIZE		0x10000

/*
 * include cru grf gpio1~3
 * 0xff760000 0xff7b0000
 */
#define PD_ALIVE_BASE		0xff760000
#define PD_ALIVE_SIZE		0x50000

#define SGRF_BASE		0xff740000
#define SGRF_SIZE		0x10000

#define PMU_BASE		0xff730000
#define PMU_SIZE		0x10000

#define RK_INTMEM_BASE		0xff8c0000
#define RK_INTMEM_SIZE		0x10000

/*
 * include i2c pmu/audio, pwm0-3 rkpwm0-3 uart_dbg,mailbox scr
 * 0xff650000 -0xff6c0000
 */
#define PD_BUS0_BASE		0xff650000
#define PD_BUS0_SIZE		0x70000

#define RK_GIC_BASE		(GIC400_BASE)
#define PLAT_RK_CCI_BASE	(CCI400_BASE)
#define CRU_BASE		0xff760000

#define PMUSRAM_BASE            0xff720000
#define PMUSRAM_SIZE            0x10000

/**************************************************************************
 * UART related constants
 **************************************************************************/
#define RK3399_UART2_BASE	0xff690000
#define RK3399_BAUDRATE		115200
#define RK3399_UART_CLOCK	24000000

/******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	24000000
#define SYS_COUNTER_FREQ_IN_MHZ		24

/******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base rk_platform compatible GIC memory map */
#define BASE_GICD_BASE		(RK_GIC_BASE + 0x1000)
#define BASE_GICC_BASE		(RK_GIC_BASE + 0x2000)
#define BASE_GICR_BASE		0	/* no GICR in GIC-400 */

/*****************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_RK_CCI_CLUSTER0_SL_IFACE_IX	3
#define PLAT_RK_CCI_CLUSTER1_SL_IFACE_IX	4

/******************************************************************************
 * cpu up status
 ******************************************************************************/
#define PMU_CPU_HOTPLUG		0xdeadbeaf
#define PMU_CPU_AUTO_PWRDN	0xabcdef12

/******************************************************************************
 * allocate pmu sram
 ******************************************************************************/
/*
  * Needed aligned 16 bytes for sp stack top
  */
#define PSRAM_DT_SIZE		(16 * 2)
#define PSRAM_DT_BASE		(0xff721000 - PSRAM_DT_SIZE)

/******************************************************************************
 * sgi, ppi
 ******************************************************************************/
#define ARM_IRQ_SEC_PHY_TIMER	29

#define ARM_IRQ_SEC_SGI_0	8
#define ARM_IRQ_SEC_SGI_1	9
#define ARM_IRQ_SEC_SGI_2	10
#define ARM_IRQ_SEC_SGI_3	11
#define ARM_IRQ_SEC_SGI_4	12
#define ARM_IRQ_SEC_SGI_5	13
#define ARM_IRQ_SEC_SGI_6	14
#define ARM_IRQ_SEC_SGI_7	15

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define ARM_G1S_IRQS		(ARM_IRQ_SEC_PHY_TIMER)
#define ARM_G0_IRQS

#endif /* __PLAT_DEF_H__ */
