/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/xlat_tables/xlat_tables_v2.h>

#include <platform_def.h>

const mmap_region_t imx_mmap[] = {
	AIPS1_MAP, AIPS2_MAP, AIPS3_MAP, AIPS4_MAP, GIC_MAP,
	GPIO2_MAP, GPIO4_MAP, GPIO6_MAP, ELE_MU_MAP,
	{0},
};

const unsigned long gpio_base[GPIO_NUM] = {
	GPIO2_BASE, GPIO3_BASE, GPIO4_BASE, GPIO5_BASE,
	GPIO6_BASE, GPIO7_BASE
};
