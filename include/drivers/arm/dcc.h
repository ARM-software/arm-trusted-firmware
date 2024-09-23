/*
 * Copyright (c) 2021-2022, Xilinx Inc.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DCC_H
#define DCC_H

#include <stdint.h>
#include <drivers/console.h>

/*
 * Initialize a new dcc console instance and register it with the console
 * framework.
 */
int console_dcc_register(console_t *console);
void console_dcc_unregister(console_t *console);

#endif /* DCC_H */
