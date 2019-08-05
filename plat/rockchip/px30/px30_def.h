/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PX30_DEF_H__
#define __PX30_DEF_H__

#define MAJOR_VERSION		(1)
#define MINOR_VERSION		(0)

#define SIZE_K(n)		((n) * 1024)

#define WITH_16BITS_WMSK(bits)	(0xffff0000 | (bits))

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define RK_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define PMU_BASE		0xff000000
#define PMU_SIZE		SIZE_K(64)

#define PMUGRF_BASE		0xff010000
#define PMUGRF_SIZE		SIZE_K(64)

#define PMUSRAM_BASE		0xff020000
#define PMUSRAM_SIZE		SIZE_K(64)
#define PMUSRAM_RSIZE		SIZE_K(8)

#define UART0_BASE		0xff030000
#define UART0_SIZE		SIZE_K(64)

#define GPIO0_BASE		0xff040000
#define GPIO0_SIZE		SIZE_K(64)

#define PMUSGRF_BASE		0xff050000
#define PMUSGRF_SIZE		SIZE_K(64)

#define INTSRAM_BASE		0xff0e0000
#define INTSRAM_SIZE		SIZE_K(64)

#define SGRF_BASE		0xff11c000
#define SGRF_SIZE		SIZE_K(16)

#define GIC400_BASE		0xff130000
#define GIC400_SIZE		SIZE_K(64)

#define GRF_BASE		0xff140000
#define GRF_SIZE		SIZE_K(64)

#define UART1_BASE		0xff158000
#define UART1_SIZE		SIZE_K(64)

#define UART2_BASE		0xff160000
#define UART2_SIZE		SIZE_K(64)

#define UART5_BASE		0xff178000
#define UART5_SIZE		SIZE_K(64)

#define I2C0_BASE		0xff180000
#define I2C0_SIZE		SIZE_K(64)

#define PWM0_BASE		0xff200000
#define PWM0_SIZE		SIZE_K(32)

#define PWM1_BASE		0xff208000
#define PWM1_SIZE		SIZE_K(32)

#define NTIME_BASE		0xff210000
#define NTIME_SIZE		SIZE_K(64)

#define STIME_BASE		0xff220000
#define STIME_SIZE		SIZE_K(64)

#define DCF_BASE		0xff230000
#define DCF_SIZE		SIZE_K(64)

#define GPIO1_BASE		0xff250000
#define GPIO1_SIZE		SIZE_K(64)

#define GPIO2_BASE		0xff260000
#define GPIO2_SIZE		SIZE_K(64)

#define GPIO3_BASE		0xff270000
#define GPIO3_SIZE		SIZE_K(64)

#define DDR_PHY_BASE		0xff2a0000
#define DDR_PHY_SIZE		SIZE_K(64)

#define CRU_BASE		0xff2b0000
#define CRU_SIZE		SIZE_K(32)

#define CRU_BOOST_BASE		0xff2b8000
#define CRU_BOOST_SIZE		SIZE_K(16)

#define PMUCRU_BASE		0xff2bc000
#define PMUCRU_SIZE		SIZE_K(16)

#define VOP_BASE		0xff460000
#define VOP_SIZE		SIZE_K(16)

#define SERVER_MSCH_BASE	0xff530000
#define SERVER_MSCH_SIZE	SIZE_K(64)

#define FIREWALL_DDR_BASE	0xff534000
#define FIREWALL_DDR_SIZE	SIZE_K(16)

#define DDR_UPCTL_BASE		0xff600000
#define DDR_UPCTL_SIZE		SIZE_K(64)

#define DDR_MNTR_BASE		0xff610000
#define DDR_MNTR_SIZE		SIZE_K(64)

#define DDR_STDBY_BASE		0xff620000
#define DDR_STDBY_SIZE		SIZE_K(64)

#define DDRGRF_BASE		0xff630000
#define DDRGRF_SIZE		SIZE_K(32)

/**************************************************************************
 * UART related constants
 **************************************************************************/
#define PX30_UART_BASE		UART2_BASE
#define PX30_BAUDRATE		1500000
#define PX30_UART_CLOCK		24000000

/******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	24000000
#define SYS_COUNTER_FREQ_IN_MHZ		24

/******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base rk_platform compatible GIC memory map */
#define PX30_GICD_BASE		(GIC400_BASE + 0x1000)
#define PX30_GICC_BASE		(GIC400_BASE + 0x2000)
#define PX30_GICR_BASE		0	/* no GICR in GIC-400 */

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
 * Define a list of Group 0 interrupts.
 */
#define PLAT_RK_GICV2_G0_IRQS						\
	INTR_PROP_DESC(RK_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,	\
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),		\
	INTR_PROP_DESC(RK_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL)

#define SHARE_MEM_BASE		0x100000/* [1MB, 1MB+60K]*/
#define SHARE_MEM_PAGE_NUM	15
#define SHARE_MEM_SIZE		SIZE_K(SHARE_MEM_PAGE_NUM * 4)

#define DDR_PARAM_BASE		0x02000000
#define DDR_PARAM_SIZE		SIZE_K(4)

#endif /* __PLAT_DEF_H__ */
