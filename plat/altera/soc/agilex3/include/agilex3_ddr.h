/*
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef agilex3_DDR_H
#define agilex3_DDR_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <lib/utils_def.h>

#include "socfpga_handoff.h"

#define CONFIG_NR_DRAM_BANKS	1

typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;
typedef phys_addr_t fdt_addr_t;

/* DDR/RAM configuration */
struct ddr_info {
	phys_addr_t start;
	phys_size_t size;
};

int agilex3_ddr_init(handoff *hoff_ptr);

#endif /* agilex3_DDR_H */
