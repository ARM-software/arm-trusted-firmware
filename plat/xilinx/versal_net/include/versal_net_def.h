/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VERSAL_NET_DEF_H
#define VERSAL_NET_DEF_H

#include <plat/arm/common/smccc_def.h>
#include <plat/common/common_def.h>

/* This part is taken from U-Boot project under GPL that's why dual license above */
#define __bf_shf(x) (__builtin_ffsll(x) - 1U)
#define FIELD_GET(_mask, _reg)						\
	({								\
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask));	\
	})

/* List all consoles */
#define VERSAL_NET_CONSOLE_ID_pl011	U(1)
#define VERSAL_NET_CONSOLE_ID_pl011_0	U(1)
#define VERSAL_NET_CONSOLE_ID_pl011_1	U(2)

#define VERSAL_NET_CONSOLE_IS(con)	(VERSAL_NET_CONSOLE_ID_ ## con == VERSAL_NET_CONSOLE)

/* List all platforms */
#define VERSAL_NET_SILICON		U(0)
#define VERSAL_NET_SPP			U(1)
#define VERSAL_NET_EMU			U(2)
#define VERSAL_NET_QEMU			U(3)

/* For platform detection */
#define PMC_TAP				U(0xF11A0000)
#define PMC_TAP_VERSION			(PMC_TAP + 0x4U)
# define PLATFORM_MASK			GENMASK(27U, 24U)
# define PLATFORM_VERSION_MASK		GENMASK(31U, 28U)

/* Global timer reset */
#define PSX_CRF			U(0xEC200000)
#define PSX_CRF_RST_TIMESTAMP_OFFSET	U(0x33C)

/* Firmware Image Package */
#define VERSAL_NET_PRIMARY_CPU		U(0)

/*******************************************************************************
 * memory map related constants
 ******************************************************************************/
/* IPP 1.2/SPP 0.9 mapping */
#define DEVICE0_BASE		U(0xE8000000) /* psx, crl, iou */
#define DEVICE0_SIZE		U(0x08000000)
#define DEVICE1_BASE		U(0xE2000000) /* gic */
#define DEVICE1_SIZE		U(0x00800000)
#define DEVICE2_BASE		U(0xF1000000) /* uart, pmc_tap */
#define DEVICE2_SIZE		U(0x01000000)
#define CRF_BASE		U(0xFD1A0000)
#define CRF_SIZE		U(0x00600000)

/* CRL */
#define VERSAL_NET_CRL					U(0xEB5E0000)
#define VERSAL_NET_CRL_TIMESTAMP_REF_CTRL_OFFSET	U(0x14C)
#define VERSAL_NET_CRL_RST_TIMESTAMP_OFFSET		U(0x348)

#define VERSAL_NET_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT	(1U << 25U)

/* IOU SCNTRS */
#define VERSAL_NET_IOU_SCNTRS					U(0xEC920000)
#define VERSAL_NET_IOU_SCNTRS_COUNTER_CONTROL_REG_OFFSET	U(0)
#define VERSAL_NET_IOU_SCNTRS_BASE_FREQ_OFFSET			U(0x20)

#define VERSAL_NET_IOU_SCNTRS_CONTROL_EN	U(1)

/*******************************************************************************
 * IRQ constants
 ******************************************************************************/
#define VERSAL_NET_IRQ_SEC_PHY_TIMER	U(29)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define VERSAL_NET_UART0_BASE		U(0xF1920000)
#define VERSAL_NET_UART_BAUDRATE	115200

#define VERSAL_NET_UART_BASE		VERSAL_NET_UART0_BASE

#define PLAT_VERSAL_NET_CRASH_UART_BASE		VERSAL_NET_UART_BASE
#define PLAT_VERSAL_NET_CRASH_UART_CLK_IN_HZ	VERSAL_NET_UART_CLOCK
#define VERSAL_NET_CONSOLE_BAUDRATE		VERSAL_NET_UART_BAUDRATE

#endif /* VERSAL_NET_DEF_H */
