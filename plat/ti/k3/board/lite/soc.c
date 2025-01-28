/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <plat_private.h>

/* Table of regions to map using the MMU */
const mmap_region_t plat_k3_mmap[] = {
	K3_MAP_REGION_FLAT(K3_USART_BASE,       K3_USART_SIZE,       MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(K3_GIC_BASE,         K3_GIC_SIZE,         MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(K3_GTC_BASE,         K3_GTC_SIZE,         MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(SEC_PROXY_RT_BASE,   SEC_PROXY_RT_SIZE,   MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(SEC_PROXY_SCFG_BASE, SEC_PROXY_SCFG_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(SEC_PROXY_DATA_BASE, SEC_PROXY_DATA_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{ /* sentinel */ }
};

void ti_soc_init(void) {}

