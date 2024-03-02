/*
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NRD_SOC_PLATFORM_DEF_H
#define NRD_SOC_PLATFORM_DEF_H

#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/soc/common/soc_css_def.h>

#include <nrd_base_platform_def.h>
#include <nrd_soc_css_def.h>

/* Map the System registers to access from S-EL0 */
#define CSS_SYSTEMREG_DEVICE_BASE	(0x1C010000)
#define CSS_SYSTEMREG_DEVICE_SIZE	(0x00010000)
#define PLAT_ARM_SECURE_MAP_SYSTEMREG	MAP_REGION_FLAT(		    \
						CSS_SYSTEMREG_DEVICE_BASE,  \
						CSS_SYSTEMREG_DEVICE_SIZE,  \
						(MT_DEVICE | MT_RW |	    \
						 MT_SECURE | MT_USER))

/* Map the NOR2 Flash to access from S-EL0 */
#define CSS_NOR2_FLASH_DEVICE_BASE	(0x10000000)
#define CSS_NOR2_FLASH_DEVICE_SIZE	(0x04000000)
#define PLAT_ARM_SECURE_MAP_NOR2	MAP_REGION_FLAT(                    \
						CSS_NOR2_FLASH_DEVICE_BASE, \
						CSS_NOR2_FLASH_DEVICE_SIZE, \
						(MT_DEVICE | MT_RW |	    \
						 MT_SECURE | MT_USER))

#endif /* NRD_SOC_PLATFORM_DEF_H */
