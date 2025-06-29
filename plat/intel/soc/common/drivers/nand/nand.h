/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NAND_H
#define NAND_H

#include <lib/mmio.h>
#include "socfpga_handoff.h"

/* FUNCTION DEFINATION */
/*
 * @brief Nand controller initialization function
 *
 * Return: 0 on success, a negative errno on failure
 */
int nand_init(void);

#endif