/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TEGRA_PLATFORM_H__
#define __TEGRA_PLATFORM_H__

#include <sys/cdefs.h>

/*
 * Tegra chip major/minor version
 */
uint32_t tegra_get_chipid_major(void);
uint32_t tegra_get_chipid_minor(void);

/*
 * Tegra chip identifiers
 */
uint8_t tegra_chipid_is_t132(void);
uint8_t tegra_chipid_is_t210(void);
uint8_t tegra_chipid_is_t186(void);


/*
 * Tegra platform identifiers
 */
uint8_t tegra_platform_is_silicon(void);
uint8_t tegra_platform_is_qt(void);
uint8_t tegra_platform_is_emulation(void);
uint8_t tegra_platform_is_fpga(void);

#endif /* __TEGRA_PLATFORM_H__ */
