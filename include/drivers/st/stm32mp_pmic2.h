/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_PMIC2_H
#define STM32MP_PMIC2_H

#include <stdbool.h>
#include <drivers/st/regulator.h>

#include <platform_def.h>

/*
 * dt_pmic_status - Check PMIC status from device tree
 *
 * Returns the status of the PMIC (secure, non-secure), or a negative value on
 * error
 */
int dt_pmic_status(void);

/*
 * initialize_pmic_i2c - Initialize I2C for the PMIC control
 *
 * Returns true if PMIC is available, false if not found, panics on errors
 */
bool initialize_pmic_i2c(void);

/*
 * initialize_pmic - Main PMIC initialization function, called at platform init
 *
 * Panics on errors
 */
void initialize_pmic(void);

/*
 * stpmic2_set_prop - Set PMIC2 proprietary property
 *
 * Returns non zero on errors
 */
int stpmic2_set_prop(const struct regul_description *desc, uint16_t prop, uint32_t value);

/*
 * pmic_switch_off - switch off the platform with PMIC
 *
 * Panics on errors
 */
void pmic_switch_off(void);

#endif /* STM32MP_PMIC2_H */
