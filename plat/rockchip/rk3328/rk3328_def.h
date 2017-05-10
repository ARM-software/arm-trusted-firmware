/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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

#define MAJOR_VERSION		(1)
#define MINOR_VERSION		(2)

#define SIZE_K(n)		((n) * 1024)

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define RK_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define UART2_BASE		0xff130000
#define UART2_SIZE		SIZE_K(64)

#define PMU_BASE		0xff140000
#define PMU_SIZE		SIZE_K(64)

#define SGRF_BASE		0xff0d0000
#define SGRF_SIZE		SIZE_K(64)

#define CRU_BASE		0xff440000
#define CRU_SIZE		SIZE_K(64)

#define GRF_BASE		0xff100000
#define GRF_SIZE		SIZE_K(64)

#define GPIO0_BASE		0xff210000
#define GPIO0_SIZE		SIZE_K(32)

#define GPIO1_BASE		0xff220000
#define GPIO1_SIZE		SIZE_K(32)

#define GPIO2_BASE		0xff230000
#define GPIO2_SIZE		SIZE_K(64)

#define GPIO3_BASE		0xff240000
#define GPIO3_SIZE		SIZE_K(64)

#define STIME_BASE		0xff1d0000
#define STIME_SIZE		SIZE_K(64)

#define INTMEM_BASE		0xff090000
#define INTMEM_SIZE		SIZE_K(32)

#define SRAM_LDS_BASE		(INTMEM_BASE + SIZE_K(4))
#define SRAM_LDS_SIZE		(INTMEM_SIZE - SIZE_K(4))

#define PMUSRAM_BASE		INTMEM_BASE
#define PMUSRAM_SIZE		SIZE_K(4)
#define PMUSRAM_RSIZE		SIZE_K(4)

#define VOP_BASE		0xff370000
#define VOP_SIZE		SIZE_K(16)

#define DDR_PHY_BASE		0xff400000
#define DDR_PHY_SIZE		SIZE_K(4)

#define SERVER_MSCH_BASE	0xff720000
#define SERVER_MSCH_SIZE	SIZE_K(4)

#define DDR_UPCTL_BASE		0xff780000
#define DDR_UPCTL_SIZE		SIZE_K(12)

#define DDR_MONITOR_BASE	0xff790000
#define DDR_MONITOR_SIZE	SIZE_K(4)

#define FIREWALL_DDR_BASE	0xff7c0000
#define FIREWALL_DDR_SIZE	SIZE_K(64)

#define FIREWALL_CFG_BASE	0xff7d0000
#define FIREWALL_CFG_SIZE	SIZE_K(64)

#define GIC400_BASE		0xff810000
#define GIC400_SIZE		SIZE_K(64)

#define DDR_GRF_BASE		0xff798000
#define DDR_GRF_SIZE		SIZE_K(16)

#define PWM_BASE		0xff1b0000
#define PWM_SIZE		SIZE_K(64)

#define DDR_PARAM_BASE		0x02000000
#define DDR_PARAM_SIZE		SIZE_K(4)

#define EFUSE8_BASE		0xff260000
#define EFUSE8_SIZE		SIZE_K(4)

#define EFUSE32_BASE		0xff0b0000
#define EFUSE32_SIZE		SIZE_K(4)

/**************************************************************************
 * UART related constants
 **************************************************************************/
#define RK3328_UART2_BASE	UART2_BASE
#define RK3328_BAUDRATE	1500000
#define RK3328_UART_CLOCK	24000000

/******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	24000000U
#define SYS_COUNTER_FREQ_IN_MHZ		24

/******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base rk_platform compatible GIC memory map */
#define RK3328_GICD_BASE		(GIC400_BASE + 0x1000)
#define RK3328_GICC_BASE		(GIC400_BASE + 0x2000)
#define RK3328_GICR_BASE		0	/* no GICR in GIC-400 */

/******************************************************************************
 * sgi, ppi
 ******************************************************************************/
#define RK_IRQ_SEC_PHY_TIMER	29

#define RK_IRQ_SEC_SGI_0	8
#define RK_IRQ_SEC_SGI_1	9
#define RK_IRQ_SEC_SGI_2	10
#define RK_IRQ_SEC_SGI_3	11
#define RK_IRQ_SEC_SGI_4	12
#define RK_IRQ_SEC_SGI_5	13
#define RK_IRQ_SEC_SGI_6	14
#define RK_IRQ_SEC_SGI_7	15

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define RK_G1S_IRQS		RK_IRQ_SEC_PHY_TIMER, RK_IRQ_SEC_SGI_6

#define SHARE_MEM_BASE          0x100000/* [1MB, 1MB+60K]*/
#define SHARE_MEM_PAGE_NUM      15
#define SHARE_MEM_SIZE          SIZE_K(SHARE_MEM_PAGE_NUM * 4)

#endif /* __PLAT_DEF_H__ */
