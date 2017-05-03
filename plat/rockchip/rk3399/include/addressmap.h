/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ROCKCHIP_RK3399_INCLUDE_ADDRESSMAP_H__
#define __ROCKCHIP_RK3399_INCLUDE_ADDRESSMAP_H__

#include <addressmap_shared.h>

/* Registers base address */
#define MMIO_BASE		0xF8000000

/* Aggregate of all devices in the first GB */
#define DEV_RNG0_BASE		MMIO_BASE
#define DEV_RNG0_SIZE		SIZE_M(125)

#endif /* __ROCKCHIP_RK3399_INCLUDE_ADDRESSMAP_H__ */
