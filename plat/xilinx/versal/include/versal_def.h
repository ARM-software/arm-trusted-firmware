/*
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VERSAL_DEF_H
#define VERSAL_DEF_H

#include <plat/arm/common/smccc_def.h>
#include <plat/common/common_def.h>

#define PLATFORM_MASK                  GENMASK(27U, 24U)
#define PLATFORM_VERSION_MASK          GENMASK(31U, 28U)

/* number of interrupt handlers. increase as required */
#define MAX_INTR_EL3			2
/* List all consoles */
#define VERSAL_CONSOLE_ID_none		0
#define VERSAL_CONSOLE_ID_pl011	1
#define VERSAL_CONSOLE_ID_pl011_0	1
#define VERSAL_CONSOLE_ID_pl011_1	2
#define VERSAL_CONSOLE_ID_dcc		3
#define VERSAL_CONSOLE_ID_dtb		4

#define CONSOLE_IS(con)	(VERSAL_CONSOLE_ID_ ## con == VERSAL_CONSOLE)

/* Runtime console */
#define RT_CONSOLE_ID_pl011	1
#define RT_CONSOLE_ID_pl011_0	1
#define RT_CONSOLE_ID_pl011_1	2
#define RT_CONSOLE_ID_dcc	3
#define RT_CONSOLE_ID_dtb	4

#define RT_CONSOLE_IS(con)	(RT_CONSOLE_ID_ ## con == CONSOLE_RUNTIME)

/* List of platforms */
#define VERSAL_SILICON              U(0)
#define VERSAL_SPP                  U(1)
#define VERSAL_EMU                  U(2)
#define VERSAL_QEMU                 U(3)
#define VERSAL_COSIM                U(7)

/* Firmware Image Package */
#define VERSAL_PRIMARY_CPU	0

/*******************************************************************************
 * memory map related constants
 ******************************************************************************/
#define DEVICE0_BASE		0xFF000000
#define DEVICE0_SIZE		0x00E00000
#define DEVICE1_BASE		0xF9000000
#define DEVICE1_SIZE		0x00800000

/*******************************************************************************
 * IRQ constants
 ******************************************************************************/
#define VERSAL_IRQ_SEC_PHY_TIMER		U(29)
#define ARM_IRQ_SEC_PHY_TIMER	29

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_ARM_CCI_BASE		0xFD000000
#define PLAT_ARM_CCI_SIZE		0x00100000
#define PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX	4
#define PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX	5

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define VERSAL_UART0_BASE		0xFF000000
#define VERSAL_UART1_BASE		0xFF010000

#if CONSOLE_IS(pl011) || CONSOLE_IS(dtb)
# define UART_BASE	VERSAL_UART0_BASE
# define UART_TYPE	CONSOLE_PL011
#elif CONSOLE_IS(pl011_1)
# define UART_BASE	VERSAL_UART1_BASE
# define UART_TYPE	CONSOLE_PL011
#elif CONSOLE_IS(dcc)
# define UART_BASE	0x0
# define UART_TYPE	CONSOLE_DCC
#elif CONSOLE_IS(none)
# define UART_TYPE	CONSOLE_NONE
#else
# error "invalid VERSAL_CONSOLE"
#endif

/* Runtime console */
#if defined(CONSOLE_RUNTIME)
#if RT_CONSOLE_IS(pl011) || RT_CONSOLE_IS(dtb)
# define RT_UART_BASE VERSAL_UART0_BASE
# define RT_UART_TYPE	CONSOLE_PL011
#elif RT_CONSOLE_IS(pl011_1)
# define RT_UART_BASE VERSAL_UART1_BASE
# define RT_UART_TYPE	CONSOLE_PL011
#elif RT_CONSOLE_IS(dcc)
# define RT_UART_BASE	0x0
# define RT_UART_TYPE	CONSOLE_DCC
#else
# error "invalid CONSOLE_RUNTIME"
#endif
#endif

/*******************************************************************************
 * Platform related constants
 ******************************************************************************/
#define UART_BAUDRATE  115200

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

/* IOU SCNTRS */
#define IOU_SCNTRS_BASE	U(0xFF140000)
#define IOU_SCNTRS_BASE_FREQ_OFFSET	U(0x20)

/* APU registers and bitfields */
#define FPD_APU_BASE		0xFD5C0000U
#define FPD_APU_CONFIG_0	(FPD_APU_BASE + 0x20U)
#define FPD_APU_RVBAR_L_0	(FPD_APU_BASE + 0x40U)
#define FPD_APU_RVBAR_H_0	(FPD_APU_BASE + 0x44U)
#define FPD_APU_PWRCTL		(FPD_APU_BASE + 0x90U)

#define FPD_APU_CONFIG_0_VINITHI_SHIFT	8U
#define APU_0_PWRCTL_CPUPWRDWNREQ_MASK	1U
#define APU_1_PWRCTL_CPUPWRDWNREQ_MASK	2U

/* PMC registers and bitfields */
#define PMC_GLOBAL_BASE			0xF1110000U
#define PMC_GLOBAL_GLOB_GEN_STORAGE4	(PMC_GLOBAL_BASE + 0x40U)

#endif /* VERSAL_DEF_H */
