/*
 * Copyright (c) 2023, Aspeed Technology Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_REG_H
#define PLATFORM_REG_H

/* GIC */
#define GICD_BASE	U(0x12200000)
#define GICD_SIZE	U(0x10000)
#define GICR_BASE	U(0x12280000)
#define GICR_SIZE	U(0x100000)

/* UART */
#define UART_BASE	U(0x14c33000)
#define UART12_BASE	(UART_BASE + 0xb00)

/* CPU-die SCU */
#define SCU_CPU_BASE	U(0x12c02000)
#define SCU_CPU_SMP_EP0	(SCU_CPU_BASE + 0x780)
#define SCU_CPU_SMP_EP1	(SCU_CPU_BASE + 0x788)
#define SCU_CPU_SMP_EP2	(SCU_CPU_BASE + 0x790)
#define SCU_CPU_SMP_EP3	(SCU_CPU_BASE + 0x798)

#endif /* PLATFORM_REG_H */
