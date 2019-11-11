/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>

#ifndef FPGA_DEF_H
#define FPGA_DEF_H

/*
 * The initial FPGA image configures a system with 2 clusters, 1 core in each,
 * and multi-threading is unimplemented.
 */
#define FPGA_MAX_CLUSTER_COUNT			2
#define FPGA_MAX_CPUS_PER_CLUSTER		1
#define FPGA_MAX_PE_PER_CPU			1

#define FPGA_PRIMARY_CPU			0x0

/*******************************************************************************
 * FPGA image memory map related constants
 ******************************************************************************/

/* UART base address and clock frequency, as configured by the image */
#define PLAT_FPGA_BOOT_UART_BASE 		0x7ff80000
#define PLAT_FPGA_BOOT_UART_CLK_IN_HZ 		10000000

#define PLAT_FPGA_CRASH_UART_BASE		PLAT_FPGA_BOOT_UART_BASE
#define PLAT_FPGA_CRASH_UART_CLK_IN_HZ		PLAT_FPGA_BOOT_UART_CLK_IN_HZ

#endif
