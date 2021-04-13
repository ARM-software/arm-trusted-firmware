/*
 * Copyright (c) 2021,  Xilinx Inc.
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
int console_dcc_register(void);

#endif /* DCC */
