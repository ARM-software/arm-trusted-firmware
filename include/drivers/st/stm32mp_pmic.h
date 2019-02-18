/*
 * Copyright (c) 2017-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_PMIC_H
#define STM32MP_PMIC_H

#include <stdbool.h>

#include <platform_def.h>

/*
 * dt_pmic_status - Check PMIC status from device tree
 *
 * Returns the status of the PMIC (secure, non-secure), or a negative value on
 * error
 */
int dt_pmic_status(void);

/*
 * dt_pmic_configure_boot_on_regulators - Configure boot-on and always-on
 * regulators from device tree configuration
 *
 * Returns 0 on success, and negative values on errors
 */
int dt_pmic_configure_boot_on_regulators(void);

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
 * pmic_ddr_power_init - Initialize regulators required for DDR
 *
 * Returns 0 on success, and negative values on errors
 */
int pmic_ddr_power_init(enum ddr_type ddr_type);

#endif /* STM32MP_PMIC_H */
