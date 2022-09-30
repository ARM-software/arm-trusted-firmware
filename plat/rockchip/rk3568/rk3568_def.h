/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_DEF_H__
#define __PLAT_DEF_H__

#define MAJOR_VERSION		(1)
#define MINOR_VERSION		(0)

#define SIZE_K(n)		((n) * 1024)

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define RK_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define GIC600_BASE		0xfd400000
#define GIC600_SIZE		SIZE_K(64)

#define PMUSGRF_BASE		0xfdc00000
#define SYSSGRF_BASE		0xfdc10000
#define PMUGRF_BASE		0xfdc20000
#define CPUGRF_BASE		0xfdc30000
#define DDRGRF_BASE		0xfdc40000
#define PIPEGRF_BASE		0xfdc50000
#define GRF_BASE		0xfdc60000
#define PIPEPHY_GRF0		0xfdc70000
#define PIPEPHY_GRF1		0xfdc80000
#define PIPEPHY_GRF2		0xfdc90000
#define USBPHY_U3_GRF		0xfdca0000
#define USB2PHY_U2_GRF		0xfdca8000
#define EDPPHY_GRF		0xfdcb0000
#define SYSSRAM_BASE		0xfdcc0000
#define PCIE30PHY_GRF		0xfdcb8000
#define USBGRF_BASE		0xfdcf0000

#define PMUCRU_BASE		0xfdd00000
#define SCRU_BASE		0xfdd10000
#define SGRF_BASE		0xfdd18000
#define STIME_BASE		0xfdd1c000
#define CRU_BASE		0xfdd20000
#define PMUSCRU_BASE		0xfdd30000
#define I2C0_BASE		0xfdd40000

#define UART0_BASE		0xfdd50000
#define GPIO0_BASE		0xfdd60000
#define PMUPVTM_BASE		0xfdd80000
#define PMU_BASE		0xfdd90000
#define PMUSRAM_BASE		0xfdcd0000
#define PMUSRAM_SIZE		SIZE_K(128)
#define PMUSRAM_RSIZE		SIZE_K(8)

#define DDRSGRF_BASE		0xfe200000
#define UART1_BASE		0xfe650000
#define UART2_BASE		0xfe660000
#define GPIO1_BASE		0xfe740000
#define GPIO2_BASE		0xfe750000
#define GPIO3_BASE		0xfe760000
#define GPIO4_BASE		0xfe770000

#define REMAP_BASE		0xffff0000
#define REMAP_SIZE		SIZE_K(64)
/**************************************************************************
 * UART related constants
 **************************************************************************/
#define FPGA_UART_BASE		UART2_BASE
#define FPGA_BAUDRATE		1500000
#define FPGA_UART_CLOCK		24000000

/******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	24000000
#define SYS_COUNTER_FREQ_IN_MHZ		24

/******************************************************************************
 * GIC-600 & interrupt handling related constants
 ******************************************************************************/

/* Base rk_platform compatible GIC memory map */
#define PLAT_GICD_BASE		GIC600_BASE
#define PLAT_GICC_BASE		0
#define PLAT_GICR_BASE		(GIC600_BASE + 0x60000)

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

#define SHARE_MEM_BASE		0x100000/* [1MB, 1MB+60K]*/
#define SHARE_MEM_PAGE_NUM	15
#define SHARE_MEM_SIZE		SIZE_K(SHARE_MEM_PAGE_NUM * 4)

#endif /* __PLAT_DEF_H__ */
