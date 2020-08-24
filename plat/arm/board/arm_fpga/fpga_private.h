/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FPGA_PRIVATE_H
#define FPGA_PRIVATE_H

#include "../fpga_def.h"
#include <platform_def.h>

#define C_RUNTIME_READY_KEY	(0xaa55aa55)
#define VALID_MPID		(1U)
#define FPGA_MAX_DTB_SIZE	0x10000

#ifndef __ASSEMBLER__

extern unsigned char fpga_valid_mpids[PLATFORM_CORE_COUNT];

void fpga_console_init(void);

void plat_fpga_gic_init(void);
void fpga_pwr_gic_on_finish(void);
void fpga_pwr_gic_off(void);
unsigned int plat_fpga_calc_core_pos(uint32_t mpid);
unsigned int fpga_get_nr_gic_cores(void);

#endif /* __ASSEMBLER__ */

#endif /* FPGA_PRIVATE_H */
