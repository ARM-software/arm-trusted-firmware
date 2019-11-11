/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FPGA_PRIVATE_H
#define FPGA_PRIVATE_H

unsigned int plat_fpga_calc_core_pos(u_register_t mpidr);

void fpga_console_init(void);

#endif
