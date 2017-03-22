/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA_PLATFORM_H
#define TEGRA_PLATFORM_H

#include <cdefs.h>
#include <stdbool.h>

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
bool tegra_platform_is_silicon(void);
bool tegra_platform_is_qt(void);
bool tegra_platform_is_emulation(void);
bool tegra_platform_is_linsim(void);
bool tegra_platform_is_fpga(void);
bool tegra_platform_is_unit_fpga(void);
bool tegra_platform_is_virt_dev_kit(void);

#endif /* TEGRA_PLATFORM_H */
