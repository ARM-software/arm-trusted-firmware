/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCU_H
#define SCU_H

#include <stdint.h>

#define SCU_CTRL_REG	0x00
#define SCU_CFG_REG	0x04

#define SCU_ENABLE_BIT	(1 << 0)

void enable_snoop_ctrl_unit(uintptr_t base);
uint32_t read_snoop_ctrl_unit_cfg(uintptr_t base);

#endif /* SCU_H */
