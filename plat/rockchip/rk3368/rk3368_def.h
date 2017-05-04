/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_DEF_H__
#define __PLAT_DEF_H__

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define RK_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define CCI400_BASE		0xffb90000
#define CCI400_SIZE		0x10000

#define GIC400_BASE		0xffb70000
#define GIC400_SIZE		0x10000

#define STIME_BASE		0xff830000
#define STIME_SIZE		0x10000

#define CRU_BASE		0xff760000
#define CRU_SIZE		0x10000

#define GRF_BASE		0xff770000
#define GRF_SIZE		0x10000

#define SGRF_BASE		0xff740000
#define SGRF_SIZE		0x10000

#define PMU_BASE		0xff730000
#define PMU_GRF_BASE		0xff738000
#define PMU_SIZE		0x10000

#define RK_INTMEM_BASE		0xff8c0000
#define RK_INTMEM_SIZE		0x10000

#define UART_DBG_BASE		0xff690000
#define UART_DBG_SIZE		0x10000

#define CRU_BASE		0xff760000

#define PMUSRAM_BASE            0xff720000
#define PMUSRAM_SIZE            0x10000
#define PMUSRAM_RSIZE           0x1000

#define DDR_PCTL_BASE		0xff610000
#define DDR_PCTL_SIZE		0x10000

#define DDR_PHY_BASE		0xff620000
#define DDR_PHY_SIZE		0x10000

#define SERVICE_BUS_BASE	0xffac0000
#define SERVICE_BUS_SISE	0x50000

#define COLD_BOOT_BASE		0xffff0000
/**************************************************************************
 * UART related constants
 **************************************************************************/
#define RK3368_UART2_BASE	UART_DBG_BASE
#define RK3368_BAUDRATE		115200
#define RK3368_UART_CLOCK	24000000

/******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	24000000

/******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base rk_platform compatible GIC memory map */
#define RK3368_GICD_BASE		(GIC400_BASE + 0x1000)
#define RK3368_GICC_BASE		(GIC400_BASE + 0x2000)
#define RK3368_GICR_BASE		0	/* no GICR in GIC-400 */

/*****************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_RK_CCI_CLUSTER0_SL_IFACE_IX	3
#define PLAT_RK_CCI_CLUSTER1_SL_IFACE_IX	4

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
#define RK_G1S_IRQS		(RK_IRQ_SEC_PHY_TIMER)

#endif /* __PLAT_DEF_H__ */
