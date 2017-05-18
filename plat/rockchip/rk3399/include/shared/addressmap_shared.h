/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ROCKCHIP_RK3399_INCLUDE_SHARED_ADDRESSMAP_SHARED_H__
#define __ROCKCHIP_RK3399_INCLUDE_SHARED_ADDRESSMAP_SHARED_H__

#define SIZE_K(n)		((n) * 1024)
#define SIZE_M(n)		((n) * 1024 * 1024)
#define SRAM_TEXT_LIMIT		(4 * 1024)
#define SRAM_DATA_LIMIT		(4 * 1024)
#define SRAM_BIN_LIMIT		(4 * 1024)

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
#define WDT1_BASE		(MMIO_BASE + 0x07840000)
#define WDT0_BASE		(MMIO_BASE + 0x07848000)
#define TIMER_BASE		(MMIO_BASE + 0x07850000)
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
