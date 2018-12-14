/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VERSAL_DEF_H
#define VERSAL_DEF_H

#include <plat/common/common_def.h>

/* List all consoles */
#define VERSAL_CONSOLE_ID_pl011	1
#define VERSAL_CONSOLE_ID_pl011_0	1
#define VERSAL_CONSOLE_ID_pl011_1	2
#define VERSAL_CONSOLE_ID_dcc		3

#define VERSAL_CONSOLE_IS(con)	(VERSAL_CONSOLE_ID_ ## con == VERSAL_CONSOLE)

/* List all supported platforms */
#define VERSAL_PLATFORM_ID_versal_virt	1

#define VERSAL_PLATFORM_IS(con)	(VERSAL_PLATFORM_ID_ ## con == VERSAL_PLATFORM)

/* Firmware Image Package */
#define VERSAL_PRIMARY_CPU	0

/*******************************************************************************
 * memory map related constants
 ******************************************************************************/
#define DEVICE0_BASE		0xFF000000
#define DEVICE0_SIZE		0x00E00000
#define DEVICE1_BASE		0xF9000000
#define DEVICE1_SIZE		0x00800000

/* CRL */
#define VERSAL_CRL				0xFF5E0000
#define VERSAL_CRL_IOU_SWITCH_CTRL		(VERSAL_CRL + 0x114)
#define VERSAL_CRL_TIMESTAMP_REF_CTRL		(VERSAL_CRL + 0x14C)
#define VERSAL_CRL_RST_TIMESTAMP_OFFSET	(VERSAL_CRL + 0x348)

#define VERSAL_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT	(1 << 25)
#define VERSAL_IOU_SWITCH_CTRL_CLKACT_BIT		(1 << 25)
#define VERSAL_IOU_SWITCH_CTRL_DIVISOR0_SHIFT		8

/* IOU SCNTRS */
#define VERSAL_IOU_SCNTRS			 0xFF140000
#define VERSAL_IOU_SCNTRS_COUNTER_CONTROL_REG	(VERSAL_IOU_SCNTRS + 0x0)
#define VERSAL_IOU_SCNTRS_BASE_FREQ		(VERSAL_IOU_SCNTRS + 0x20)

#define VERSAL_IOU_SCNTRS_CONTROL_EN	1

/*******************************************************************************
 * IRQ constants
 ******************************************************************************/
#define VERSAL_IRQ_SEC_PHY_TIMER		29

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define VERSAL_UART0_BASE		0xFF000000
#define VERSAL_UART1_BASE		0xFF010000

#if VERSAL_CONSOLE_IS(pl011)
# define VERSAL_UART_BASE	VERSAL_UART0_BASE
#elif VERSAL_CONSOLE_IS(pl011_1)
# define VERSAL_UART_BASE	VERSAL_UART1_BASE
#else
# error "invalid VERSAL_CONSOLE"
#endif

#define PLAT_VERSAL_CRASH_UART_BASE		VERSAL_UART_BASE
#define PLAT_VERSAL_CRASH_UART_CLK_IN_HZ	VERSAL_UART_CLOCK
#define VERSAL_CONSOLE_BAUDRATE			VERSAL_UART_BAUDRATE

/*******************************************************************************
 * Platform related constants
 ******************************************************************************/
#if VERSAL_PLATFORM_IS(versal_virt)
# define PLATFORM_NAME		"Versal Virt"
# define VERSAL_UART_CLOCK	25000000
# define VERSAL_UART_BAUDRATE	115200
# define VERSAL_CPU_CLOCK	62500000
#endif

/* Access control register defines */
#define ACTLR_EL3_L2ACTLR_BIT	(1 << 6)
#define ACTLR_EL3_CPUACTLR_BIT	(1 << 0)

/* For cpu reset APU space here too 0xFE5F1000 CRF_APB*/
#define CRF_BASE		0xFD1A0000
#define CRF_SIZE		0x00600000

/* CRF registers and bitfields */
#define CRF_RST_APU	(CRF_BASE + 0X00000300)

#define CRF_RST_APU_ACPU_RESET		(1 << 0)
#define CRF_RST_APU_ACPU_PWRON_RESET	(1 << 10)

/* APU registers and bitfields */
#define FPD_APU_BASE		0xFD5C0000
#define FPD_APU_CONFIG_0	(FPD_APU_BASE + 0x20)
#define FPD_APU_RVBAR_L_0	(FPD_APU_BASE + 0x40)
#define FPD_APU_RVBAR_H_0	(FPD_APU_BASE + 0x44)
#define FPD_APU_PWRCTL		(FPD_APU_BASE + 0x90)

#define FPD_APU_CONFIG_0_VINITHI_SHIFT	8

#endif /* VERSAL_DEF_H */
