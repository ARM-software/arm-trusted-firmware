/*
 * Copyright 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_PLAT_COMMON_H
#define IMX_PLAT_COMMON_H

#include <stdint.h>

#include <arch_helpers.h>

uint32_t plat_get_spsr_for_bl33_entry(void);

#endif /*IMX_PLAT_COMMON_H */
