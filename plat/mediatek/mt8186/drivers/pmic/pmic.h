/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_H
#define PMIC_H

#include <stdint.h>

#define PMIC_RG_HWCID_ADDR 0x8
#define PMIC_PWRHOLD 0xa08

/* external API */
uint32_t pmic_get_hwcid(void);
void pmic_power_off(void);

#endif /* PMIC_H */
