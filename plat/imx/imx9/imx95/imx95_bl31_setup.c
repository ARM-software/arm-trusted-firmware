/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/xlat_tables/xlat_tables_v2.h>

#include <platform_def.h>

const mmap_region_t imx_mmap[] = {
	AIPS2_MAP, GIC_MAP, AIPS1_MAP, GPIO2_MAP, GPIO4_MAP, ELE_MAP,

	{0},
};

const uintptr_t gpio_base[GPIO_NUM] = {
	GPIO2_BASE, GPIO3_BASE, GPIO4_BASE, GPIO5_BASE
};
