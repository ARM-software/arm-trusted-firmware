/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>

#ifndef FPGA_DEF_H
#define FPGA_DEF_H

/*
 * These are set to large values to account for images describing systems with
 * larger cluster configurations.
 *
 * For cases where the number of clusters, cores or threads is smaller than a
 * maximum value below, this does not affect the PSCI functionality as any PEs
 * that are present will still be indexed appropriately regardless of any empty
 * entries in the array used to represent the topology.
 */
#define FPGA_MAX_CLUSTER_COUNT			2
#define FPGA_MAX_CPUS_PER_CLUSTER		8
#define FPGA_MAX_PE_PER_CPU			4

#define FPGA_PRIMARY_CPU			0x0

/*******************************************************************************
 * FPGA image memory map related constants
 ******************************************************************************/

/* UART base address and clock frequency, as configured by the image */
#define PLAT_FPGA_BOOT_UART_BASE 		0x7ff80000
#define PLAT_FPGA_BOOT_UART_CLK_IN_HZ 		10000000

#define PLAT_FPGA_CRASH_UART_BASE		PLAT_FPGA_BOOT_UART_BASE
#define PLAT_FPGA_CRASH_UART_CLK_IN_HZ		PLAT_FPGA_BOOT_UART_CLK_IN_HZ

#define FPGA_TIMER_FREQUENCY			10000000

#endif
