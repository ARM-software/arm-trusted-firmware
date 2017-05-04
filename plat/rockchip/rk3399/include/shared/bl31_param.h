/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_ROCKCHIP_RK3399_INCLUDE_SHARED_BL31_PARAM_H__
#define __PLAT_ROCKCHIP_RK3399_INCLUDE_SHARED_BL31_PARAM_H__

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
/* TF text, ro, rw, Size: 1MB */
#define TZRAM_BASE		(0x0)
#define TZRAM_SIZE		(0x100000)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted RAM
 */
#define BL31_BASE		(TZRAM_BASE + 0x1000)
#define BL31_LIMIT		(TZRAM_BASE + TZRAM_SIZE)

#endif /*__PLAT_ROCKCHIP_RK3399_INCLUDE_SHARED_BL31_PARAM_H__*/
