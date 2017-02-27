/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __ROCKCHIP_RK3399_INCLUDE_SHARED_ADDRESSMAP_SHARED_H__
#define __ROCKCHIP_RK3399_INCLUDE_SHARED_ADDRESSMAP_SHARED_H__

#define SIZE_K(n)		((n) * 1024)
#define SIZE_M(n)		((n) * 1024 * 1024)

/*
 * The parts of the shared defined registers address with AP and M0,
 * let's note and mark the previous defines like this:
 */
#define GIC500_BASE		(MMIO_BASE + 0x06E00000)
#define UART0_BASE		(MMIO_BASE + 0x07180000)
#define UART1_BASE		(MMIO_BASE + 0x07190000)
#define UART2_BASE		(MMIO_BASE + 0x071A0000)
#define UART3_BASE		(MMIO_BASE + 0x071B0000)

#define PMU_BASE		(MMIO_BASE + 0x07310000)
#define PMUGRF_BASE		(MMIO_BASE + 0x07320000)
#define SGRF_BASE		(MMIO_BASE + 0x07330000)
#define PMUSRAM_BASE		(MMIO_BASE + 0x073B0000)
#define PWM_BASE		(MMIO_BASE + 0x07420000)

#define CIC_BASE		(MMIO_BASE + 0x07620000)
#define PD_BUS0_BASE		(MMIO_BASE + 0x07650000)
#define DCF_BASE		(MMIO_BASE + 0x076A0000)
#define GPIO0_BASE		(MMIO_BASE + 0x07720000)
#define GPIO1_BASE		(MMIO_BASE + 0x07730000)
#define PMUCRU_BASE		(MMIO_BASE + 0x07750000)
#define CRU_BASE		(MMIO_BASE + 0x07760000)
#define GRF_BASE		(MMIO_BASE + 0x07770000)
#define GPIO2_BASE		(MMIO_BASE + 0x07780000)
#define GPIO3_BASE		(MMIO_BASE + 0x07788000)
#define GPIO4_BASE		(MMIO_BASE + 0x07790000)
#define STIME_BASE		(MMIO_BASE + 0x07860000)
#define SRAM_BASE		(MMIO_BASE + 0x078C0000)
#define SERVICE_NOC_0_BASE	(MMIO_BASE + 0x07A50000)
#define DDRC0_BASE		(MMIO_BASE + 0x07A80000)
#define SERVICE_NOC_1_BASE	(MMIO_BASE + 0x07A84000)
#define DDRC1_BASE		(MMIO_BASE + 0x07A88000)
#define SERVICE_NOC_2_BASE	(MMIO_BASE + 0x07A8C000)
#define SERVICE_NOC_3_BASE	(MMIO_BASE + 0x07A90000)
#define CCI500_BASE		(MMIO_BASE + 0x07B00000)
#define COLD_BOOT_BASE		(MMIO_BASE + 0x07FF0000)

/* Registers size */
#define GIC500_SIZE		SIZE_M(2)
#define UART0_SIZE		SIZE_K(64)
#define UART1_SIZE		SIZE_K(64)
#define UART2_SIZE		SIZE_K(64)
#define UART3_SIZE		SIZE_K(64)
#define PMU_SIZE		SIZE_K(64)
#define PMUGRF_SIZE		SIZE_K(64)
#define SGRF_SIZE		SIZE_K(64)
#define PMUSRAM_SIZE		SIZE_K(64)
#define PMUSRAM_RSIZE		SIZE_K(8)
#define PWM_SIZE		SIZE_K(64)
#define CIC_SIZE		SIZE_K(4)
#define DCF_SIZE		SIZE_K(4)
#define GPIO0_SIZE		SIZE_K(64)
#define GPIO1_SIZE		SIZE_K(64)
#define PMUCRU_SIZE		SIZE_K(64)
#define CRU_SIZE		SIZE_K(64)
#define GRF_SIZE		SIZE_K(64)
#define GPIO2_SIZE		SIZE_K(32)
#define GPIO3_SIZE		SIZE_K(32)
#define GPIO4_SIZE		SIZE_K(32)
#define STIME_SIZE		SIZE_K(64)
#define SRAM_SIZE		SIZE_K(192)
#define SERVICE_NOC_0_SIZE	SIZE_K(192)
#define DDRC0_SIZE		SIZE_K(32)
#define SERVICE_NOC_1_SIZE	SIZE_K(16)
#define DDRC1_SIZE		SIZE_K(32)
#define SERVICE_NOC_2_SIZE	SIZE_K(16)
#define SERVICE_NOC_3_SIZE	SIZE_K(448)
#define CCI500_SIZE		SIZE_M(1)
#define PD_BUS0_SIZE		SIZE_K(448)

/* DDR Registers address */
#define CTL_BASE(ch)		(DDRC0_BASE + (ch) * 0x8000)
#define CTL_REG(ch, n)		(CTL_BASE(ch) + (n) * 0x4)

#define PI_OFFSET		0x800
#define PI_BASE(ch)		(CTL_BASE(ch) + PI_OFFSET)
#define PI_REG(ch, n)		(PI_BASE(ch) + (n) * 0x4)

#define PHY_OFFSET		0x2000
#define PHY_BASE(ch)		(CTL_BASE(ch) + PHY_OFFSET)
#define PHY_REG(ch, n)		(PHY_BASE(ch) + (n) * 0x4)

#define MSCH_BASE(ch)		(SERVICE_NOC_1_BASE + (ch) * 0x8000)

#endif /* __ROCKCHIP_RK3399_INCLUDE_SHARED_ADDRESSMAP_SHARED_H__ */
