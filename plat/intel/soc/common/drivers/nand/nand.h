/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_H
#define DDR_H

#include <lib/mmio.h>
#include "socfpga_handoff.h"

/* FUNCTION DEFINATION */
/*
 * @brief Nand controller initialization function
 *
 * @hoff_ptr: Pointer to the hand-off data
 * Return: 0 on success, a negative errno on failure
 */
int nand_init(handoff *hoff_ptr);

#endif
