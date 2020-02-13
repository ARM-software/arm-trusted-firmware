/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_PARAMETER_H
#define DDR_PARAMETER_H

#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <plat_private.h>
#include <soc.h>

#define DDR_REGION_NR_MAX		10
#define REGION_NR_OFFSET		0
#define REGION_ADDR_OFFSET		8
#define REGION_DATA_PER_BYTES		8
#define RG_SIZE_MB(byte)		((byte) >> 20)

/* unit: MB */
struct param_ddr_usage {
	uint64_t boundary;

	uint32_t ns_nr;
	uint64_t ns_base[DDR_REGION_NR_MAX];
	uint64_t ns_top[DDR_REGION_NR_MAX];

	uint32_t s_nr;
	uint64_t s_base[DDR_REGION_NR_MAX + 1];
	uint64_t s_top[DDR_REGION_NR_MAX + 1];
};

struct param_ddr_usage ddr_region_usage_parse(uint64_t addr, uint64_t max_mb);

#endif /* DDR_PARAMETER_H */
