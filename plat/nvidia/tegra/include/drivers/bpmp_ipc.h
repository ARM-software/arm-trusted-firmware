/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BPMP_IPC_H
#define BPMP_IPC_H

#include <lib/utils_def.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Currently supported reset identifiers
 */
#define TEGRA_RESET_ID_XUSB_PADCTL	U(114)
#define TEGRA_RESET_ID_GPCDMA		U(70)

/**
 * Clock identifier for the SE device
 */
#define TEGRA_CLK_SE        U(124)

/**
 * Function to initialise the IPC with the bpmp
 */
int32_t tegra_bpmp_ipc_init(void);

/**
 * Handler to reset a module
 */
int32_t tegra_bpmp_ipc_reset_module(uint32_t rst_id);

/**
 * Handler to enable clock to a module. Only SE device is
 * supported for now.
 */
int tegra_bpmp_ipc_enable_clock(uint32_t clk_id);

/**
 * Handler to disable clock to a module. Only SE device is
 * supported for now.
 */
int tegra_bpmp_ipc_disable_clock(uint32_t clk_id);

#endif /* BPMP_IPC_H */
