/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RK3288_DEF_H
#define RK3288_DEF_H

/* Special value used to verify platform parameters from BL2 to BL31 */
#define RK_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define SIZE_K(n)		((n) * 1024)
#define SIZE_M(n)		((n) * 1024 * 1024)

#define SRAM_TEXT_LIMIT		(4 * 1024)
#define SRAM_DATA_LIMIT		(4 * 1024)

#define DDR_PCTL0_BASE		0xff610000
#define DDR_PCTL0_SIZE		SIZE_K(64)

#define DDR_PHY0_BASE		0xff620000
#define DDR_PHY0_SIZE		SIZE_K(64)

#define DDR_PCTL1_BASE		0xff630000
#define DDR_PCTL1_SIZE		SIZE_K(64)

#define DDR_PHY1_BASE		0xff640000
#define DDR_PHY1_SIZE		SIZE_K(64)

#define UART0_BASE		0xff180000
#define UART0_SIZE		SIZE_K(64)

#define UART1_BASE		0xff190000
#define UART1_SIZE		SIZE_K(64)

#define UART2_BASE		0xff690000
#define UART2_SIZE		SIZE_K(64)

#define UART3_BASE		0xff1b0000
#define UART3_SIZE		SIZE_K(64)

#define UART4_BASE		0xff1c0000
#define UART4_SIZE		SIZE_K(64)

/* 96k instead of 64k? */
#define SRAM_BASE		0xff700000
#define SRAM_SIZE		SIZE_K(64)

#define PMUSRAM_BASE		0xff720000
#define PMUSRAM_SIZE		SIZE_K(4)
#define PMUSRAM_RSIZE		SIZE_K(4)

#define PMU_BASE		0xff730000
#define PMU_SIZE		SIZE_K(64)

#define SGRF_BASE		0xff740000
#define SGRF_SIZE		SIZE_K(64)

#define CRU_BASE		0xff760000
#define CRU_SIZE		SIZE_K(64)

#define GRF_BASE		0xff770000
#define GRF_SIZE		SIZE_K(64)

/* timer 6+7 can be set as secure in SGRF */
#define STIME_BASE		0xff810000
#define STIME_SIZE		SIZE_K(64)

#define SERVICE_BUS_BASE	0xffac0000
#define SERVICE_BUS_SIZE	SIZE_K(64)

#define TZPC_BASE		0xffb00000
#define TZPC_SIZE		SIZE_K(64)

#define GIC400_BASE		0xffc00000
#define GIC400_SIZE		SIZE_K(64)

#define CORE_AXI_BUS_BASE	0xffd00000
#define CORE_AXI_BUS_SIZE	SIZE_M(1)

#define COLD_BOOT_BASE		0xffff0000
/**************************************************************************
 * UART related constants
 **************************************************************************/
#define RK3288_BAUDRATE		115200
#define RK3288_UART_CLOCK	24000000

/******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	24000000

/******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base rk_platform compatible GIC memory map */
#define RK3288_GICD_BASE		(GIC400_BASE + 0x1000)
#define RK3288_GICC_BASE		(GIC400_BASE + 0x2000)
#define RK3288_GICR_BASE		0	/* no GICR in GIC-400 */

/******************************************************************************
 * sgi, ppi
 ******************************************************************************/
#define RK_IRQ_SEC_PHY_TIMER	29

/* what are these, and are they present on rk3288? */
#define RK_IRQ_SEC_SGI_0	8
#define RK_IRQ_SEC_SGI_1	9
#define RK_IRQ_SEC_SGI_2	10
#define RK_IRQ_SEC_SGI_3	11
#define RK_IRQ_SEC_SGI_4	12
#define RK_IRQ_SEC_SGI_5	13
#define RK_IRQ_SEC_SGI_6	14
#define RK_IRQ_SEC_SGI_7	15

/*
 * Define a list of Group 0 interrupts.
 */
#define PLAT_RK_GICV2_G0_IRQS						\
	INTR_PROP_DESC(RK_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,	\
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),		\
	INTR_PROP_DESC(RK_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL)

#endif /* RK3288_DEF_H */
